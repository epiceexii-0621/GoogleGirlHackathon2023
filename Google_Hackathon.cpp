#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "picosat.h" // SAT solver library

using namespace std;

// Function to convert a Boolean expression to Conjunctive Normal Form (CNF)
vector<vector<int>> toCNF(const string& expr, unordered_map<string, int>& varMap) {
    vector<vector<int>> cnfClauses;

    size_t pos = 0;
    while (pos < expr.length()) {
        if (expr[pos] == ' ') {
            pos++; // Skip whitespaces
            continue;
        }
        if (expr[pos] == '(') {
            // Find the closing parenthesis
            size_t closingPos = expr.find(')', pos + 1);
            string subExpr = expr.substr(pos + 1, closingPos - pos - 1);
            pos = closingPos + 1;

            vector<vector<int>> subClauses = toCNF(subExpr, varMap);
            for (const auto& clause : subClauses) {
                cnfClauses.push_back(clause);
            }
        }
        else {
            // Find the next token
            size_t spacePos = expr.find(' ', pos);
            size_t tokenLen = (spacePos != string::npos) ? (spacePos - pos) : (expr.length() - pos);
            string token = expr.substr(pos, tokenLen);
            pos = spacePos;

            if (token[0] == '~') {
                // Unary operator (NOT)
                string operand = token.substr(1);
                int varId = varMap[operand];
                cnfClauses.push_back({ -varId });
            }
            else if (token == "&" || token == "|" || token == "^") {
                // Binary operator (AND, OR, XOR)
                char op = token[0];

                // Find the left operand
                vector<int> leftOperands;
                while (pos < expr.length() && (expr[pos] == ' ' || expr[pos] == '(')) {
                    pos++;
                }
                spacePos = expr.find(' ', pos);
                tokenLen = (spacePos != string::npos) ? (spacePos - pos) : (expr.length() - pos);
                string leftOperand = expr.substr(pos, tokenLen);
                if (leftOperand[0] == '(') {
                    leftOperand = leftOperand.substr(1);
                }
                int leftVarId = varMap[leftOperand];
                leftOperands.push_back(leftVarId);
                pos = spacePos;

                // Find the right operand
                vector<int> rightOperands;
                while (pos < expr.length() && (expr[pos] == ' ' || expr[pos] == '(')) {
                    pos++;
                }
                spacePos = expr.find(' ', pos);
                tokenLen = (spacePos != string::npos) ? (spacePos - pos) : (expr.length() - pos);
                string rightOperand = expr.substr(pos, tokenLen);
                if (rightOperand[rightOperand.length() - 1] == ')') {
                    rightOperand = rightOperand.substr(0, rightOperand.length() - 1);
                }
                int rightVarId = varMap[rightOperand];
                rightOperands.push_back(rightVarId);
                pos = spacePos;

                // Add the CNF clauses for the binary operator
                if (op == '&') {
                    for (const auto& leftVar : leftOperands) {
                        for (const auto& rightVar : rightOperands) {
                            cnfClauses.push_back({ -leftVar, -rightVar });
                        }
                    }
                }
                else if (op == '|') {
                    cnfClauses.push_back(leftOperands);
                    cnfClauses.push_back(rightOperands);
                }
                else if (op == '^') {
                    for (const auto& leftVar : leftOperands) {
                        for (const auto& rightVar : rightOperands) {
                            cnfClauses.push_back({ -leftVar, -rightVar });
                            cnfClauses.push_back({ leftVar, rightVar });
                        }
                    }
                }
            }
            else {
                // Input variable or net_<alphanumeric string>
                if (varMap.find(token) == varMap.end()) {
                    int varId = picosat_inc_max_var();
                    varMap[token] = varId;
                }
            }
        }
    }

    return cnfClauses;
}

// Function to find a satisfying assignment using SAT solver
vector<int> findSatisfyingAssignment(const vector<vector<int>>& cnfClauses) {
    int varsCount = picosat_variables();

    for (const auto& clause : cnfClauses) {
        for (const auto& literal : clause) {
            picosat_add(literal);
        }
        picosat_add(0); // Terminate the clause
    }

    int res = picosat_sat(1); // Solve the SAT problem

    if (res == PICOSAT_SATISFIABLE) {
        vector<int> assignment(varsCount + 1);
        for (int i = 1; i <= varsCount; i++) {
            assignment[i] = picosat_deref(i);
        }
        return assignment;
    }

    return {};
}

// Function to print the input vector and expected output value to output.txt
void printOutput(const vector<string>& inputVars, const vector<int>& inputValues, int outputValue) {
    ofstream outputFile("output.txt");
    if (outputFile.is_open()) {
        outputFile << "[";
        for (size_t i = 0; i < inputVars.size(); i++) {
            outputFile << inputVars[i] << ": " << inputValues[i];
            if (i != inputVars.size() - 1) {
                outputFile << ", ";
            }
        }
        outputFile << "], Z = " << outputValue << endl;

        outputFile.close();
    }
    else {
        cout << "Unable to open the output file." << endl;
    }
}

int main() {
    // Circuit file (replace this with the actual circuit representation)
}