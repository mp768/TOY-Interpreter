#pragma once
#include <vector>
#include <any>
#include <functional>

#include "scanner.hpp"
//#include "interpreter.hpp"

namespace toy {

struct NO_VALUE {
    NO_VALUE(int y) : h(y) {};

    int h;
};

struct Interpreter;

using FuncType = std::function<std::any(Interpreter&, const std::vector<std::any>&)>;

struct ToyCallable {
    ToyCallable(int arity, FuncType func) : argument_count(arity), native_function_call(func) {};

    ToyCallable() : argument_count(0) {}

    ToyCallable(Stmt v_declaration) {
        if (v_declaration.type != stmt_function_t) {
            error("Cannot pass a stmt that is not a function!");
            return;
        }

        declaration = v_declaration;
        argument_count = static_cast<int>(declaration.params.size());
    }

    int arity() {
        return argument_count;
    }

    std::any call(Interpreter& interpreter, const std::vector<std::any> arguments) {
        if (declaration.type == stmt_none_t) {
            return native_function_call(interpreter, arguments);
        }

        return std::any(NO_VALUE(0)); // return because for some reason I can't use Interpreter's methods and have to do the code inside the interpreter method.
    }

    int argument_count;
    FuncType native_function_call;

    Stmt declaration;
};
} // end of toy namespace