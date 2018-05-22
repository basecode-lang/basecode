#include "compiler.h"
#include "lexer.h"

namespace basecode {

    compiler::compiler(
            size_t heap_size,
            size_t stack_size): _terp(heap_size, stack_size),
                                _global_scope(nullptr, nullptr) {
    }

    compiler::~compiler() {
    }

    void compiler::build_scope_tree(
            result& r,
            basecode::scope* scope,
            const ast_node_shared_ptr& node) {
        if (scope == nullptr || node == nullptr)
            return;

        for (auto& child_node : node->children) {
            if (child_node->type == ast_node_types_t::basic_block) {
                auto child_scope = scope->add_child_scope(child_node);
                build_scope_tree(r, child_scope, child_node);
            } else {
                // XXX: need to recurse down lhs and rhs nodes
            }
        }
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
            formatter.format_graph_viz();

            build_scope_tree(r, &_global_scope, program_node);
        }
        return !r.is_failed();
    }

    bool compiler::compile_file(result& r, const std::filesystem::path& path) {

        return !r.is_failed();
    }

}