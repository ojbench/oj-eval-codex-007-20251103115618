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
#include <cstdint>


/* Implementation of the Statement class */

int stringToInt(std::string str);

Statement::Statement() = default;

Statement::~Statement() = default;

static bool isReservedKeyword(const std::string &name) {
    std::string u = toUpperCase(name);
    return u == "REM" || u == "LET" || u == "PRINT" || u == "INPUT" || u == "END" ||
           u == "GOTO" || u == "IF" || u == "THEN" || u == "RUN" || u == "LIST" ||
           u == "CLEAR" || u == "QUIT" || u == "HELP";
}

void RemStatement::execute(EvalState &state, Program &program) {
    (void) state; (void) program; // no-op
}

void LetStatement::execute(EvalState &state, Program &program) {
    (void) program;
    if (isReservedKeyword(name)) error("SYNTAX ERROR");
    int v = exp->eval(state);
    state.setValue(name, v);
}

void PrintStatement::execute(EvalState &state, Program &program) {
    (void) program;
    int v = exp->eval(state);
    std::cout << v << std::endl;
}

static bool parseInteger(const std::string &s, int &out) {
    if (s.empty()) return false;
    size_t i = 0; bool neg = false;
    if (s[0] == '+' || s[0] == '-') { neg = (s[0] == '-'); i = 1; }
    if (i >= s.size()) return false;
    long long val = 0;
    for (; i < s.size(); ++i) {
        if (s[i] < '0' || s[i] > '9') return false;
        val = val * 10 + (s[i] - '0');
        if (val > 2147483648LL) return false; // guard
    }
    long long signedVal = neg ? -val : val;
    if (signedVal < INT32_MIN || signedVal > INT32_MAX) return false;
    out = static_cast<int>(signedVal);
    return true;
}

void InputStatement::execute(EvalState &state, Program &program) {
    (void) program;
    if (isReservedKeyword(name)) error("SYNTAX ERROR");
    while (true) {
        std::cout << " ? ";
        std::cout.flush();
        std::string line;
        if (!std::getline(std::cin, line)) {
            error("INVALID NUMBER");
        }
        // trim
        // Use simple trimming of spaces and tabs
        size_t l = 0, r = line.size();
        while (l < r && (line[l] == ' ' || line[l] == '\t')) ++l;
        while (r > l && (line[r-1] == ' ' || line[r-1] == '\t')) --r;
        std::string t = line.substr(l, r - l);
        int v = 0;
        if (parseInteger(t, v)) {
            state.setValue(name, v);
            break;
        } else {
            std::cout << "INVALID NUMBER" << std::endl;
        }
    }
}

void EndStatement::execute(EvalState &state, Program &program) {
    (void) state;
    program.requestEnd();
}

void GotoStatement::execute(EvalState &state, Program &program) {
    (void) state;
    program.requestNextLine(target);
}

void IfStatement::execute(EvalState &state, Program &program) {
    int lv = lhs->eval(state);
    int rv = rhs->eval(state);
    bool cond = false;
    if (op == "=") cond = (lv == rv);
    else if (op == "<") cond = (lv < rv);
    else if (op == ">") cond = (lv > rv);
    else if (op == "<=") cond = (lv <= rv);
    else if (op == ">=") cond = (lv >= rv);
    else if (op == "<>") cond = (lv != rv);
    else error("SYNTAX ERROR");
    if (cond) program.requestNextLine(target);
}
