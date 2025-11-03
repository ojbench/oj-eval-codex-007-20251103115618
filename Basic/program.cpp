/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"



Program::Program() = default;

Program::~Program() {
    clear();
}

void Program::clear() {
    for (auto &kv : parsedStmts) {
        delete kv.second;
    }
    parsedStmts.clear();
    sourceLines.clear();
    nextLineRequest = -2;
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    // Replace or insert source line text
    sourceLines[lineNumber] = line;
    // If there was a parsed statement before, delete it (will be reset by caller)
    auto it = parsedStmts.find(lineNumber);
    if (it != parsedStmts.end()) {
        delete it->second;
        parsedStmts.erase(it);
    }
}

void Program::removeSourceLine(int lineNumber) {
    sourceLines.erase(lineNumber);
    auto it = parsedStmts.find(lineNumber);
    if (it != parsedStmts.end()) {
        delete it->second;
        parsedStmts.erase(it);
    }
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = sourceLines.find(lineNumber);
    if (it == sourceLines.end()) return "";
    return it->second;
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    if (sourceLines.find(lineNumber) == sourceLines.end()) {
        // No such line; free provided stmt and raise error
        delete stmt;
        error("LINE NUMBER ERROR");
    }
    auto it = parsedStmts.find(lineNumber);
    if (it != parsedStmts.end()) {
        delete it->second;
        it->second = stmt;
    } else {
        parsedStmts.emplace(lineNumber, stmt);
    }
}

Statement *Program::getParsedStatement(int lineNumber) {
    auto it = parsedStmts.find(lineNumber);
    if (it == parsedStmts.end()) return nullptr;
    return it->second;
}

int Program::getFirstLineNumber() {
    if (sourceLines.empty()) return -1;
    return sourceLines.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = sourceLines.upper_bound(lineNumber);
    if (it == sourceLines.end()) return -1;
    return it->first;
}

void Program::requestNextLine(int lineNumber) { nextLineRequest = lineNumber; }

void Program::requestEnd() { nextLineRequest = -1; }

bool Program::hasRequest() const { return nextLineRequest != -2; }

int Program::consumeRequest() {
    int v = nextLineRequest;
    nextLineRequest = -2;
    return v;
}
