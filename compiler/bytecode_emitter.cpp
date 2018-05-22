#include <parser/lexer.h>
#include "bytecode_emitter.h"

namespace basecode::compiler {

    bytecode_emitter::bytecode_emitter(
            size_t heap_size,
            size_t stack_size): _terp(heap_size, stack_size),
                                _global_scope(nullptr, nullptr) {
    }

    bytecode_emitter::~bytecode_emitter() {
    }

    void bytecode_emitter::build_scope_tree(
            common::result& r,
            compiler::scope* scope,
            const syntax::ast_node_shared_ptr& node) {
        if (scope == nullptr || node == nullptr)
            return;

        for (auto& child_node : node->children) {
            if (child_node->type == syntax::ast_node_types_t::basic_block) {
                auto child_scope = scope->add_child_scope(child_node);
                build_scope_tree(r, child_scope, child_node);
            } else {
                // XXX: need to recurse down lhs and rhs nodes
            }
        }
    }

    bool bytecode_emitter::initialize(common::result& r) {
        return _terp.initialize(r);
    }

    bool bytecode_emitter::compile(common::result& r, std::istream& input) {
        compile_stream(r, input);
        return !r.is_failed();
    }

    bool bytecode_emitter::compile_stream(common::result& r, std::istream& input) {
        syntax::parser alpha_parser(input);
        auto program_node = alpha_parser.parse(r);
        if (program_node != nullptr) {
            syntax::ast_formatter formatter(program_node);
            formatter.format_graph_viz();

            build_scope_tree(r, &_global_scope, program_node);
        }
        return !r.is_failed();
    }

    bool bytecode_emitter::compile_file(common::result& r, const std::filesystem::path& path) {

        return !r.is_failed();
    }

}