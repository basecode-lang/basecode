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
#include "type.h"
#include "program.h"
#include "any_type.h"
#include "attribute.h"
#include "directive.h"
#include "identifier.h"
#include "initializer.h"
#include "string_type.h"
#include "numeric_type.h"
#include "line_comment.h"
#include "block_comment.h"
#include "unary_operator.h"
#include "composite_type.h"
#include "procedure_type.h"
#include "binary_operator.h"

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
                auto attr = make_attribute(
                    node->token.value,
                    evaluate(r, node->rhs));
                scope->attributes().add(attr);
                return attr;
            }
            case syntax::ast_node_types_t::directive: {
                auto scope = current_scope();
                auto directive_element = make_directive(node->token.value);
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
            case syntax::ast_node_types_t::statement:
            case syntax::ast_node_types_t::namespace_expression: {
                return evaluate(r, node->rhs);
            }
            case syntax::ast_node_types_t::expression: {
                break;
            }
            case syntax::ast_node_types_t::assignment: {
                // binary_operator returned from here
//                auto list_of_identifiers = evaluate(r, node->lhs);
//                auto list_of_expressions = evaluate(r, node->rhs);
                break;
            }
            case syntax::ast_node_types_t::line_comment: {
                auto scope = current_scope();
                auto comment = make_line_comment(node->token.value);
                scope->children().push_back(comment);
                return comment;
            }
            case syntax::ast_node_types_t::block_comment: {
                auto scope = current_scope();
                auto comment = make_block_comment(node->token.value);
                scope->children().push_back(comment);
                return comment;
            }
            case syntax::ast_node_types_t::unary_operator: {
                break;
            }
            case syntax::ast_node_types_t::binary_operator: {
                break;
            }
            case syntax::ast_node_types_t::proc_expression: {
                break;
            }
            case syntax::ast_node_types_t::enum_expression: {
                auto scope = current_scope();
                auto& scope_types = scope->types();

                auto type = make_enum();
                scope_types.add(type);

                for (const auto& child : node->rhs->children) {
                    auto field_element = evaluate(r, child);
                    auto field = make_field("", nullptr, nullptr);
                    type->fields().add(field);
                }

                return type;
            }
            case syntax::ast_node_types_t::union_expression: {
                auto scope = current_scope();
                auto& scope_types = scope->types();

                auto type = make_union();
                scope_types.add(type);

                for (const auto& child : node->rhs->children) {
                }

                return type;
            }
            case syntax::ast_node_types_t::struct_expression: {
                auto scope = current_scope();
                auto& scope_types = scope->types();

                auto type = make_struct();
                scope_types.add(type);

                for (const auto& child : node->rhs->children) {
                }

                return type;
            }
            case syntax::ast_node_types_t::constant_expression: {
                // XXX: highly doubt this correct
                auto assignment = dynamic_cast<binary_operator*>(evaluate(r, node->rhs));
                auto variable = dynamic_cast<identifier*>(assignment->lhs());
                variable->constant(true);
                break;
            }
            case syntax::ast_node_types_t::qualified_symbol_reference: {
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

    field* program::make_field(
            const std::string& name,
            compiler::type* type,
            compiler::initializer* initializer) {
        auto field = new compiler::field(current_scope(), name, type, initializer);
        _elements.insert(std::make_pair(field->id(), field));
        return field;
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
        auto& scope_types = current_scope()->types();
        scope_types.add(make_any_type());
        scope_types.add(make_string_type());
        scope_types.add(make_numeric_type("bool",    0,         1));
        scope_types.add(make_numeric_type("u8",      0,         UINT8_MAX));
        scope_types.add(make_numeric_type("u16",     0,         UINT16_MAX));
        scope_types.add(make_numeric_type("u32",     0,         UINT32_MAX));
        scope_types.add(make_numeric_type("u64",     0,         UINT64_MAX));
        scope_types.add(make_numeric_type("s8",      INT8_MIN,  INT8_MAX));
        scope_types.add(make_numeric_type("s16",     INT16_MIN, INT16_MAX));
        scope_types.add(make_numeric_type("s32",     INT32_MIN, INT32_MAX));
        scope_types.add(make_numeric_type("s64",     INT64_MIN, INT64_MAX));
        scope_types.add(make_numeric_type("f32",     0,         UINT32_MAX));
        scope_types.add(make_numeric_type("f64",     0,         UINT64_MAX));
        scope_types.add(make_numeric_type("address", 0,         UINTPTR_MAX));
    }

    attribute* program::make_attribute(
            const std::string& name,
            element* expr) {
        auto attr = new attribute(current_scope(), name, expr);
        _elements.insert(std::make_pair(attr->id(), attr));
        return attr;
    }

    composite_type* program::make_enum() {
        auto type = new composite_type(
            current_scope(),
            composite_types_t::enum_type,
            fmt::format("__enum_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    composite_type* program::make_union() {
        auto type = new composite_type(
            current_scope(),
            composite_types_t::union_type,
            fmt::format("__union_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
    }

    composite_type* program::make_struct() {
        auto type = new composite_type(
            current_scope(),
            composite_types_t::struct_type,
            fmt::format("__struct_{}__", common::id_pool::instance()->allocate()));
        _elements.insert(std::make_pair(type->id(), type));
        return type;
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

    identifier* program::make_identifier(element* expr) {
        return nullptr;
    }

    directive* program::make_directive(const std::string& name) {
        auto directive1 = new compiler::directive(current_scope(), name);
        _elements.insert(std::make_pair(directive1->id(), directive1));
        return directive1;
    }

    line_comment* program::make_line_comment(const std::string& value) {
        auto comment = new compiler::line_comment(current_scope(), value);
        _elements.insert(std::make_pair(comment->id(), comment));
        return comment;
    }

    block_comment* program::make_block_comment(const std::string& value) {
        auto comment = new compiler::block_comment(current_scope(), value);
        _elements.insert(std::make_pair(comment->id(), comment));
        return comment;
    }

};