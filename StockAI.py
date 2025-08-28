import os
import pandas as pd
import numpy as np
import requests
import torch
import torch.nn as nn
import torch.nn.functional as F
from transformers import AutoModelForSequenceClassification, AutoTokenizer  # Changed import
import logging
from datetime import datetime, timedelta, timezone
from numba import jit
from retry import retry
import schedule
import time
import pykalman
from scipy.fft import fft
import sqlite3
from pathlib import Path
import pickle
from tenacity import retry, stop_after_attempt, wait_exponential, retry_if_exception_type
from typing import Optional, Dict, List, Tuple
import matplotlib.pyplot as plt
from polygon import RESTClient
import sys


# Configuration
class Config:
    DB_PATH = "trading_data.db"
    MODEL_PATH = "enhanced_model.pth"
    CACHE_DIR = "cache"
    MAX_API_CALLS_PER_MINUTE = 5  # NewsAPI rate limit
    SYMBOLS = ["SPY", "AAPL", "MSFT", "GOOGL", "AMZN"]  
    QUERY_MAP = {  # Map symbols to relevant news queries
        "SPY": "S&P 500 OR stock market",
        "AAPL": "Apple OR AAPL",
        "MSFT": "Microsoft OR MSFT",
        "GOOGL": "Google OR Alphabet OR GOOGL",
        "AMZN": "Amazon OR AMZN"
    }

    BACKTEST_DAYS = 365
    NEWS_API_KEY = "e0985a3944d94773af15680c3d7c0fe8"  
    POLYGON_API_KEY = "3fFPtOWzOWW3tCDu8Mk3WzrCmHS654JY" 

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
    handlers=[
        logging.FileHandler("trading_system.log"),
        logging.StreamHandler()
    ]
)

# --------------------------
# DATA PERSISTENCE LAYER
# --------------------------
class DataStore:
    def __init__(self):
        os.makedirs(Config.CACHE_DIR, exist_ok=True)
        self._init_db()
        
    def _init_db(self):
        with sqlite3.connect(Config.DB_PATH) as conn:
            conn.execute("""
                CREATE TABLE IF NOT EXISTS market_data (
                    timestamp DATETIME PRIMARY KEY,
                    symbol TEXT,
                    open REAL,
                    high REAL,
                    low REAL,
                    close REAL,
                    volume REAL,
                    indicators BLOB
                )
            """)
            conn.execute("""
                CREATE TABLE IF NOT EXISTS news_sentiment (
                    timestamp DATETIME PRIMARY KEY,
                    query TEXT,
                    score REAL,
                    raw_data BLOB
                )
            """)
            conn.execute("""
                CREATE TABLE IF NOT EXISTS predictions (
                    timestamp DATETIME PRIMARY KEY,
                    symbol TEXT,
                    action TEXT,
                    confidence REAL,
                    price REAL,
                    metadata BLOB
                )
            """)
    
    def save_market_data(self, df: pd.DataFrame, symbol: str):
        try:
            with sqlite3.connect(Config.DB_PATH) as conn:
                for idx, row in df.iterrows():
                    timestamp = idx.to_pydatetime() if isinstance(idx, pd.Timestamp) else idx
                    indicators = {
                        'MA_50': row.get('MA_50'),
                        'MA_200': row.get('MA_200'),
                        'RSI': row.get('RSI'),
                        'MACD': row.get('MACD'),
                        'ATR': row.get('ATR')
                    }
                    conn.execute(
                        """INSERT OR REPLACE INTO market_data 
                        (timestamp, symbol, open, high, low, close, volume, indicators) 
                        VALUES (?, ?, ?, ?, ?, ?, ?, ?)""",
                        (timestamp, symbol, row['open'], row['high'], row['low'], row['close'], 
                         row['volume'], pickle.dumps(indicators))
                    )
        except Exception as e:
            logging.error(f"Error saving market data: {e}")

    def save_sentiment(self, query: str, score: float, raw_data: dict):
        try:
            with sqlite3.connect(Config.DB_PATH) as conn:
                conn.execute(
                    """INSERT OR REPLACE INTO news_sentiment VALUES (?,?,?,?)""",
                    (datetime.now(), query, score, pickle.dumps(raw_data))
                )
        except Exception as e:
            logging.error(f"Error saving sentiment: {e}")

    def save_prediction(self, prediction: dict):
        try:
            with sqlite3.connect(Config.DB_PATH) as conn:
                conn.execute(
                    """INSERT OR REPLACE INTO predictions VALUES (?,?,?,?,?,?)""",
                    (prediction['timestamp'], prediction['symbol'], prediction['action'],
                     prediction['confidence'], prediction['price'], pickle.dumps(prediction))
                )
        except Exception as e:
            logging.error(f"Error saving prediction: {e}")

    def get_historical_data(self, symbol: str, days: int) -> Optional[pd.DataFrame]:
        try:
            with sqlite3.connect(Config.DB_PATH) as conn:
                cutoff = datetime.now() - timedelta(days=days)
                df = pd.read_sql(
                    """SELECT * FROM market_data 
                    WHERE symbol = ? AND timestamp >= ? 
                    ORDER BY timestamp""", 
                    conn, params=(symbol, cutoff)
                )
                if not df.empty:
                    df.set_index('timestamp', inplace=True)
                return df
        except Exception as e:
            logging.error(f"Error loading historical data: {e}")
        return None

