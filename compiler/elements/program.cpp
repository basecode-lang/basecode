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

#include "type.h"
#include "program.h"
#include "directive.h"
#include "line_comment.h"

namespace basecode::compiler {

    program::program() : block(nullptr) {
    }

    program::~program() {
        for (auto element : _elements)
            delete element.second;
        _elements.clear();
    }

    bool program::initialize(
            common::result& r,
            const syntax::ast_node_shared_ptr& root) {
        initialize_core_types();

        if (root->type != syntax::ast_node_types_t::program) {
            r.add_message(
                "P001",
                "The root AST node must be of type 'program'.",
                true);
            return false;
        }

        evaluate(r, root);

        return true;
    }

    void program::initialize_core_types() {
        auto& type_map = types();
        type_map.add("u8",  &s_u8_type);
        type_map.add("u16", &s_u16_type);
        type_map.add("u32", &s_u32_type);
        type_map.add("u64", &s_u64_type);
        type_map.add("s8",  &s_s8_type);
        type_map.add("s16", &s_s16_type);
        type_map.add("s32", &s_s32_type);
        type_map.add("s64", &s_s64_type);
        type_map.add("f32", &s_f32_type);
        type_map.add("any", &s_any_type);
        type_map.add("bool", &s_bool_type);
        type_map.add("string", &s_string_type);
        type_map.add("address", &s_address_type);
    }

    element* program::find_element(id_t id) {
        auto it = _elements.find(id);
        if (it != _elements.end())
            return it->second;
        return nullptr;
    }

