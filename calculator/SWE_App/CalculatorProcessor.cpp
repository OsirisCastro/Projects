#include "CalculatorProcessor.h"
#include <cmath>
#include <stack>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <wx/math.h>

CalculatorProcessor* CalculatorProcessor::GetInstance() {
    static CalculatorProcessor instance;
    return &instance;
}

double CalculatorProcessor::Calculate(const std::string& expression) {
    try {
        return EvaluateExpression(expression);
    }
    catch (const std::exception& e) {

        throw;
    }
}

double CalculatorProcessor::EvaluateExpression(const std::string& expression) {
    try {
        std::vector<std::string> tokens = Tokenize(expression);
        std::vector<std::string> postfix = ConvertToPostfix(tokens);
        return EvaluatePostfix(postfix);
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error evaluating expression: ") + e.what());
    }
}

std::vector<std::string> CalculatorProcessor::Tokenize(const std::string& expression) {
    std::vector<std::string> tokens;
    std::string token;
    bool lastToken = true;
    try {
        for (size_t i = 0; i < expression.size(); ++i) {
            char c = expression[i];
            if (isspace(c)) continue;

            if (isdigit(c) || c == '.') {
                token += c;
                lastToken = false;
            }
            else {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }

                if (isalpha(c)) {
                    while (i < expression.size() && isalpha(expression[i])) {
                        token += expression[i++];
                    }
                    --i;
                    tokens.push_back(token);
                    token.clear();
                }
                else {
                    if (c == '-' && lastToken) {
                        token = "-";
                    }
                    else {
                        tokens.push_back(std::string(1, c));
                    }
                }
                lastToken = true;
            }
        }

        if (!token.empty()) tokens.push_back(token);
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error tokenizing expression: ") + e.what());
    }

    return tokens;
}

bool CalculatorProcessor::IsOperator(const std::string& token) {
    return token == "+" || token == "-" || token == "*" || token == "/" || token == "%";
}

bool CalculatorProcessor::IsTrigFunction(const std::string& token) {
    return token == "sin" || token == "cos" || token == "tan";
}

int CalculatorProcessor::GetPrecedence(const std::string& op) {
    if (op == "+" || op == "-") return 1;
    if (op == "*" || op == "/" || op == "%") return 2;
    if (IsTrigFunction(op)) return 3;
    return 0;
}

std::vector<std::string> CalculatorProcessor::ConvertToPostfix(const std::vector<std::string>& tokens) {
    std::vector<std::string> output;
    std::stack<std::string> ops;

    try {
        for (const auto& token : tokens) {
            if (isdigit(token[0]) || (token.size() > 1 && isdigit(token[1]))) {
                output.push_back(token);
            }
            else if (IsTrigFunction(token)) {
                ops.push(token);
            }
            else if (IsOperator(token)) {
                while (!ops.empty() && IsOperator(ops.top()) && GetPrecedence(ops.top()) >= GetPrecedence(token)) {
                    output.push_back(ops.top());
                    ops.pop();
                }
                ops.push(token);
            }
            else if (token == "(") {
                ops.push(token);
            }
            else if (token == ")") {
                while (!ops.empty() && ops.top() != "(") {
                    output.push_back(ops.top());
                    ops.pop();
                }
                if (!ops.empty()) {
                    ops.pop();
                }
                if (!ops.empty() && IsTrigFunction(ops.top())) {
                    output.push_back(ops.top());
                    ops.pop();
                }
            }
        }

        while (!ops.empty()) {
            output.push_back(ops.top());
            ops.pop();
        }
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error converting to postfix: ") + e.what());
    }

    return output;
}

double CalculatorProcessor::EvaluatePostfix(const std::vector<std::string>& postfix) {
    std::stack<double> values;

    try {
        for (const auto& token : postfix) {
            if (isdigit(token[0]) || (token.size() > 1 && isdigit(token[1]))) {
                values.push(std::stod(token));
            }
            else if (IsOperator(token)) {
                if (values.size() < 2) throw std::runtime_error("Insufficient operands for the operator " + token);
                double right = values.top(); values.pop();
                double left = values.top(); values.pop();
                values.push(PerformOperation(token, left, right));
            }
            else if (IsTrigFunction(token)) {
                if (values.empty()) throw std::runtime_error("Insufficient operands for the function " + token);
                double value = values.top(); values.pop();
                values.push(PerformOperation(token, value));
            }
            else {
                throw std::runtime_error("Unknown token: " + token);
            }
        }

        if (values.size() != 1) throw std::runtime_error("Invalid expression");
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error evaluating the postfix expression: ") + e.what());
    }

    return values.top();
}

double CalculatorProcessor::PerformOperation(const std::string& op, double left, double right) {
    if (op == "+") return left + right;
    if (op == "-") return left - right;
    if (op == "*") return left * right;
    if (op == "/") {
        if (right == 0) throw std::runtime_error("Division by zero!");
        return left / right;
    }
    if (op == "%") {
        if (right == 0) throw std::runtime_error("Division by zero!");
        return std::fmod(left, right);
    }
    if (IsTrigFunction(op)) {
        if (op == "sin") return std::sin(left);
        if (op == "cos") return std::cos(left);
        if (op == "tan") return std::tan(left);
    }

    throw std::runtime_error("Unknown operation or trigonometric function");
}