# --------------------------
# RATE-LIMITED API CLIENT
# --------------------------
class RateLimitedAPIClient:
    def __init__(self):
        self.last_call_time = 0
        self.call_count = 0
        self.news_api_key = Config.NEWS_API_KEY
        if not self.news_api_key:
            logging.warning("NEWS_API_KEY not set! Using placeholder")

    def _check_rate_limit(self):
        now = time.time()
        if now - self.last_call_time > 60:
            self.call_count = 0
            self.last_call_time = now
        
        if self.call_count >= Config.MAX_API_CALLS_PER_MINUTE:
            sleep_time = 60 - (now - self.last_call_time)
            logging.warning(f"Rate limit reached. Sleeping for {sleep_time:.1f} seconds")
            time.sleep(max(1, sleep_time))
            self.call_count = 0
            self.last_call_time = time.time()
        
        self.call_count += 1

    @retry(
        stop=stop_after_attempt(3),
        wait=wait_exponential(multiplier=1, min=4, max=10),
        retry=retry_if_exception_type((requests.exceptions.RequestException,))
    )
    def get_news_data(self, query: str) -> dict:
        self._check_rate_limit()
        try:
            url = f"https://newsapi.org/v2/everything?q={query}&apiKey={self.news_api_key}&language=en&sortBy=publishedAt"
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            return response.json()
        except Exception as e:
            logging.error(f"News API request failed: {e}")
            raise

