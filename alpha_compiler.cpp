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

        token_t token;
        while (alpha_lexer.next(token)) {
//            fmt::print(
//                "\ntoken.type = {}\n"
//                    "token.line = {}\n"
//                    "token.column = {}\n"
//                    "token.value = {}\n"
//                    "token.radix = {}\n",
//                token.name(),
//                token.line,
//                token.column,
//                token.value,
//                token.radix);
        }

        return !r.is_failed();
    }

}