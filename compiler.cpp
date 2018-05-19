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
        parser alpha_parser(input);
        auto program_node = alpha_parser.parse(r);
        if (program_node != nullptr) {
            ast_formatter formatter(program_node);
            formatter.format();
        }
        return !r.is_failed();
    }

}