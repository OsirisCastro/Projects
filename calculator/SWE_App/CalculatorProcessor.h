#pragma once
#include <string>
#include <vector>

class CalculatorProcessor {
public:

    static CalculatorProcessor* GetInstance();
    double Calculate(const std::string& expression);

    CalculatorProcessor() = default;
    double EvaluateExpression(const std::string& expression);
    std::vector<std::string> Tokenize(const std::string& expression);
    std::vector<std::string> ConvertToPostfix(const std::vector<std::string>& tokens);
    double EvaluatePostfix(const std::vector<std::string>& postfix);
    double PerformOperation(const std::string& op, double left, double right = 0);
    bool IsOperator(const std::string& token);
    bool IsTrigFunction(const std::string& token);
    int GetPrecedence(const std::string& op);
};

