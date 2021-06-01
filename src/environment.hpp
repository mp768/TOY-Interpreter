#pragma once
#include <map>
#include <string>
#include <memory>

#include "scanner.hpp"
#include "expr.hpp"
#include "ToyCallable.hpp"

namespace toy {

struct EnvironmentError : public std::exception { };

EnvironmentError environment_error(std::string message) {
    error(message);
    return EnvironmentError();
}

struct Environment {
    std::map<std::string, Expr *> values;
    std::map<std::string, ToyCallable> callables;
    Environment* enclosing;

    Environment() { enclosing = nullptr; }
    Environment(Environment* v_enclosing) { enclosing = v_enclosing; }

    void define(std::string name, Expr* val) {
        values[name] = val;
    } 

    void define(std::string name, ToyCallable val) {
        callables[name] = val;
    }

    void assign(Token name, Expr* val) {

        auto it = values.find(name.lexeme);
        if (it != values.end()) {
            values[name.lexeme] = val;
            return;
        }

        if (enclosing != nullptr) {
            enclosing->assign(name, val);
            return;
        }

        error("Undefined variable '" + name.lexeme + "'.");
        throw environment_error("Undefined variable '" + name.lexeme + "'.");
    }

    std::map<std::string, Expr*> get_values() {
        return values;
    }

    Expr* get(Token name) {

        auto it = values.find(name.lexeme);
        if (it != values.end())
            return values[name.lexeme];

        if (enclosing != nullptr) {
            return enclosing->get(name);
        }

        error("Undefined variable '" + name.lexeme + "'.");
        throw environment_error("Undefined variable '" + name.lexeme + "'.");
    }

    ToyCallable get_callable(Token name) {

        auto it = callables.find(name.lexeme);
        if (it != callables.end())
            return callables[name.lexeme];

        if (enclosing) {
            return enclosing->get_callable(name);
        }

        error("Undefined callable '" + name.lexeme + "'.");
        throw environment_error("Undefined callable '" + name.lexeme + "'.");
    }
};
} // end of toy namespace