# --------------------------
# REAL-TIME DATA FETCHER WITH ERROR RECOVERY
# --------------------------
class RobustDataFetcher:
    
    def __init__(self, data_store: DataStore):
        self.data_store = data_store
        self.api_client = RateLimitedAPIClient()
        self.tokenizer = AutoTokenizer.from_pretrained("yiyanghkust/finbert-tone")
        # Fixed model type
        self.sentiment_model = AutoModelForSequenceClassification.from_pretrained("yiyanghkust/finbert-tone")
        self._init_cache()
        polygon_api_key = Config.POLYGON_API_KEY
        if polygon_api_key:
            self.polygon = RESTClient(polygon_api_key)
        else:
            logging.warning("POLYGON_API_KEY not set! Polygon functionality disabled.")
            self.polygon = None

    def _init_cache(self):
        self.market_data_cache = pd.DataFrame()
        self.sentiment_cache = {}

    @retry(
        stop=stop_after_attempt(3),
        wait=wait_exponential(multiplier=1, min=2, max=10),
        retry=retry_if_exception_type(Exception)
    )
    def get_market_data(self, symbol: str) -> pd.DataFrame:
        if self.polygon is None:
            logging.error("Polygon client not initialized")
            return pd.DataFrame()
            
        try:
            # Get current date and previous day
            today = datetime.now().date()
            start_date = today - timedelta(days=1)
            
            # Fetch minute bars from Polygon
            aggs = []
            for agg in self.polygon.list_aggs(
                ticker=symbol,
                multiplier=1,
                timespan="minute",
                from_=start_date.strftime("%Y-%m-%d"),
                to=today.strftime("%Y-%m-%d"),
                limit=50000
            ):
                aggs.append(agg)
                
            if not aggs:
                logging.warning("Empty data from Polygon, trying cache")
                cached = self.data_store.get_historical_data(symbol, 1)
                if cached is not None:
                    return cached
                raise ValueError("No market data available")

            # Convert to DataFrame
            data = []
            for agg in aggs:
                dt = datetime.fromtimestamp(agg.timestamp / 1000, tz=timezone.utc).replace(tzinfo=None)
                data.append({
                    "timestamp": dt,
                    "open": agg.open,
                    "high": agg.high,
                    "low": agg.low,
                    "close": agg.close,
                    "volume": agg.volume
                })
                
            df = pd.DataFrame(data)
            if df.empty:
                raise ValueError("Empty DataFrame from Polygon")
                
            df.set_index('timestamp', inplace=True)
            
            # Store to cache and database
            self.market_data_cache = df
            self.data_store.save_market_data(df, symbol)
            
            return df.dropna()
            
        except Exception as e:
            logging.error(f"Market data fetch failed: {e}")
            raise

    def get_news_sentiment(self, symbol: str) -> float:
        query = Config.QUERY_MAP.get(symbol, symbol)
        cache_key = f"{symbol}_{datetime.now().date()}"
        if cache_key in self.sentiment_cache:
            return self.sentiment_cache[cache_key]
            
        try:
            news_data = self.api_client.get_news_data(query)
            sentiments = []
            
            for article in news_data.get('articles', [])[:5]:
                text = f"{article['title']}. {article['description']}"
                sentiment = self._analyze_sentiment(text)
                sentiments.append(sentiment)
            
            score = np.mean(sentiments) if sentiments else 0.0
            self.sentiment_cache[cache_key] = score
            self.data_store.save_sentiment(query, score, news_data)
            
            return score
        except Exception as e:
            logging.error(f"News sentiment failed, using cached value: {e}")
            return self.sentiment_cache.get(cache_key, 0.0)

    def _analyze_sentiment(self, text: str) -> float:
        try:
            inputs = self.tokenizer(text, return_tensors="pt", truncation=True, max_length=512)
            outputs = self.sentiment_model(**inputs)
            # Sentiment calculation
            predictions = F.softmax(outputs.logits, dim=-1)
            # Positive (0) minus Negative (1) score
            return (predictions[0][0] - predictions[0][1]).item()
        except Exception as e:
            logging.error(f"Sentiment analysis failed: {e}")
            return 0.0

# --------------------------
# TECHNICAL INDICATORS (NUMBA-OPTIMIZED)
# --------------------------
@jit(nopython=True)
def compute_rsi(close_prices, period=14):
    """Compute Relative Strength Index (RSI)"""
    deltas = np.diff(close_prices)
    gains = np.maximum(deltas, 0)
    losses = -np.minimum(deltas, 0)

    avg_gain = np.mean(gains[:period])
    avg_loss = np.mean(losses[:period])
    rsi_values = np.zeros_like(close_prices)

    for i in range(period, len(close_prices)):
        avg_gain = (avg_gain * (period - 1) + gains[i - 1]) / period
        avg_loss = (avg_loss * (period - 1) + losses[i - 1]) / period
        rs = avg_gain / avg_loss if avg_loss != 0 else np.inf
        rsi_values[i] = 100 - (100 / (1 + rs))

    return rsi_values

