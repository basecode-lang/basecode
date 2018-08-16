// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <compiler/session.h>
#include "type.h"
#include "cast.h"
#include "label.h"
#include "alias.h"
#include "import.h"
#include "module.h"
#include "comment.h"
#include "program.h"
#include "any_type.h"
#include "bool_type.h"
#include "attribute.h"
#include "directive.h"
#include "statement.h"
#include "type_info.h"
#include "transmute.h"
#include "expression.h"
#include "identifier.h"
#include "if_element.h"
#include "array_type.h"
#include "tuple_type.h"
#include "initializer.h"
#include "module_type.h"
#include "string_type.h"
#include "numeric_type.h"
#include "unknown_type.h"
#include "pointer_type.h"
#include "argument_list.h"
#include "float_literal.h"
#include "string_literal.h"
#include "unary_operator.h"
#include "composite_type.h"
#include "procedure_type.h"
#include "return_element.h"
#include "procedure_call.h"
#include "namespace_type.h"
#include "symbol_element.h"
#include "element_builder.h"
#include "boolean_literal.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "module_reference.h"
#include "namespace_element.h"
#include "procedure_instance.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    element_builder::element_builder(compiler::session* session): _session(session) {
    }

    import* element_builder::make_import(
            compiler::block* parent_scope,
            compiler::element* expr,
            compiler::element* from_expr,
            compiler::module* imported_module) {
        auto import_element = new compiler::import(
            _session->program().current_module(),
            parent_scope,
            expr,
            from_expr,
            imported_module);
        _session->elements().add(import_element);

        if (expr != nullptr)
            expr->parent_element(import_element);

        if (from_expr != nullptr)
            from_expr->parent_element(import_element);

        return import_element;
    }

    compiler::type* element_builder::make_complete_type(
            compiler::session& session,
            type_find_result_t& result,
            compiler::block* parent_scope) {
        result.type = _session->program().find_type(result.type_name, parent_scope);
        if (result.type != nullptr) {
            if (result.is_array) {
                auto array_type = _session->program().find_array_type(
                    result.type,
                    result.array_size,
                    parent_scope);
                if (array_type == nullptr) {
                    array_type = make_array_type(
                        session,
                        parent_scope,
                        make_block(parent_scope, element_type_t::block),
                        result.type,
                        result.array_size);
                }
                result.type = array_type;
            }

            if (result.is_pointer) {
                auto pointer_type = _session->program().find_pointer_type(
                    result.type,
                    parent_scope);
                if (pointer_type == nullptr) {
                    pointer_type = make_pointer_type(
                        session,
                        parent_scope,
                        result.type);
                }
                result.type = pointer_type;
            }
        }
        return result.type;
    }

    void element_builder::make_qualified_symbol(
            qualified_symbol_t& symbol,
            const syntax::ast_node_t* node) {
        if (!node->children.empty()) {
            for (size_t i = 0; i < node->children.size() - 1; i++)
                symbol.namespaces.push_back(node->children[i]->token.value);
        }
        symbol.name = node->children.back()->token.value;
        symbol.location = node->location;
        symbol.fully_qualified_name = make_fully_qualified_name(symbol);
    }

    compiler::symbol_element* element_builder::make_symbol_from_node(
            compiler::session& session,
            const syntax::ast_node_t* node) {
        qualified_symbol_t qualified_symbol {};
        make_qualified_symbol(qualified_symbol, node);
        auto symbol = make_symbol(
            _session->program().current_scope(),
            qualified_symbol.name,
            qualified_symbol.namespaces);
        symbol->location(node->location);
        symbol->constant(node->is_constant_expression());
        return symbol;
    }

    namespace_type* element_builder::make_namespace_type(
            compiler::session& session,
            compiler::block* parent_scope) {
        auto type = new compiler::namespace_type(
            _session->program().current_module(),
            parent_scope);
        if (!type->initialize(session))
            return nullptr;

        _session->elements().add(type);
        return type;
    }

    unknown_type* element_builder::make_unknown_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            bool is_pointer,
            bool is_array,
            size_t array_size) {
        auto type = new compiler::unknown_type(
            _session->program().current_module(),
            parent_scope,
            symbol);
        if (!type->initialize(session))
            return nullptr;
        type->is_array(is_array);
        type->is_pointer(is_pointer);
        type->array_size(array_size);
        symbol->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    procedure_type* element_builder::make_procedure_type(
            compiler::block* parent_scope,
            compiler::block* block_scope) {
        auto type_name = fmt::format("__proc_{}__", common::id_pool::instance()->allocate());
        auto type = new compiler::procedure_type(
            _session->program().current_module(),
            parent_scope,
            block_scope,
            make_symbol(parent_scope, type_name));
        if (block_scope != nullptr)
            block_scope->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    statement* element_builder::make_statement(
            compiler::block* parent_scope,
            label_list_t labels,
            element* expr) {
        auto statement = new compiler::statement(
            _session->program().current_module(),
            parent_scope,
            expr);
        // XXX: double check this
        if (expr != nullptr && expr->parent_element() == nullptr)
            expr->parent_element(statement);
        for (auto label : labels) {
            statement->labels().push_back(label);
            label->parent_element(statement);
        }
        _session->elements().add(statement);
        return statement;
    }

    string_literal* element_builder::make_string(
        compiler::block* parent_scope,
        const std::string& value) {
        auto literal = new compiler::string_literal(
            _session->program().current_module(),
            parent_scope,
            value);
        _session->elements().add(literal);

        auto it = _session->program()._interned_string_literals.find(value);
        if (it != _session->program()._interned_string_literals.end()) {
            auto& list = it->second;
            list.emplace_back(literal);
        } else {
            string_literal_list_t list {};
            list.emplace_back(literal);
            _session->program()._interned_string_literals.insert(std::make_pair(value, list));
        }

        return literal;
    }

    pointer_type* element_builder::make_pointer_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::type* base_type) {
        auto type = new compiler::pointer_type(
            _session->program().current_module(),
            parent_scope,
            base_type);
        if (!type->initialize(session))
            return nullptr;
        _session->elements().add(type);
        return type;
    }

    array_type* element_builder::make_array_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::type* entry_type,
            size_t size) {
        auto type = new compiler::array_type(
            _session->program().current_module(),
            parent_scope,
            scope,
            entry_type,
            size);
        if (!type->initialize(session))
            return nullptr;
        scope->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    module* element_builder::make_module(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto module_element = new compiler::module(
            _session->program().current_module(),
            parent_scope,
            scope);
        _session->elements().add(module_element);
        scope->parent_element(module_element);
        return module_element;
    }

    module_reference* element_builder::make_module_reference(
            compiler::block* parent_scope,
            compiler::element* expr) {
        auto module_reference = new compiler::module_reference(
            _session->program().current_module(),
            parent_scope,
            expr);
        _session->elements().add(module_reference);
        if (expr != nullptr)
            expr->parent_element(module_reference);
        return module_reference;
    }

    compiler::block* element_builder::make_block(
            compiler::block* parent_scope,
            element_type_t type) {
        auto block_element = new compiler::block(
            _session->program().current_module(),
            parent_scope,
            type);
        _session->elements().add(block_element);
        return block_element;
    }

    cast* element_builder::make_cast(
            compiler::block* parent_scope,
            compiler::type* type,
            element* expr) {
        auto cast = new compiler::cast(
            _session->program().current_module(),
            parent_scope,
            type, expr);
        _session->elements().add(cast);
        if (expr != nullptr)
            expr->parent_element(cast);
        return cast;
    }

    transmute* element_builder::make_transmute(
            compiler::block* parent_scope,
            compiler::type* type,
            element* expr) {
        auto transmute = new compiler::transmute(
            _session->program().current_module(),
            parent_scope,
            type,
            expr);
        _session->elements().add(transmute);
        if (expr != nullptr)
            expr->parent_element(transmute);
        return transmute;
    }

    procedure_instance* element_builder::make_procedure_instance(
            compiler::block* parent_scope,
            compiler::type* procedure_type,
            compiler::block* scope) {
        auto instance = new compiler::procedure_instance(
            _session->program().current_module(),
            parent_scope,
            procedure_type,
            scope);
        scope->parent_element(instance);
        _session->elements().add(instance);
        return instance;
    }

    tuple_type* element_builder::make_tuple_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::tuple_type(
            _session->program().current_module(),
            parent_scope,
            scope);
        if (!type->initialize(session))
            return nullptr;
        scope->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    module_type* element_builder::make_module_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::module_type(
            _session->program().current_module(),
            parent_scope,
            scope);
        if (!type->initialize(session))
            return nullptr;
        scope->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    string_type* element_builder::make_string_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::string_type(
            _session->program().current_module(),
            parent_scope,
            scope);
        if (!type->initialize(session))
            return nullptr;
        scope->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    namespace_element* element_builder::make_namespace(
            compiler::block* parent_scope,
            element* expr) {
        auto ns = new compiler::namespace_element(
            _session->program().current_module(),
            parent_scope,
            expr);
        if (expr != nullptr)
            expr->parent_element(ns);
        _session->elements().add(ns);
        return ns;
    }

    composite_type* element_builder::make_union_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type_name = fmt::format("__union_{}__", common::id_pool::instance()->allocate());
        auto symbol = make_symbol(parent_scope, type_name);
        auto type = new compiler::composite_type(
            _session->program().current_module(),
            parent_scope,
            composite_types_t::union_type,
            scope,
            symbol);
        symbol->parent_element(type);
        scope->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    composite_type* element_builder::make_struct_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type_name = fmt::format("__struct_{}__", common::id_pool::instance()->allocate());
        auto symbol = make_symbol(parent_scope, type_name);
        auto type = new compiler::composite_type(
            _session->program().current_module(),
            parent_scope,
            composite_types_t::struct_type,
            scope,
            symbol);
        symbol->parent_element(type);
        scope->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    procedure_call* element_builder::make_procedure_call(
            compiler::block* parent_scope,
            compiler::identifier_reference* reference,
            compiler::argument_list* args) {
        auto proc_call = new compiler::procedure_call(
            _session->program().current_module(),
            parent_scope,
            reference,
            args);
        _session->elements().add(proc_call);
        args->parent_element(proc_call);
        reference->parent_element(proc_call);
        return proc_call;
    }

    argument_list* element_builder::make_argument_list(compiler::block* parent_scope) {
        auto list = new compiler::argument_list(
            _session->program().current_module(),
            parent_scope);
        _session->elements().add(list);
        return list;
    }

    unary_operator* element_builder::make_unary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* rhs) {
        auto unary_operator = new compiler::unary_operator(
            _session->program().current_module(),
            parent_scope,
            type,
            rhs);
        rhs->parent_element(unary_operator);
        _session->elements().add(unary_operator);
        return unary_operator;
    }

    binary_operator* element_builder::make_binary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* lhs,
            element* rhs) {
        auto binary_operator = new compiler::binary_operator(
            _session->program().current_module(),
            parent_scope,
            type,
            lhs,
            rhs);
        lhs->parent_element(binary_operator);
        rhs->parent_element(binary_operator);
        _session->elements().add(binary_operator);
        return binary_operator;
    }

    label* element_builder::make_label(
            compiler::block* parent_scope,
            const std::string& name) {
        auto label = new compiler::label(
            _session->program().current_module(),
            parent_scope,
            name);
        _session->elements().add(label);
        return label;
    }

    field* element_builder::make_field(
            compiler::block* parent_scope,
            compiler::identifier* identifier) {
        auto field = new compiler::field(
            _session->program().current_module(),
            parent_scope,
            identifier);
        identifier->parent_element(field);
        _session->elements().add(field);
        return field;
    }

    float_literal* element_builder::make_float(
            compiler::block* parent_scope,
            double value) {
        auto literal = new compiler::float_literal(
            _session->program().current_module(),
            parent_scope,
            value);
        _session->elements().add(literal);
        return literal;
    }

    boolean_literal* element_builder::make_bool(
            compiler::block* parent_scope,
            bool value) {
        auto boolean_literal = new compiler::boolean_literal(
            _session->program().current_module(),
            parent_scope,
            value);
        _session->elements().add(boolean_literal);
        return boolean_literal;
    }

    expression* element_builder::make_expression(
            compiler::block* parent_scope,
            element* expr) {
        auto expression = new compiler::expression(
            _session->program().current_module(),
            parent_scope,
            expr);
        if (expr != nullptr)
            expr->parent_element(expression);
        _session->elements().add(expression);
        return expression;
    }

    integer_literal* element_builder::make_integer(
            compiler::block* parent_scope,
            uint64_t value) {
        auto literal = new compiler::integer_literal(
            _session->program().current_module(),
            parent_scope,
            value);
        _session->elements().add(literal);
        return literal;
    }

    composite_type* element_builder::make_enum_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type_name = fmt::format("__enum_{}__", common::id_pool::instance()->allocate());
        auto symbol = make_symbol(parent_scope, type_name);
        auto type = new compiler::composite_type(
            _session->program().current_module(),
            parent_scope,
            composite_types_t::enum_type,
            scope,
            symbol);
        symbol->parent_element(type);
        scope->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    initializer* element_builder::make_initializer(
            compiler::block* parent_scope,
            element* expr) {
        auto initializer = new compiler::initializer(
            _session->program().current_module(),
            parent_scope,
            expr);
        if (expr != nullptr)
            expr->parent_element(initializer);
        _session->elements().add(initializer);
        return initializer;
    }

    return_element* element_builder::make_return(compiler::block* parent_scope) {
        auto return_element = new compiler::return_element(
            _session->program().current_module(),
            parent_scope);
        _session->elements().add(return_element);
        return return_element;
    }

    numeric_type* element_builder::make_numeric_type(
            compiler::session& session,
            compiler::block* parent_scope,
            const std::string& name,
            int64_t min,
            uint64_t max,
            bool is_signed,
            type_number_class_t number_class) {
        auto type = new compiler::numeric_type(
            _session->program().current_module(),
            parent_scope,
            make_symbol(parent_scope, name),
            min,
            max,
            is_signed,
            number_class);
        if (!type->initialize(session))
            return nullptr;

        _session->elements().add(type);
        return type;
    }

    if_element* element_builder::make_if(
            compiler::block* parent_scope,
            element* predicate,
            element* true_branch,
            element* false_branch) {
        auto if_element = new compiler::if_element(
            _session->program().current_module(),
            parent_scope,
            predicate,
            true_branch,
            false_branch);
        if (predicate != nullptr)
            predicate->parent_element(if_element);
        if (true_branch != nullptr)
            true_branch->parent_element(if_element);
        if (false_branch != nullptr)
            false_branch->parent_element(if_element);
        _session->elements().add(if_element);
        return if_element;
    }

    comment* element_builder::make_comment(
            compiler::block* parent_scope,
            comment_type_t type,
            const std::string& value) {
        auto comment = new compiler::comment(
            _session->program().current_module(),
            parent_scope,
            type,
            value);
        _session->elements().add(comment);
        return comment;
    }

    compiler::directive* element_builder::make_directive(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr) {
        auto directive = new compiler::directive(
            _session->program().current_module(),
            parent_scope,
            name,
            expr);
        if (expr != nullptr)
            expr->parent_element(directive);
        _session->elements().add(directive);
        return directive;
    }

    attribute* element_builder::make_attribute(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr) {
        auto attr = new compiler::attribute(
            _session->program().current_module(),
            parent_scope,
            name,
            expr);
        if (expr != nullptr)
            expr->parent_element(attr);
        _session->elements().add(attr);
        return attr;
    }

    compiler::symbol_element* element_builder::make_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces) {
        auto symbol = new compiler::symbol_element(
            _session->program().current_module(),
            parent_scope,
            name,
            namespaces);
        _session->elements().add(symbol);
        symbol->cache_fully_qualified_name();
        return symbol;
    }

    compiler::symbol_element* element_builder::make_temp_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces) {
        return new compiler::symbol_element(
            _session->program().current_module(),
            parent_scope,
            name,
            namespaces);
    }

    identifier_reference* element_builder::make_identifier_reference(
            compiler::block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier) {
        auto reference = new compiler::identifier_reference(
            _session->program().current_module(),
            parent_scope,
            symbol,
            identifier);
        _session->elements().add(reference);
        if (!reference->resolved())
            _session->program()._unresolved_identifier_references.emplace_back(reference);
        reference->location(symbol.location);
        return reference;
    }

    identifier* element_builder::make_identifier(
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            initializer* expr) {
        auto identifier = new compiler::identifier(
            _session->program().current_module(),
            parent_scope,
            symbol,
            expr);

        if (expr != nullptr) {
            expr->parent_element(identifier);
        }

        symbol->parent_element(identifier);
        identifier->location(symbol->location());

        _session->elements().add(identifier);

        return identifier;
    }

    type_info* element_builder::make_type_info_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::type_info(
            _session->program().current_module(),
            parent_scope,
            scope);
        if (!type->initialize(session))
            return nullptr;
        scope->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    alias* element_builder::make_alias(
            compiler::block* parent_scope,
            element* expr) {
        auto alias_type = new compiler::alias(
            _session->program().current_module(),
            parent_scope,
            expr);
        if (expr != nullptr)
            expr->parent_element(alias_type);
        _session->elements().add(alias_type);
        return alias_type;
    }

    bool_type* element_builder::make_bool_type(
            compiler::session& session,
            compiler::block* parent_scope) {
        auto type = new compiler::bool_type(
            _session->program().current_module(),
            parent_scope);
        if (!type->initialize(session))
            return nullptr;
        _session->elements().add(type);
        return type;
    }

    any_type* element_builder::make_any_type(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::any_type(
            _session->program().current_module(),
            parent_scope,
            scope);
        if (!type->initialize(session))
            return nullptr;
        scope->parent_element(type);
        _session->elements().add(type);
        return type;
    }

    unknown_type* element_builder::make_unknown_type_from_find_result(
            compiler::session& session,
            compiler::block* scope,
            compiler::identifier* identifier,
            const type_find_result_t& result) {
        auto symbol = make_symbol(
            scope,
            result.type_name.name,
            result.type_name.namespaces);
        auto unknown_type = make_unknown_type(
            session,
            scope,
            symbol,
            result.is_pointer,
            result.is_array,
            result.array_size);
        _session->program()._identifiers_with_unknown_types.push_back(identifier);
        return unknown_type;
    }

};