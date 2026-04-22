/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"
#include <iostream>


/* Implementation of the Statement class */

int stringToInteger(std::string str);

Statement::Statement() = default;

Statement::~Statement() = default;

// RemStatement implementation
RemStatement::RemStatement(const std::string& comment) : comment(comment) {}

void RemStatement::execute(EvalState &state, Program &program) {
    // REM statements do nothing
}

// LetStatement implementation
LetStatement::LetStatement(const std::string& var, Expression *exp) : var(var), exp(exp) {}

LetStatement::~LetStatement() {
    delete exp;
}

void LetStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    state.setValue(var, value);
}

// PrintStatement implementation
PrintStatement::PrintStatement(Expression *exp) : exp(exp) {}

PrintStatement::~PrintStatement() {
    delete exp;
}

void PrintStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    std::cout << value << std::endl;
}

// InputStatement implementation
InputStatement::InputStatement(const std::string& var) : var(var) {}

void InputStatement::execute(EvalState &state, Program &program) {
    while (true) {
        std::cout << " ? " << std::flush;
        std::string line;

        if (!std::getline(std::cin, line)) {
            // EOF or error
            error("INPUT ERROR");
        }

        // Trim whitespace
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            std::cout << "INVALID NUMBER" << std::endl;
            continue;
        }
        size_t end = line.find_last_not_of(" \t");
        std::string trimmed = line.substr(start, end - start + 1);

        // Use TokenScanner to check if it's a valid number
        TokenScanner scanner;
        scanner.scanNumbers();
        scanner.setInput(trimmed);

        if (scanner.hasMoreTokens()) {
            std::string token = scanner.nextToken();
            if (scanner.getTokenType(token) == NUMBER && !scanner.hasMoreTokens()) {
                int value = stringToInteger(token);
                state.setValue(var, value);
                return;
            }
        }
        std::cout << "INVALID NUMBER" << std::endl;
    }
}

// EndStatement implementation
EndStatement::EndStatement() {}

void EndStatement::execute(EvalState &state, Program &program) {
    exit(0);
}

// GotoStatement implementation
GotoStatement::GotoStatement(int lineNumber) : lineNumber(lineNumber) {}

void GotoStatement::execute(EvalState &state, Program &program) {
    // This will be handled by the main interpreter loop
    // We need to throw an exception or use another mechanism
    throw ErrorException("GOTO " + std::to_string(lineNumber));
}

// IfStatement implementation
IfStatement::IfStatement(Expression *exp1, const std::string& op, Expression *exp2, int targetLine)
    : exp1(exp1), exp2(exp2), op(op), targetLine(targetLine) {}

IfStatement::~IfStatement() {
    delete exp1;
    delete exp2;
}

void IfStatement::execute(EvalState &state, Program &program) {
    int val1 = exp1->eval(state);
    int val2 = exp2->eval(state);
    bool condition = false;

    if (op == "=") {
        condition = (val1 == val2);
    } else if (op == "<") {
        condition = (val1 < val2);
    } else if (op == ">") {
        condition = (val1 > val2);
    } else if (op == "<=") {
        condition = (val1 <= val2);
    } else if (op == ">=") {
        condition = (val1 >= val2);
    } else if (op == "<>") {
        condition = (val1 != val2);
    }

    if (condition) {
        throw ErrorException("GOTO " + std::to_string(targetLine));
    }
}

// RunStatement implementation
RunStatement::RunStatement() {}

void RunStatement::execute(EvalState &state, Program &program) {
    int currentLine = program.getFirstLineNumber();

    while (currentLine != -1) {
        Statement *stmt = program.getParsedStatement(currentLine);
        if (stmt == nullptr) {
            // Parse the statement if not already parsed
            std::string line = program.getSourceLine(currentLine);
            TokenScanner scanner;
            scanner.ignoreWhitespace();
            scanner.scanNumbers();
            scanner.setInput(line);

            // Skip line number
            scanner.nextToken();

            std::string command = scanner.nextToken();
            // Parse and create statement
            // This is a simplified version - full parsing will be in Basic.cpp
        }

        try {
            stmt->execute(state, program);
            currentLine = program.getNextLineNumber(currentLine);
        } catch (ErrorException &ex) {
            std::string msg = ex.getMessage();
            if (msg.substr(0, 5) == "GOTO ") {
                int targetLine = stringToInteger(msg.substr(5));
                if (program.getSourceLine(targetLine) == "") {
                    error("LINE NUMBER ERROR");
                }
                currentLine = targetLine;
            } else {
                throw;
            }
        }
    }
}

// ListStatement implementation
ListStatement::ListStatement() {}

void ListStatement::execute(EvalState &state, Program &program) {
    int currentLine = program.getFirstLineNumber();
    while (currentLine != -1) {
        std::cout << program.getSourceLine(currentLine) << std::endl;
        currentLine = program.getNextLineNumber(currentLine);
    }
}

// ClearStatement implementation
ClearStatement::ClearStatement() {}

void ClearStatement::execute(EvalState &state, Program &program) {
    program.clear();
    state.Clear();
}

// QuitStatement implementation
QuitStatement::QuitStatement() {}

void QuitStatement::execute(EvalState &state, Program &program) {
    exit(0);
}