@jit(nopython=True)
def compute_macd(close_prices, fast=12, slow=26, signal=9):
    """Compute MACD with signal line"""
    fast_ema = np.zeros_like(close_prices)
    slow_ema = np.zeros_like(close_prices)
    
    # Initial EMA values
    fast_ema[fast-1] = np.mean(close_prices[:fast])
    slow_ema[slow-1] = np.mean(close_prices[:slow])
    
    # Calculating here EMAs
    for i in range(fast, len(close_prices)):
        fast_ema[i] = fast_ema[i-1] * (1 - 2/(fast+1)) + close_prices[i] * 2/(fast+1)
    
    for i in range(slow, len(close_prices)):
        slow_ema[i] = slow_ema[i-1] * (1 - 2/(slow+1)) + close_prices[i] * 2/(slow+1)
    
    macd = fast_ema - slow_ema
    signal_line = np.zeros_like(close_prices)
    
    # calculating here signal line
    signal_line[slow+signal-2] = np.mean(macd[slow-1:slow+signal-1])
    for i in range(slow+signal-1, len(close_prices)):
        signal_line[i] = signal_line[i-1] * (1 - 2/(signal+1)) + macd[i] * 2/(signal+1)
    
    return macd, signal_line

@jit(nopython=True)
def compute_bollinger_bands(close_prices, window=20, num_std=2):
    """Compute Bollinger Bands"""
    rolling_mean = np.zeros_like(close_prices)
    rolling_std = np.zeros_like(close_prices)
    
    for i in range(window-1, len(close_prices)):
        rolling_mean[i] = np.mean(close_prices[i-window+1:i+1])
        rolling_std[i] = np.std(close_prices[i-window+1:i+1])
    
    upper_band = rolling_mean + (rolling_std * num_std)
    lower_band = rolling_mean - (rolling_std * num_std)
    
    return upper_band, rolling_mean, lower_band

@jit(nopython=True)
def compute_atr(high_prices, low_prices, close_prices, window=14):
    """Compute Average True Range"""
    tr = np.zeros_like(close_prices)
    atr = np.zeros_like(close_prices)
    
    for i in range(1, len(close_prices)):
        tr[i] = max(
            high_prices[i] - low_prices[i],
            abs(high_prices[i] - close_prices[i-1]),
            abs(low_prices[i] - close_prices[i-1])
        )
    
    atr[window-1] = np.mean(tr[1:window])
    for i in range(window, len(close_prices)):
        atr[i] = (atr[i-1] * (window-1) + tr[i]) / window
    
    return atr

def compute_fibonacci_retracement(high, low):
    """Calculate Fibonacci retracement levels"""
    diff = high - low
    return {
        '0.236': high - diff * 0.236,
        '0.382': high - diff * 0.382,
        '0.5': high - diff * 0.5,
        '0.618': high - diff * 0.618,
        '0.786': high - diff * 0.786
    }

# --------------------------
# NEURAL NETWORK MODEL
# --------------------------
class TimeSeriesAttention(nn.Module):
    def __init__(self, hidden_size):
        super().__init__()
        self.query = nn.Linear(hidden_size, hidden_size)
        self.key = nn.Linear(hidden_size, hidden_size)
        self.value = nn.Linear(hidden_size, hidden_size)
        
    def forward(self, x):
        Q = self.query(x)
        K = self.key(x)
        V = self.value(x)
        
        attention_scores = torch.matmul(Q, K.transpose(-2, -1)) / torch.sqrt(torch.tensor(x.size(-1)))
        attention_weights = F.softmax(attention_scores, dim=-1)
        return torch.matmul(attention_weights, V)

