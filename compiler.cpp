#include "compiler.h"
#include "lexer.h"

namespace basecode {

    compiler::compiler(size_t heap_size): _terp(heap_size) {
    }

    compiler::~compiler() {
    }

    bool compiler::initialize(result& r) {
        return _terp.initialize(r);
    }

    bool compiler::compile(result& r, std::istream& input) {
        compile_stream(r, input);
        return !r.is_failed();
    }

    bool compiler::compile_stream(result& r, std::istream& input) {
        lexer alpha_lexer(input);

        token_t token;
        while (alpha_lexer.next(token)) {
            fmt::print(
                "\ntoken.type = {}\n"
                    "token.line = {}\n"
                    "token.column = {}\n"
                    "token.value = {}\n"
                    "token.radix = {}\n"
                    "token.number_type = {}\n",
                token.name(),
                token.line,
                token.column,
                token.value,
                token.radix,
                static_cast<uint16_t>(token.number_type));
        }

        return !r.is_failed();
    }

}