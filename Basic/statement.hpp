/*
 * File: statement.h
 * -----------------
 * This file defines the Statement abstract type.  In
 * the finished version, this file will also specify subclasses
 * for each of the statement types.  As you design your own
 * version of this class, you should pay careful attention to
 * the exp.h interface, which is an excellent model for
 * the Statement class hierarchy.
 */

#ifndef _statement_h
#define _statement_h

#include <string>
#include <sstream>
#include "evalstate.hpp"
#include "exp.hpp"
#include "Utils/tokenScanner.hpp"
#include "program.hpp"
#include "parser.hpp"
#include "Utils/error.hpp"
#include "Utils/strlib.hpp"

class Program;

/*
 * Class: Statement
 * ----------------
 * This class is used to represent a statement in a program.
 * The model for this class is Expression in the exp.h interface.
 * Like Expression, Statement is an abstract class with subclasses
 * for each of the statement and command types required for the
 * BASIC interpreter.
 */

class Statement {

public:

/*
 * Constructor: Statement
 * ----------------------
 * The base class constructor is empty.  Each subclass must provide
 * its own constructor.
 */

    Statement();

/*
 * Destructor: ~Statement
 * Usage: delete stmt;
 * -------------------
 * The destructor deallocates the storage for this expression.
 * It must be declared virtual to ensure that the correct subclass
 * destructor is called when deleting a statement.
 */

    virtual ~Statement();

/*
 * Method: execute
 * Usage: stmt->execute(state);
 * ----------------------------
 * This method executes a BASIC statement.  Each of the subclasses
 * defines its own execute method that implements the necessary
 * operations.  As was true for the expression evaluator, this
 * method takes an EvalState object for looking up variables or
 * controlling the operation of the interpreter.
 */

    virtual void execute(EvalState &state, Program &program) = 0;

};

// REM statement (no-op)
class RemStatement : public Statement {
public:
    explicit RemStatement(const std::string &comment) : comment(comment) {}
    void execute(EvalState &state, Program &program) override;
private:
    std::string comment;
};

// LET statement
class LetStatement : public Statement {
public:
    LetStatement(const std::string &name, Expression *exp) : name(name), exp(exp) {}
    ~LetStatement() override { delete exp; }
    void execute(EvalState &state, Program &program) override;
private:
    std::string name;
    Expression *exp;
};

// PRINT statement
class PrintStatement : public Statement {
public:
    explicit PrintStatement(Expression *exp) : exp(exp) {}
    ~PrintStatement() override { delete exp; }
    void execute(EvalState &state, Program &program) override;
private:
    Expression *exp;
};

// INPUT statement
class InputStatement : public Statement {
public:
    explicit InputStatement(const std::string &name) : name(name) {}
    void execute(EvalState &state, Program &program) override;
private:
    std::string name;
};

// END statement
class EndStatement : public Statement {
public:
    EndStatement() = default;
    void execute(EvalState &state, Program &program) override;
};

// GOTO statement
class GotoStatement : public Statement {
public:
    explicit GotoStatement(int target) : target(target) {}
    void execute(EvalState &state, Program &program) override;
private:
    int target;
};

// IF ... THEN ... statement
class IfStatement : public Statement {
public:
    IfStatement(Expression *lhs, std::string op, Expression *rhs, int target)
            : lhs(lhs), op(std::move(op)), rhs(rhs), target(target) {}
    ~IfStatement() override { delete lhs; delete rhs; }
    void execute(EvalState &state, Program &program) override;
private:
    Expression *lhs;
    std::string op;
    Expression *rhs;
    int target;
};


/*
 * The remainder of this file must consists of subclass
 * definitions for the individual statement forms.  Each of
 * those subclasses must define a constructor that parses a
 * statement from a scanner and a method called execute,
 * which executes that statement.  If the private data for
 * a subclass includes data allocated on the heap (such as
 * an Expression object), the class implementation must also
 * specify its own destructor method to free that memory.
 */

#endif
