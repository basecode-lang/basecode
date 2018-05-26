// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include <fmt/format.h>
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
            case syntax::ast_node_types_t::expression: {
                node->lhs = evaluate(r, node->lhs);
                break;
            }
            case syntax::ast_node_types_t::statement: {
                node->rhs = evaluate(r, node->rhs);
                break;
            }
            case syntax::ast_node_types_t::assignment: {
                if (is_subtree_constant(node->rhs)) {
                    auto folded_expression = evaluate(r, node->rhs);
                } else {
                    return node;
                }
                break;
            }
            case syntax::ast_node_types_t::unary_operator: {
                auto folded_rhs = evaluate(r, node->rhs);

                uint64_t rhs_value;
                if (folded_rhs->token.is_boolean()) {
                    rhs_value = folded_rhs->token.as_bool() ? 1 : 0;
                } else {
                    folded_rhs->token.parse(rhs_value);
                }

                switch (node->token.type) {
                    case syntax::token_types_t::bang: {
                        auto flag = !rhs_value;
                        node->token = flag ? syntax::s_true_literal : syntax::s_false_literal;
                        break;
                    }
                    case syntax::token_types_t::tilde: {
                        node->type = syntax::ast_node_types_t::number_literal;
                        node->token.type = syntax::token_types_t::number_literal;
                        node->token.value = std::to_string(~rhs_value);
                        break;
                    }
                    case syntax::token_types_t::minus: {
                        node->type = syntax::ast_node_types_t::number_literal;
                        node->token.type = syntax::token_types_t::number_literal;
                        node->token.value = std::to_string(-rhs_value);
                        break;
                    }
                    default:
                        break;
                }

                node->lhs = nullptr;
                node->rhs = nullptr;

                break;
            }
            case syntax::ast_node_types_t::binary_operator: {
                auto folded_lhs = evaluate(r, node->lhs);
                auto folded_rhs = evaluate(r, node->rhs);

                switch (node->token.type) {
                    case syntax::token_types_t::pipe:
                    case syntax::token_types_t::plus:
                    case syntax::token_types_t::minus:
                    case syntax::token_types_t::slash:
                    case syntax::token_types_t::caret:
                    case syntax::token_types_t::percent:
                    case syntax::token_types_t::asterisk:
                    case syntax::token_types_t::ampersand:
                    {
                        uint64_t lhs_value, rhs_value;
                        folded_lhs->token.parse(lhs_value);
                        folded_rhs->token.parse(rhs_value);

                        switch (node->token.type) {
                            case syntax::token_types_t::plus:
                                node->token.value = std::to_string(lhs_value + rhs_value);
                                break;
                            case syntax::token_types_t::pipe:
                                node->token.value = std::to_string(lhs_value | rhs_value);
                                break;
                            case syntax::token_types_t::minus:
                                node->token.value = std::to_string(lhs_value - rhs_value);
                                break;
                            case syntax::token_types_t::slash:
                                node->token.value = std::to_string(lhs_value / rhs_value);
                                break;
                            case syntax::token_types_t::caret:
                                node->token.value = std::to_string(lhs_value ^ rhs_value);
                                break;
                            case syntax::token_types_t::percent:
                                node->token.value = std::to_string(lhs_value % rhs_value);
                                break;
                            case syntax::token_types_t::asterisk:
                                node->token.value = std::to_string(lhs_value * rhs_value);
                                break;
                            case syntax::token_types_t::ampersand:
                                node->token.value = std::to_string(lhs_value & rhs_value);
                                break;
                            default:
                                break;
                        }

                        node->lhs = nullptr;
                        node->rhs = nullptr;
                        node->type = syntax::ast_node_types_t::number_literal;
                        node->token.type = syntax::token_types_t::number_literal;

                        break;
                    }
                    case syntax::token_types_t::equals: {
                        node->type = syntax::ast_node_types_t::boolean_literal;
                        if (*folded_lhs == *folded_rhs) {
                            node->token = syntax::s_true_literal;
                        } else {
                            node->token = syntax::s_false_literal;
                        }
                        node->lhs = nullptr;
                        node->rhs = nullptr;
                        break;
                    }
                    case syntax::token_types_t::less_than: {
                        break;
                    }
                    case syntax::token_types_t::not_equals: {
                        node->type = syntax::ast_node_types_t::boolean_literal;
                        if (*folded_lhs != *folded_rhs) {
                            node->token = syntax::s_true_literal;
                        } else {
                            node->token = syntax::s_false_literal;
                        }
                        node->lhs = nullptr;
                        node->rhs = nullptr;
                        break;
                    }
                    case syntax::token_types_t::logical_or: {
                        break;
                    }
                    case syntax::token_types_t::logical_and: {
                        break;
                    }
                    case syntax::token_types_t::greater_than: {
                        break;
                    }
                    case syntax::token_types_t::less_than_equal: {
                        break;
                    }
                    case syntax::token_types_t::greater_than_equal: {
                        break;
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case syntax::ast_node_types_t::program:
            case syntax::ast_node_types_t::basic_block:
            case syntax::ast_node_types_t::namespace_statement: {
                for (auto it = node->children.begin();
                        it != node->children.end();) {
                    auto block_child = *it;
                    if (block_child->type == syntax::ast_node_types_t::line_comment
                    ||  block_child->type == syntax::ast_node_types_t::block_comment) {
                        it = node->children.erase(it);
                    } else {
                        block_child = evaluate(r, block_child);
                        ++it;
                    }
                }

                break;
            }
            default: {
                break;
            }
        }

        return node;
    }

    bool constant_expression_evaluator::is_subtree_constant(
            const syntax::ast_node_shared_ptr& node) {
        if (node == nullptr)
            return false;

        switch (node->type) {
            case syntax::ast_node_types_t::expression: {
                return is_subtree_constant(node->lhs);
            }
            case syntax::ast_node_types_t::assignment: {
                return is_subtree_constant(node->rhs);
            }
            case syntax::ast_node_types_t::unary_operator: {
                return is_subtree_constant(node->rhs);
            }
            case syntax::ast_node_types_t::binary_operator: {
                return is_subtree_constant(node->lhs)
                       && is_subtree_constant(node->rhs);
            }
            case syntax::ast_node_types_t::basic_block:
            case syntax::ast_node_types_t::line_comment:
            case syntax::ast_node_types_t::none_literal:
            case syntax::ast_node_types_t::null_literal:
            case syntax::ast_node_types_t::block_comment:
            case syntax::ast_node_types_t::empty_literal:
            case syntax::ast_node_types_t::number_literal:
            case syntax::ast_node_types_t::string_literal:
            case syntax::ast_node_types_t::boolean_literal:
            case syntax::ast_node_types_t::character_literal:
                return true;
            default:
                return false;
        }
    }

};