class EnhancedPredictor(nn.Module):
    def __init__(self, input_size=11, hidden_size=128):
        super().__init__()
        self.bilstm = nn.LSTM(input_size, hidden_size, batch_first=True, bidirectional=True)
        self.attention = TimeSeriesAttention(hidden_size * 2)
        self.linear = nn.Sequential(
            nn.Linear(hidden_size * 2, 64),
            nn.ReLU(),
            nn.Linear(64, 1)
        )
        
    def forward(self, x):
        x, _ = self.bilstm(x)
        x = self.attention(x)
        x = torch.mean(x, dim=1)  # Global average pooling
        return self.linear(x)

def predict_movement(model, data, sentiment_score):
    """Make prediction using model with sentiment"""
    seq_length = 30
    features = ['open', 'high', 'low', 'close', 'volume', 'RSI', 
               'MACD', 'ATR', 'Kalman_Filter', 'Fib_0.618', 'Cycle_Strength']
    sequence = data[-seq_length:][features].values
    
    # Normalize sentiment score to [0, 1] range
    sentiment_weight = (sentiment_score + 1) / 2
    
    with torch.no_grad():
        numerical_pred = model(torch.tensor(sequence).float().unsqueeze(0))
        # Combine numerical prediction with sentiment
        prediction = numerical_pred.item() * 0.7 + sentiment_weight * 0.3
    
    return prediction

# --------------------------
# BACKTESTING FRAMEWORK
# --------------------------
class Backtester:
    
    def __init__(self, data_store: DataStore):
        self.data_store = data_store
        self.model = self._load_model()
        
    def _load_model(self) -> Optional[nn.Module]:
        try:
            model_path =  Path(Config.MODEL_PATH)
            if model_path.exists():
                model = EnhancedPredictor(input_size=11)
                model.load_state_dict(torch.load(Config.MODEL_PATH, map_location=torch.device('cpu')))
                model.eval()
                return model
            else:
                logging.warning("Model not found. Using Dummy model.")
                return EnhancedPredictor() #This function creates untrained model
        except Exception as e:
            logging.error(f"Model loading failed: {e}")
            return None

    def run_backtest(self, symbols: List[str], days: int) -> Dict[str, Dict]:
        results = {}
        if not self.model:
            return {"error": "Model not available"}
            
        for symbol in symbols:
            data = self.data_store.get_historical_data(symbol, days)
            if data is None or data.empty:
                results[symbol] = {"error": f"No historical data for {symbol}"}
                continue
                
            symbol_results = {
                'initial_balance': 10000,
                'balance': 10000,
                'positions': 0,
                'trades': []
            }
            
            # Simulate trading
            for i in range(30, len(data)):  # Skip first 30 days for indicators
                window = data.iloc[:i]
                processed = technical_analysis(window)
                
                if processed.empty:
                    continue
                    
                # Mock sentiment for backtest
                sentiment = 0
                
                try:
                    prediction = predict_movement(self.model, processed, sentiment)
                    self._execute_trade(prediction, processed.iloc[-1], symbol_results, data.index[i])
                except Exception as e:
                    logging.error(f"Backtest error for {symbol} at {window.index[-1]}: {e}")
                    
            # Calculate metrics
            symbol_results = self._calculate_metrics(symbol_results)
            self._plot_results(symbol_results, symbol)
            results[symbol] = symbol_results
            
        return results
        
    def _execute_trade(self, prediction: float, current_data: pd.Series, results: Dict, timestamp):
        price = current_data['close']
        # More sensitive threshold with hold zone
        if prediction > 0.55:
            action = "BUY"
        elif prediction < 0.45:
            action = "SELL"
        else:
            action = "HOLD"
        
        # Simple trading strategy
        if action == "BUY" and results['positions'] <= 0:
            # Buy with 10% of balance
            amount = results['balance'] * 0.1
            shares = amount / price
            results['positions'] += shares
            results['balance'] -= amount
            results['trades'].append({
                "time": timestamp,  # Use proper timestamp
                "action": "BUY",
                "price": price,
                "shares": shares,
                "balance": results['balance']
            })
        elif action == "SELL" and results['positions'] > 0:
            # Sell all positions
            amount = results['positions'] * price
            results['balance'] += amount
            results['trades'].append({
                "time": timestamp,  # Use proper timestamp
                "action": "SELL",
                "price": price,
                "shares": results['positions'],
                "balance": results['balance']
            })
            results['positions'] = 0
            
    def _calculate_metrics(self, results: Dict) -> Dict:
        if not results['trades']:
            return results
            
        # Calculate returns
        returns = []
        balance = results['initial_balance']
        for trade in results['trades']:
            returns.append((trade['balance'] - balance) / balance)
            balance = trade['balance']
            
        # Sharpe ratio (simplified)
        if len(returns) > 1:
            results['sharpe_ratio'] = np.mean(returns) / np.std(returns)
            
        # Max drawdown
        peak = results['initial_balance']
        max_dd = 0
        for trade in results['trades']:
            if trade['balance'] > peak:
                peak = trade['balance']
            dd = (peak - trade['balance']) / peak
            if dd > max_dd:
                max_dd = dd
        results['max_drawdown'] = max_dd
        
        return results
        
    def _plot_results(self, results: Dict, symbol: str):
        if not results['trades']:
            return
            
        times = [t['time'] for t in results['trades']]
        balances = [t['balance'] for t in results['trades']]
        
        plt.figure(figsize=(12, 6))
        plt.plot(times, balances)
        plt.title(f"Backtest Results for {symbol}")
        plt.xlabel("Time")
        plt.ylabel("Portfolio Value")
        plt.grid(True)
        plt.savefig(f"backtest_results_{symbol}.png")
        plt.close()

