/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);
static Statement* parseStatementFromRemainder(const std::string &remainder);
static Statement* parseStatementWithKeyword(const std::string &keyword, const std::string &remainder);
static bool isNumberToken(const std::string &tok);
static bool isReserved(const std::string &name);
static void runProgram(Program &program, EvalState &state, int startLine = -1);

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

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);
    if (!scanner.hasMoreTokens()) return;
    std::string first = scanner.nextToken();
    TokenType tt = scanner.getTokenType(first);

    if (tt == NUMBER) {
        int lineNumber = stringToInteger(first);
        // remainder of the line (including possible statement). Use original line to preserve formatting
        // Determine if there is any content after the first token
        // Find first occurrence of first token and skip following space
        size_t pos = line.find(first);
        std::string remainder;
        if (pos != std::string::npos) {
            size_t after = pos + first.size();
            if (after < line.size() && line[after] == ' ') remainder = line.substr(after + 1);
            else if (after < line.size()) remainder = line.substr(after);
        }
        if (trim(remainder).empty()) {
            program.removeSourceLine(lineNumber);
            return;
        }
        program.addSourceLine(lineNumber, line);
        Statement *stmt = parseStatementFromRemainder(remainder);
        program.setParsedStatement(lineNumber, stmt);
        return;
    }

    // Immediate mode commands
    std::string keyword = toUpperCase(first);
    // Build remainder string (raw)
    size_t firstPos = line.find(first);
    std::string remainder;
    if (firstPos != std::string::npos) {
        size_t after = firstPos + first.size();
        if (after < line.size() && line[after] == ' ') remainder = line.substr(after + 1);
        else if (after < line.size()) remainder = line.substr(after);
    }

    if (keyword == "REM") {
        // immediate comment: no-op
        return;
    } else if (keyword == "LET" || keyword == "PRINT" || keyword == "INPUT" || keyword == "END" ||
               keyword == "GOTO" || keyword == "IF") {
        Statement *stmt = parseStatementWithKeyword(keyword, remainder);
        stmt->execute(state, program);
        delete stmt;
        return;
    } else if (keyword == "RUN") {
        int start = program.getFirstLineNumber();
        runProgram(program, state, start);
        return;
    } else if (keyword == "LIST") {
        for (auto it = program.getFirstLineNumber(); it != -1; it = program.getNextLineNumber(it)) {
            std::cout << program.getSourceLine(it) << std::endl;
        }
        return;
    } else if (keyword == "CLEAR") {
        program.clear();
        state.Clear();
        return;
    } else if (keyword == "QUIT") {
        std::exit(0);
    } else if (keyword == "HELP") {
        // optional; ignore or print simple help
        return;
    }

    error("SYNTAX ERROR");
}

static Statement* parseStatementFromRemainder(const std::string &remainder) {
    TokenScanner s;
    s.ignoreWhitespace();
    s.scanNumbers();
    s.setInput(remainder);
    if (!s.hasMoreTokens()) return new RemStatement("");
    std::string kw = s.nextToken();
    std::string keyword = toUpperCase(kw);
    // Build the remainder after keyword from original remainder string
    size_t pos = remainder.find(kw);
    std::string after;
    if (pos != std::string::npos) {
        size_t a = pos + kw.size();
        if (a < remainder.size() && remainder[a] == ' ') after = remainder.substr(a + 1);
        else if (a < remainder.size()) after = remainder.substr(a);
    }
    return parseStatementWithKeyword(keyword, after);
}

