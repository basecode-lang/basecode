#include "constant_expression_evaluator.h"

namespace basecode::compiler {

    constant_expression_evaluator::constant_expression_evaluator(
        compiler::scope* scope) : _scope(scope) {
    }

    constant_expression_evaluator::~constant_expression_evaluator() {
    }

    syntax::ast_node_shared_ptr constant_expression_evaluator::evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node) {
        if (node == nullptr)
            return nullptr;

        switch (node->type) {
            case syntax::ast_node_types_t::program:
            case syntax::ast_node_types_t::basic_block:
            case syntax::ast_node_types_t::namespace_statement: {
                for (auto const& block_child : node->children)
                    evaluate(r, block_child);

                break;
            }
            case syntax::ast_node_types_t::statement: {
                if (node->lhs != nullptr)
                    evaluate(r, node->lhs);

                if (node->rhs != nullptr)
                    evaluate(r, node->rhs);

                break;
            }
            case syntax::ast_node_types_t::assignment: {
                break;
            }
            case syntax::ast_node_types_t::expression: {
                return evaluate(r, node->lhs);
            }
            case syntax::ast_node_types_t::unary_operator: {
                break;
            }
            case syntax::ast_node_types_t::binary_operator: {
                break;
            }
            case syntax::ast_node_types_t::symbol_reference: {
                break;
            }
            case syntax::ast_node_types_t::qualified_symbol_reference: {
                break;
            }
            default: {
                return node;
            }
        }

        return nullptr;
    }

};