    void program::evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node) {
        if (node == nullptr)
            return;

        switch (node->type) {
            case syntax::ast_node_types_t::statement: {
                break;
            }
            case syntax::ast_node_types_t::line_comment: {
                auto comment_element = new compiler::line_comment(node->token.value);
                _elements.insert(std::make_pair(comment_element->id(), comment_element));
                children().push_back(comment_element);
                break;
            }
            case syntax::ast_node_types_t::directive: {
                auto directive_element = new compiler::directive(
                    node->token.value,
                    this);
                _elements.insert(std::make_pair(
                    directive_element->id(),
                    directive_element));
                children().push_back(directive_element);
                break;
            }
            case syntax::ast_node_types_t::expression: {
//                node->lhs = evaluate(r, node->lhs);
                break;
            }
            case syntax::ast_node_types_t::assignment: {
//                if (is_subtree_constant(node->rhs)) {
//                    auto folded_expression = evaluate(r, node->rhs);
//                } else {
//                    return node;
//                }
                break;
            }
            case syntax::ast_node_types_t::unary_operator: {
//                auto folded_rhs = evaluate(r, node->rhs);
//
//                uint64_t rhs_value;
//                if (folded_rhs->token.is_boolean()) {
//                    rhs_value = folded_rhs->token.as_bool() ? 1 : 0;
//                } else {
//                    folded_rhs->token.parse(rhs_value);
//                }
//
//                switch (node->token.type) {
//                    case syntax::token_types_t::bang: {
//                        auto flag = !rhs_value;
//                        node->token = flag ? syntax::s_true_literal : syntax::s_false_literal;
//                        break;
//                    }
//                    case syntax::token_types_t::tilde: {
//                        node->type = syntax::ast_node_types_t::number_literal;
//                        node->token.type = syntax::token_types_t::number_literal;
//                        node->token.value = std::to_string(~rhs_value);
//                        break;
//                    }
//                    case syntax::token_types_t::minus: {
//                        node->type = syntax::ast_node_types_t::number_literal;
//                        node->token.type = syntax::token_types_t::number_literal;
//                        node->token.value = std::to_string(-rhs_value);
//                        break;
//                    }
//                    default:
//                        break;
//                }
//
//                node->lhs = nullptr;
//                node->rhs = nullptr;

                break;
            }
            case syntax::ast_node_types_t::binary_operator: {
//                auto folded_lhs = evaluate(r, node->lhs);
//                auto folded_rhs = evaluate(r, node->rhs);
//
//                switch (node->token.type) {
//                    case syntax::token_types_t::pipe:
//                    case syntax::token_types_t::plus:
//                    case syntax::token_types_t::minus:
//                    case syntax::token_types_t::slash:
//                    case syntax::token_types_t::caret:
//                    case syntax::token_types_t::percent:
//                    case syntax::token_types_t::asterisk:
//                    case syntax::token_types_t::ampersand:
//                    {
//                        uint64_t lhs_value, rhs_value;
//                        folded_lhs->token.parse(lhs_value);
//                        folded_rhs->token.parse(rhs_value);
//
//                        switch (node->token.type) {
//                            case syntax::token_types_t::plus:
//                                node->token.value = std::to_string(lhs_value + rhs_value);
//                                break;
//                            case syntax::token_types_t::pipe:
//                                node->token.value = std::to_string(lhs_value | rhs_value);
//                                break;
//                            case syntax::token_types_t::minus:
//                                node->token.value = std::to_string(lhs_value - rhs_value);
//                                break;
//                            case syntax::token_types_t::slash:
//                                node->token.value = std::to_string(lhs_value / rhs_value);
//                                break;
//                            case syntax::token_types_t::caret:
//                                node->token.value = std::to_string(lhs_value ^ rhs_value);
//                                break;
//                            case syntax::token_types_t::percent:
//                                node->token.value = std::to_string(lhs_value % rhs_value);
//                                break;
//                            case syntax::token_types_t::asterisk:
//                                node->token.value = std::to_string(lhs_value * rhs_value);
//                                break;
//                            case syntax::token_types_t::ampersand:
//                                node->token.value = std::to_string(lhs_value & rhs_value);
//                                break;
//                            default:
//                                break;
//                        }
//
//                        node->lhs = nullptr;
//                        node->rhs = nullptr;
//                        node->type = syntax::ast_node_types_t::number_literal;
//                        node->token.type = syntax::token_types_t::number_literal;
//
//                        break;
//                    }
//                    case syntax::token_types_t::equals: {
//                        node->type = syntax::ast_node_types_t::boolean_literal;
//                        if (*folded_lhs == *folded_rhs) {
//                            node->token = syntax::s_true_literal;
//                        } else {
//                            node->token = syntax::s_false_literal;
//                        }
//                        node->lhs = nullptr;
//                        node->rhs = nullptr;
//                        break;
//                    }
//                    case syntax::token_types_t::less_than: {
//                        break;
//                    }
//                    case syntax::token_types_t::not_equals: {
//                        node->type = syntax::ast_node_types_t::boolean_literal;
//                        if (*folded_lhs != *folded_rhs) {
//                            node->token = syntax::s_true_literal;
//                        } else {
//                            node->token = syntax::s_false_literal;
//                        }
//                        node->lhs = nullptr;
//                        node->rhs = nullptr;
//                        break;
//                    }
//                    case syntax::token_types_t::logical_or: {
//                        break;
//                    }
//                    case syntax::token_types_t::logical_and: {
//                        break;
//                    }
//                    case syntax::token_types_t::greater_than: {
//                        break;
//                    }
//                    case syntax::token_types_t::less_than_equal: {
//                        break;
//                    }
//                    case syntax::token_types_t::greater_than_equal: {
//                        break;
//                    }
//                    default: {
//                        break;
//                    }
//                }
                break;
            }
            default: {
                break;
            }
        }

        switch (node->type) {
            case syntax::ast_node_types_t::program:
            case syntax::ast_node_types_t::directive:
            case syntax::ast_node_types_t::basic_block:
            case syntax::ast_node_types_t::namespace_statement: {
                for (auto it = node->children.begin(); it != node->children.end(); ++it) {
                    evaluate(r, *it);
                }
                break;
            }
            default:
                break;
        }
    }

    bool program::is_subtree_constant(
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
            case syntax::ast_node_types_t::null_literal:
            case syntax::ast_node_types_t::block_comment:
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