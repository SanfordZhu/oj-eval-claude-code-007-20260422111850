/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include <cstdlib>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "statement.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);

/* Main program */

int main() {
    EvalState state;
    Program program;
    //cout << "Stub implementation of BASIC" << endl;
    while (true) {
        try {
            std::string input;
            getline(std::cin, input);
            if (input.empty())
                continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

Expression *readE(TokenScanner &scanner, int prec);

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    // Check if this is a line number
    std::string token = scanner.nextToken();
    if (scanner.getTokenType(token) == NUMBER) {
        int lineNumber = stringToInteger(token);

        // Check if there's more content after the line number
        if (scanner.hasMoreTokens()) {
            // This is a program line
            program.addSourceLine(lineNumber, line);

            // Parse the statement
            std::string command = scanner.nextToken();
            command = toLowerCase(command);

            Statement *stmt = nullptr;

            if (command == "rem") {
                // For REM, just store an empty statement since comments are ignored
                stmt = new RemStatement("");
            } else if (command == "let") {
                std::string var = scanner.nextToken();
                // Check if variable name is a keyword
                std::string lowerVar = toLowerCase(var);
                if (lowerVar == "rem" || lowerVar == "let" || lowerVar == "print" ||
                    lowerVar == "input" || lowerVar == "end" || lowerVar == "goto" ||
                    lowerVar == "if" || lowerVar == "then" || lowerVar == "run" ||
                    lowerVar == "list" || lowerVar == "clear" || lowerVar == "quit" ||
                    lowerVar == "help") {
                    error("SYNTAX ERROR");
                }
                if (scanner.nextToken() != "=") {
                    error("SYNTAX ERROR");
                }
                Expression *exp = parseExp(scanner);
                stmt = new LetStatement(var, exp);
            } else if (command == "print") {
                Expression *exp = parseExp(scanner);
                stmt = new PrintStatement(exp);
            } else if (command == "input") {
                std::string var = scanner.nextToken();
                // Check if variable name is a keyword
                std::string lowerVar = toLowerCase(var);
                if (lowerVar == "rem" || lowerVar == "let" || lowerVar == "print" ||
                    lowerVar == "input" || lowerVar == "end" || lowerVar == "goto" ||
                    lowerVar == "if" || lowerVar == "then" || lowerVar == "run" ||
                    lowerVar == "list" || lowerVar == "clear" || lowerVar == "quit" ||
                    lowerVar == "help") {
                    error("SYNTAX ERROR");
                }
                stmt = new InputStatement(var);
            } else if (command == "end") {
                stmt = new EndStatement();
            } else if (command == "goto") {
                int targetLine = stringToInteger(scanner.nextToken());
                stmt = new GotoStatement(targetLine);
            } else if (command == "if") {
                // Parse first expression manually to avoid consuming THEN
                Expression *exp1 = nullptr;
                Expression *exp2 = nullptr;
                std::string op;

                // Build first expression, stopping at comparison operator
                TokenScanner exp1Scanner;
                exp1Scanner.ignoreWhitespace();
                exp1Scanner.scanNumbers();

                // Copy tokens until we hit a comparison operator
                std::vector<std::string> tokens;
                while (scanner.hasMoreTokens()) {
                    std::string token = scanner.nextToken();
                    std::string lowerToken = toLowerCase(token);
                    if (lowerToken == "=" || lowerToken == "<" || lowerToken == ">" ||
                        lowerToken == "<=" || lowerToken == ">=" || lowerToken == "<>") {
                        op = token;
                        break;
                    }
                    tokens.push_back(token);
                }

                // Parse first expression from tokens
                if (!tokens.empty()) {
                    std::string exp1Str;
                    for (const auto& token : tokens) {
                        if (!exp1Str.empty()) exp1Str += " ";
                        exp1Str += token;
                    }
                    exp1Scanner.setInput(exp1Str);
                    exp1 = parseExp(exp1Scanner);
                }

                // Build second expression, stopping at THEN
                tokens.clear();
                while (scanner.hasMoreTokens()) {
                    std::string token = scanner.nextToken();
                    std::string lowerToken = toLowerCase(token);
                    if (lowerToken == "then") {
                        break;
                    }
                    tokens.push_back(token);
                }

                // Parse second expression from tokens
                if (!tokens.empty()) {
                    TokenScanner exp2Scanner;
                    exp2Scanner.ignoreWhitespace();
                    exp2Scanner.scanNumbers();
                    std::string exp2Str;
                    for (const auto& token : tokens) {
                        if (!exp2Str.empty()) exp2Str += " ";
                        exp2Str += token;
                    }
                    exp2Scanner.setInput(exp2Str);
                    exp2 = parseExp(exp2Scanner);
                }

                // Get target line number
                int targetLine = stringToInteger(scanner.nextToken());

                stmt = new IfStatement(exp1, op, exp2, targetLine);
            } else {
                error("SYNTAX ERROR");
            }

            program.setParsedStatement(lineNumber, stmt);
        } else {
            // Empty line number - remove the line
            program.removeSourceLine(lineNumber);
        }
    } else {
        // This is a direct command
        std::string command = token;
        command = toLowerCase(command);

        Statement *stmt = nullptr;

        if (command == "run") {
            // Execute the program
            state.Clear();
            int currentLine = program.getFirstLineNumber();

            while (currentLine != -1) {
                Statement *lineStmt = program.getParsedStatement(currentLine);
                if (lineStmt == nullptr) {
                    // This shouldn't happen if parsing was done correctly
                    error("SYNTAX ERROR");
                }

                try {
                    lineStmt->execute(state, program);
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
        } else if (command == "list") {
            int currentLine = program.getFirstLineNumber();
            while (currentLine != -1) {
                std::cout << program.getSourceLine(currentLine) << std::endl;
                currentLine = program.getNextLineNumber(currentLine);
            }
        } else if (command == "clear") {
            program.clear();
            state.Clear();
        } else if (command == "quit") {
            exit(0);
        } else if (command == "help") {
            std::cout << "Not implemented" << std::endl;
        } else if (command == "rem") {
            // REM in direct mode does nothing
        } else if (command == "let") {
            std::string var = scanner.nextToken();
            // Check if variable name is a keyword
            std::string lowerVar = toLowerCase(var);
            if (lowerVar == "rem" || lowerVar == "let" || lowerVar == "print" ||
                lowerVar == "input" || lowerVar == "end" || lowerVar == "goto" ||
                lowerVar == "if" || lowerVar == "then" || lowerVar == "run" ||
                lowerVar == "list" || lowerVar == "clear" || lowerVar == "quit" ||
                lowerVar == "help") {
                error("SYNTAX ERROR");
            }
            if (scanner.nextToken() != "=") {
                error("SYNTAX ERROR");
            }
            Expression *exp = parseExp(scanner);
            int value = exp->eval(state);
            state.setValue(var, value);
            delete exp;
        } else if (command == "print") {
            Expression *exp = parseExp(scanner);
            int value = exp->eval(state);
            std::cout << value << std::endl;
            delete exp;
        } else if (command == "input") {
            std::string var = scanner.nextToken();
            // Check if variable name is a keyword
            std::string lowerVar = toLowerCase(var);
            if (lowerVar == "rem" || lowerVar == "let" || lowerVar == "print" ||
                lowerVar == "input" || lowerVar == "end" || lowerVar == "goto" ||
                lowerVar == "if" || lowerVar == "then" || lowerVar == "run" ||
                lowerVar == "list" || lowerVar == "clear" || lowerVar == "quit" ||
                lowerVar == "help") {
                error("SYNTAX ERROR");
            }
            while (true) {
                std::cout << " ? " << std::flush;
                std::string line;
                std::getline(std::cin, line);

                // Trim whitespace
                size_t start = line.find_first_not_of(" \t");
                if (start == std::string::npos) {
                    std::cout << "INVALID NUMBER" << std::endl;
                    continue;
                }
                size_t end = line.find_last_not_of(" \t");
                std::string trimmed = line.substr(start, end - start + 1);

                // Use TokenScanner to validate
                TokenScanner scanner;
                scanner.scanNumbers();
                scanner.setInput(trimmed);

                if (scanner.hasMoreTokens() && scanner.getTokenType(scanner.nextToken()) == NUMBER && !scanner.hasMoreTokens()) {
                    int value = stringToInteger(trimmed);
                    state.setValue(var, value);
                    break;
                }
                std::cout << "INVALID NUMBER" << std::endl;
            }
        } else if (command == "end") {
            exit(0);
        } else {
            error("SYNTAX ERROR");
        }
    }
}