# --------------------------
# TRADING SYSTEM WITH ALL FEATURES
# --------------------------
class TradingSystem:
    def __init__(self):
        self.data_store = DataStore()
        self.fetcher = RobustDataFetcher(self.data_store)
        self.model = self._load_model()
        self.backtester = Backtester(self.data_store)
        
    def _load_model(self) -> Optional[nn.Module]:
        try:
            if Path(Config.MODEL_PATH).exists():
                model = EnhancedPredictor(input_size=11)
                model.load_state_dict(torch.load(Config.MODEL_PATH))
                model.eval()
                return model
            logging.warning("Model file not found")
            return None
        except Exception as e:
            logging.error(f"Model loading failed: {e}")
            return None

    def update_and_predict(self):
        """Main trading loop"""
        recommendations = []
        for symbol in Config.SYMBOLS:
            try:
                # Get fresh data for this symbol
                market_data = self.fetcher.get_market_data(symbol)
                if market_data.empty:
                    logging.error(f"No market data received for {symbol}")
                    continue
                    
                # Process data
                processed_data = technical_analysis(market_data)
                if processed_data.empty:
                    logging.error(f"Technical analysis failed for {symbol}")
                    continue
                    
                # Get sentiment for this symbol
                sentiment = self.fetcher.get_news_sentiment(symbol)
                
                # Make prediction
                prediction = predict_movement(self.model, processed_data, sentiment)
                
                # Generate and save recommendation
                recommendation = self._generate_recommendation(symbol, processed_data, prediction, sentiment)
                self.data_store.save_prediction(recommendation)
                self._log_recommendation(recommendation)
                recommendations.append(recommendation)
            except Exception as e:
                logging.error(f"Trading system error for {symbol}: {e}")
        return recommendations

    def run_backtest(self, days: int = Config.BACKTEST_DAYS):
        """Run backtest and return results"""
        return self.backtester.run_backtest(Config.SYMBOLS, days)

    def _generate_recommendation(self, symbol: str, data: pd.DataFrame, prediction: float, sentiment: float) -> Dict:
        """Generate trading recommendation with all indicators"""
        last_row = data.iloc[-1]
        fib_status = "Support" if last_row['close'] > last_row['Fib_0.618'] else "Resistance"
        
        return {
            "symbol": symbol,
            "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            "price": last_row['close'],
            "action": "BUY" if prediction > 0.55 else "SELL" if prediction < 0.45 else "HOLD",
            "confidence": abs(round((prediction - 0.5) * 200, 1)),
            "sentiment_score": round(sentiment, 2),
            "indicators": {
                "trend": "Bullish" if last_row['MA_50'] > last_row['MA_200'] else "Bearish",
                "momentum": "Strong" if last_row['MACD'] > last_row['MACD_Signal'] else "Weak",
                "volatility": "High" if last_row['close'] > last_row['Upper_Band'] else "Low",
                "fibonacci": fib_status,
                "cycles": f"{last_row['Cycle_Strength']:.2f}",
                "rsi": f"{last_row['RSI']:.1f}",
                "atr": f"{last_row['ATR']:.2f}"
            }
        }

    # Moved inside the class
    def _log_recommendation(self, recommendation: Dict):
        """Log recommendation in readable format"""
        logging.info("\n=== REAL-TIME TRADING RECOMMENDATION ===")
        logging.info(f"SYMBOL:            {recommendation['symbol']}")
        logging.info(f"TIME:              {recommendation['timestamp']}")
        logging.info(f"PRICE:             {recommendation['price']:.2f}")
        logging.info(f"ACTION:            {recommendation['action']} (Confidence: {recommendation['confidence']}%)")
        logging.info(f"NEWS SENTIMENT:    {recommendation['sentiment_score']:.2f}")
        
        logging.info("\nTECHNICAL INDICATORS:")
        for name, value in recommendation['indicators'].items():
            logging.info(f"{name.upper():<15}: {value}")
        
        logging.info("\n" + "="*50)

