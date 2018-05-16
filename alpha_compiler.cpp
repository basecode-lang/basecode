#include "alpha_compiler.h"
#include "lexer.h"

namespace basecode {

    alpha_compiler::alpha_compiler(size_t heap_size): _terp(heap_size) {
    }

    alpha_compiler::~alpha_compiler() {
    }

    bool alpha_compiler::initialize(result& r) {
        return _terp.initialize(r);
    }

    bool alpha_compiler::compile(result& r, std::istream& input) {
        compile_stream(r, input);
        return !r.is_failed();
    }

    bool alpha_compiler::compile_stream(result& r, std::istream& input) {
        lexer alpha_lexer(input);

        while (alpha_lexer.has_next()) {
            auto token = alpha_lexer.next();
            fmt::print("token.type = {}\n", token.name());
            fmt::print("token.value = {}\n", token.value);
            if (token.is_numeric())
                fmt::print("token.radix = {}\n", token.radix);
            fmt::print("token.line = {}\n", token.line);
            fmt::print("token.column = {}\n\n", token.column);
        }

        return !r.is_failed();
    }

}