static Statement* parseStatementWithKeyword(const std::string &keyword, const std::string &remainder) {
    if (keyword == "REM") {
        return new RemStatement(remainder);
    }
    if (keyword == "LET") {
        TokenScanner s; s.ignoreWhitespace(); s.scanNumbers(); s.setInput(remainder);
        if (!s.hasMoreTokens()) error("SYNTAX ERROR");
        std::string var = s.nextToken();
        if (!s.hasMoreTokens() || s.nextToken() != "=") error("SYNTAX ERROR");
        // Parse expression from rest
        std::string rest;
        int pos = s.getPosition();
        (void) pos; // not used; reconstruct from remainder by skipping prefix
        // Simpler: create a new scanner from the remaining characters after "var = "
        size_t varPos = remainder.find(var);
        size_t afterVar = (varPos == std::string::npos) ? 0 : (varPos + var.size());
        size_t eqPos = remainder.find('=', afterVar);
        std::string exprStr;
        if (eqPos != std::string::npos) {
            size_t afterEq = eqPos + 1;
            if (afterEq < remainder.size() && remainder[afterEq] == ' ') exprStr = remainder.substr(afterEq + 1);
            else exprStr = remainder.substr(afterEq);
        }
        TokenScanner es; es.ignoreWhitespace(); es.scanNumbers(); es.setInput(exprStr);
        Expression *exp = parseExp(es);
        return new LetStatement(var, exp);
    }
    if (keyword == "PRINT") {
        TokenScanner es; es.ignoreWhitespace(); es.scanNumbers(); es.setInput(remainder);
        Expression *exp = parseExp(es);
        return new PrintStatement(exp);
    }
    if (keyword == "INPUT") {
        TokenScanner s; s.ignoreWhitespace(); s.scanNumbers(); s.setInput(remainder);
        if (!s.hasMoreTokens()) error("SYNTAX ERROR");
        std::string var = s.nextToken();
        return new InputStatement(var);
    }
    if (keyword == "END") {
        // Should have no trailing tokens considered as error (optional)
        return new EndStatement();
    }
    if (keyword == "GOTO") {
        TokenScanner s; s.ignoreWhitespace(); s.scanNumbers(); s.setInput(remainder);
        if (!s.hasMoreTokens()) error("SYNTAX ERROR");
        std::string ln = s.nextToken();
        if (!isNumberToken(ln)) error("SYNTAX ERROR");
        return new GotoStatement(stringToInteger(ln));
    }
    if (keyword == "IF") {
        // Parse: <exp1> <op> <exp2> THEN <line>
        // Strategy: tokenize until THEN, find comparison op among <, >, = possibly combined with next token
        TokenScanner s; s.ignoreWhitespace(); s.scanNumbers(); s.setInput(remainder);
        std::vector<std::string> toks;
        std::string tok;
        while (s.hasMoreTokens()) {
            tok = s.nextToken();
            if (toUpperCase(tok) == "THEN") break;
            toks.push_back(tok);
        }
        if (toUpperCase(tok) != "THEN") error("SYNTAX ERROR");
        if (!s.hasMoreTokens()) error("SYNTAX ERROR");
        std::string targetTok = s.nextToken();
        if (!isNumberToken(targetTok)) error("SYNTAX ERROR");
        int target = stringToInteger(targetTok);

        // find comparator in toks
        int opIndex = -1; std::string op;
        for (int i = 0; i < (int)toks.size(); ++i) {
            const std::string &t = toks[i];
            if (t == "<" || t == ">" || t == "=") {
                opIndex = i; op = t;
                // check two-char combinations
                if (i + 1 < (int)toks.size()) {
                    if (t == "<" && toks[i+1] == ">") { op = "<>"; }
                    else if ((t == "<" || t == ">") && toks[i+1] == "=") { op += "="; }
                }
                break;
            }
        }
        if (opIndex == -1) error("SYNTAX ERROR");
        int rhsStart = opIndex + 1;
        if (op.size() == 2) rhsStart = opIndex + 2;
        if (rhsStart > (int)toks.size()-1 || opIndex == 0) error("SYNTAX ERROR");
        // build strings
        std::string lhsStr, rhsStr;
        for (int i = 0; i < opIndex; ++i) {
            if (i) lhsStr += ' ';
            lhsStr += toks[i];
        }
        for (int i = rhsStart; i < (int)toks.size(); ++i) {
            if (i > rhsStart) rhsStr += ' ';
            rhsStr += toks[i];
        }
        TokenScanner ls; ls.ignoreWhitespace(); ls.scanNumbers(); ls.setInput(lhsStr);
        TokenScanner rs; rs.ignoreWhitespace(); rs.scanNumbers(); rs.setInput(rhsStr);
        Expression *lhs = readE(ls);
        if (ls.hasMoreTokens()) error("SYNTAX ERROR");
        Expression *rhs = readE(rs);
        if (rs.hasMoreTokens()) error("SYNTAX ERROR");
        return new IfStatement(lhs, op, rhs, target);
    }
    error("SYNTAX ERROR");
    return nullptr;
}

static bool isNumberToken(const std::string &tok) {
    if (tok.empty()) return false;
    for (char c : tok) if (!std::isdigit(static_cast<unsigned char>(c)) && !(c=='-'||c=='+')) return false;
    return true;
}

static bool isReserved(const std::string &name) {
    std::string u = toUpperCase(name);
    return u == "REM" || u == "LET" || u == "PRINT" || u == "INPUT" || u == "END" ||
           u == "GOTO" || u == "IF" || u == "THEN" || u == "RUN" || u == "LIST" ||
           u == "CLEAR" || u == "QUIT" || u == "HELP";
}

static void runProgram(Program &program, EvalState &state, int startLine) {
    int pc = startLine;
    while (pc != -1) {
        Statement *stmt = program.getParsedStatement(pc);
        // If no parsed statement is available but there is a line, just advance
        if (stmt) {
            try {
                // Clear any previous request
                program.consumeRequest();
                stmt->execute(state, program);
            } catch (ErrorException &ex) {
                std::cout << ex.getMessage() << std::endl;
                break;
            }
        }
        if (program.hasRequest()) {
            int req = program.consumeRequest();
            if (req == -1) break; // END
            else pc = req;
        } else {
            pc = program.getNextLineNumber(pc);
        }
    }
}