# Moved outside the class
def technical_analysis(data):
    data['MA_50'] = data['close'].rolling(50).mean()
    data['MA_200'] = data['close'].rolling(200).mean()
    data['RSI'] = compute_rsi(data['close'].values)
    data['MACD'], data['MACD_Signal'] = compute_macd(data['close'].values)
    data['Upper_Band'], _, data['Lower_Band'] = compute_bollinger_bands(data['close'].values)
    data['ATR'] = compute_atr(data['high'].values, data['low'].values, data['close'].values)
    
    # Kalman Filter
    data['Kalman_Filter'] = data['close'].ewm(span=10).mean()

    # Fibonacci Levels 
    recent_high = data['high'].rolling(50).max().iloc[-1]
    recent_low = data['low'].rolling(50).min().iloc[-1]
    data['Fib_0.618'] = recent_high - (recent_high - recent_low) * 0.618

    # Cycle Strength 
    closes = data['close'].values
    if len(closes) > 10:
        fft_result = np.abs(fft(closes)[1:len(closes)//2])
        data['Cycle_Strength'] = np.max(fft_result) if len(fft_result) > 0 else 0
    else:
        data['Cycle_Strength'] = 0
        
    # Handle NaN values
    data = data.fillna(method='ffill').fillna(0)
    return data

# --------------------------
# MAIN EXECUTION
# --------------------------
if __name__ == "__main__":
   
   #Creating model path file
    model_path = Path(Config.MODEL_PATH)
    if not model_path.exists():
        logging.info("Creating initial model file...")
        dummy_model = EnhancedPredictor(input_size=11) 
        torch.save(dummy_model.state_dict(), Config.MODEL_PATH)
        logging.info(f"Created initial model at {Config.MODEL_PATH}")

    system = TradingSystem()
    
    # Initial backtest
    logging.info("Running initial backtest...")
    backtest_results = system.run_backtest()
    logging.info(f"Backtest completed with Sharpe ratio: {backtest_results.get('sharpe_ratio', 0):.2f}")
    
    # Initial update
    system.update_and_predict()
    
    # Schedule updates every minute
    schedule.every(1).minutes.do(system.update_and_predict)
    
    logging.info("Real-time trading system started. Press Ctrl+C to exit.")
    
    try:
        while True:
            schedule.run_pending()
            time.sleep(1)
    except KeyboardInterrupt:
        logging.info("System stopped by user")