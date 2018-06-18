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
#include "attribute.h"
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

    element* program::evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node) {
        if (node == nullptr)
            return nullptr;

        switch (node->type) {
            case syntax::ast_node_types_t::attribute: {
                auto scope = current_scope();
                return scope->attributes().add(
                    node->token.value,
                    evaluate(r, node->rhs));
            }
            case syntax::ast_node_types_t::directive: {
                auto scope = current_scope();
                auto directive_element = new compiler::directive(
                    scope,
                    node->token.value);
                _elements.insert(std::make_pair(
                    directive_element->id(),
                    directive_element));
                scope->children().push_back(directive_element);
                evaluate(r, node->lhs);
                return directive_element;
            }
            case syntax::ast_node_types_t::program:
            case syntax::ast_node_types_t::basic_block: {
                make_new_block();

                if (node->type == syntax::ast_node_types_t::program) {
                    initialize_core_types();
                }

                for (auto it = node->children.begin();
                     it != node->children.end();
                     ++it) {
                    evaluate(r, *it);
                }

                pop_scope();

                break;
            }
            case syntax::ast_node_types_t::expression: {
                break;
            }
            case syntax::ast_node_types_t::assignment: {
                break;
            }
            case syntax::ast_node_types_t::line_comment: {
                auto scope = current_scope();
                auto comment_element = new compiler::line_comment(
                    this,
                    node->token.value);
                _elements.insert(std::make_pair(
                    comment_element->id(),
                    comment_element));
                scope->children().push_back(comment_element);
                return comment_element;
            }
            case syntax::ast_node_types_t::block_comment: {
                break;
            }
            case syntax::ast_node_types_t::unary_operator: {
                break;
            }
            case syntax::ast_node_types_t::binary_operator: {
                break;
            }
            default: {
                break;
            }
        }

        return nullptr;
    }

    bool program::initialize(
            common::result& r,
            const syntax::ast_node_shared_ptr& root) {
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

    block* program::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    block* program::make_new_block() {
        auto type = new block(current_scope());
        _elements.insert(std::make_pair(type->id(), type));
        push_scope(type);
        return type;
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

    any_type* program::make_any_type() {
        auto type = new any_type(current_scope());
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    block* program::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top();
    }

    void program::initialize_core_types() {
        auto& type_map = types();
        type_map.add("any",     make_any_type());
        type_map.add("string",  make_string_type());
        type_map.add("bool",    make_numeric_type("bool",    0,         1));
        type_map.add("u8",      make_numeric_type("u8",      0,         UINT8_MAX));
        type_map.add("u16",     make_numeric_type("u16",     0,         UINT16_MAX));
        type_map.add("u32",     make_numeric_type("u32",     0,         UINT32_MAX));
        type_map.add("u64",     make_numeric_type("u64",     0,         UINT64_MAX));
        type_map.add("s8",      make_numeric_type("s8",      INT8_MIN,  INT8_MAX));
        type_map.add("s16",     make_numeric_type("s16",     INT16_MIN, INT16_MAX));
        type_map.add("s32",     make_numeric_type("s32",     INT32_MIN, INT32_MAX));
        type_map.add("s64",     make_numeric_type("s64",     INT64_MIN, INT64_MAX));
        type_map.add("f32",     make_numeric_type("f32",     0,         UINT32_MAX));
        type_map.add("f64",     make_numeric_type("f64",     0,         UINT64_MAX));
        type_map.add("address", make_numeric_type("address", 0,         UINTPTR_MAX));
    }

    void program::push_scope(block* block) {
        _scope_stack.push(block);
    }

    element* program::find_element(id_t id) {
        auto it = _elements.find(id);
        if (it != _elements.end())
            return it->second;
        return nullptr;
    }

    numeric_type* program::make_numeric_type(
            const std::string& name,
            int64_t min,
            uint64_t max) {
        auto type = new numeric_type(current_scope(), name, min, max);
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    string_type* program::make_string_type() {
        auto type = new string_type(current_scope());
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

};