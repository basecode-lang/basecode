#include "alpha_compiler.h"

namespace basecode {

    alpha_compiler::alpha_compiler(size_t heap_size): _terp(heap_size),
                                                      _evaluator(this) {
    }

    alpha_compiler::~alpha_compiler() {
    }

    bool alpha_compiler::initialize(result& r) {
        return _terp.initialize(r);
    }

    bool alpha_compiler::compile(result& r, const parser_input_t& input) {
        compile_stream(r, input);
        return !r.is_failed();
    }

    bool alpha_compiler::compile_stream(result& r, const parser_input_t& input) {
        auto program_node = _parser.parse(input);
        auto parser_result = _parser.result();
        if (parser_result.is_failed()) {
            for (const auto& msg : parser_result.messages())
                r.add_message(
                    msg.code(),
                    msg.message(),
                    msg.details(),
                    msg.is_error());
            return false;
        }

        if (program_node == nullptr)
            return !r.is_failed();

        if (!_evaluator.evaluate_program(r, program_node)) {

        }

        return !r.is_failed();
    }

}