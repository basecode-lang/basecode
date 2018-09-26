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
#include <compiler/elements/type.h>
#include <compiler/elements/cast.h>
#include <compiler/elements/with.h>
#include <compiler/elements/label.h>
#include <compiler/elements/import.h>
#include <compiler/elements/module.h>
#include <compiler/elements/spread.h>
#include <compiler/elements/comment.h>
#include <compiler/elements/program.h>
#include <compiler/elements/any_type.h>
#include <compiler/elements/raw_block.h>
#include <compiler/elements/bool_type.h>
#include <compiler/elements/attribute.h>
#include <compiler/elements/directive.h>
#include <compiler/elements/statement.h>
#include <compiler/elements/type_info.h>
#include <compiler/elements/transmute.h>
#include <compiler/elements/intrinsic.h>
#include <compiler/elements/rune_type.h>
#include <compiler/elements/expression.h>
#include <compiler/elements/identifier.h>
#include <compiler/elements/if_element.h>
#include <compiler/elements/array_type.h>
#include <compiler/elements/tuple_type.h>
#include <compiler/elements/assignment.h>
#include <compiler/elements/nil_literal.h>
#include <compiler/elements/declaration.h>
#include <compiler/elements/initializer.h>
#include <compiler/elements/module_type.h>
#include <compiler/elements/string_type.h>
#include <compiler/elements/for_element.h>
#include <compiler/elements/numeric_type.h>
#include <compiler/elements/unknown_type.h>
#include <compiler/elements/pointer_type.h>
#include <compiler/elements/defer_element.h>
#include <compiler/elements/argument_list.h>
#include <compiler/elements/float_literal.h>
#include <compiler/elements/while_element.h>
#include <compiler/elements/break_element.h>
#include <compiler/elements/copy_intrinsic.h>
#include <compiler/elements/fill_intrinsic.h>
#include <compiler/elements/type_reference.h>
#include <compiler/elements/free_intrinsic.h>
#include <compiler/elements/string_literal.h>
#include <compiler/elements/unary_operator.h>
#include <compiler/elements/composite_type.h>
#include <compiler/elements/procedure_type.h>
#include <compiler/elements/return_element.h>
#include <compiler/elements/procedure_call.h>
#include <compiler/elements/namespace_type.h>
#include <compiler/elements/symbol_element.h>
#include <compiler/elements/assembly_label.h>
#include <compiler/elements/alloc_intrinsic.h>
#include <compiler/elements/boolean_literal.h>
#include <compiler/elements/binary_operator.h>
#include <compiler/elements/integer_literal.h>
#include <compiler/elements/continue_element.h>
#include <compiler/elements/module_reference.h>
#include <compiler/elements/array_constructor.h>
#include <compiler/elements/size_of_intrinsic.h>
#include <compiler/elements/namespace_element.h>
#include <compiler/elements/type_of_intrinsic.h>
#include <compiler/elements/character_literal.h>
#include <compiler/elements/align_of_intrinsic.h>
#include <compiler/elements/procedure_instance.h>
#include <compiler/elements/address_of_intrinsic.h>
#include <compiler/elements/identifier_reference.h>
#include "element_builder.h"

namespace basecode::compiler {

    element_builder::element_builder(compiler::session& session): _session(session) {
    }

    import* element_builder::make_import(
            compiler::block* parent_scope,
            compiler::element* expr,
            compiler::element* from_expr,
            compiler::module* imported_module) {
        auto import_element = new compiler::import(
            _session.scope_manager().current_module(),
            parent_scope,
            expr,
            from_expr,
            imported_module);
        _session.elements().add(import_element);

        if (expr != nullptr)
            expr->parent_element(import_element);

        if (from_expr != nullptr)
            from_expr->parent_element(import_element);

        return import_element;
    }

    compiler::type* element_builder::make_complete_type(
            type_find_result_t& result,
            compiler::block* parent_scope) {
        auto& scope_manager = _session.scope_manager();

        result.type = scope_manager.find_type(
            result.type_name,
            parent_scope);
        if (result.type != nullptr) {
            if (result.is_array) {
                auto array_type = scope_manager.find_array_type(
                    result.type,
                    result.array_subscripts,
                    parent_scope);
                if (array_type == nullptr) {
                    array_type = make_array_type(
                        parent_scope,
                        make_block(parent_scope, element_type_t::block),
                        result.type,
                        result.type_name,
                        result.array_subscripts);
                }
                result.type = array_type;
            }

            if (result.is_pointer) {
                auto pointer_type = scope_manager.find_pointer_type(
                    result.type,
                    parent_scope);
                if (pointer_type == nullptr) {
                    pointer_type = make_pointer_type(
                        parent_scope,
                        result.type_name,
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

    compiler::symbol_element* element_builder::make_symbol_from_node(const syntax::ast_node_t* node) {
        qualified_symbol_t qualified_symbol {};
        make_qualified_symbol(qualified_symbol, node);
        auto symbol = make_symbol(
            _session.scope_manager().current_scope(),
            qualified_symbol.name,
            qualified_symbol.namespaces);
        symbol->location(node->location);
        return symbol;
    }

    namespace_type* element_builder::make_namespace_type(compiler::block* parent_scope) {
        auto type = new compiler::namespace_type(
            _session.scope_manager().current_module(),
            parent_scope);
        if (!type->initialize(_session))
            return nullptr;

        _session.elements().add(type);
        return type;
    }

    unknown_type* element_builder::make_unknown_type(
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            bool is_pointer,
            bool is_array,
            const element_list_t& subscripts) {
        auto type = new compiler::unknown_type(
            _session.scope_manager().current_module(),
            parent_scope,
            symbol);
        if (!type->initialize(_session))
            return nullptr;
        type->is_array(is_array);
        type->is_pointer(is_pointer);
        type->subscripts(subscripts);
        symbol->parent_element(type);
        _session.elements().add(type);
        return type;
    }

    procedure_type* element_builder::make_procedure_type(
            compiler::block* parent_scope,
            compiler::block* block_scope) {
        auto type_name = fmt::format("__proc_{}__", common::id_pool::instance()->allocate());
        auto type = new compiler::procedure_type(
            _session.scope_manager().current_module(),
            parent_scope,
            block_scope,
            make_symbol(parent_scope, type_name));
        if (block_scope != nullptr)
            block_scope->parent_element(type);
        _session.elements().add(type);
        return type;
    }

    statement* element_builder::make_statement(
            compiler::block* parent_scope,
            label_list_t labels,
            element* expr) {
        auto statement = new compiler::statement(
            _session.scope_manager().current_module(),
            parent_scope,
            expr);
        // XXX: double check this
        if (expr != nullptr && expr->parent_element() == nullptr)
            expr->parent_element(statement);
        for (auto label : labels) {
            statement->labels().push_back(label);
            label->parent_element(statement);
        }
        _session.elements().add(statement);
        return statement;
    }

    character_literal* element_builder::make_character(
            compiler::block* parent_scope,
            common::rune_t rune) {
        auto& scope_manager = _session.scope_manager();
        auto literal = new compiler::character_literal(
            scope_manager.current_module(),
            parent_scope,
            rune);
        _session.elements().add(literal);
        return literal;
    }

    string_literal* element_builder::make_string(
            compiler::block* parent_scope,
            const std::string& value) {
        auto& scope_manager = _session.scope_manager();
        auto literal = new compiler::string_literal(
            scope_manager.current_module(),
            parent_scope,
            value);
        _session.elements().add(literal);
        return literal;
    }

    pointer_type* element_builder::make_pointer_type(
            compiler::block* parent_scope,
            const qualified_symbol_t& type_name,
            compiler::type* base_type) {
        auto type = new compiler::pointer_type(
            _session.scope_manager().current_module(),
            parent_scope,
            make_type_reference(parent_scope, type_name, base_type));
        if (!type->initialize(_session))
            return nullptr;
        _session.elements().add(type);
        return type;
    }

    array_type* element_builder::make_array_type(
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::type* entry_type,
            const qualified_symbol_t& type_name,
            const element_list_t& subscripts) {
        auto& scope_manager = _session.scope_manager();

        auto type = new compiler::array_type(
            scope_manager.current_module(),
            parent_scope,
            scope,
            make_type_reference(parent_scope, type_name, entry_type),
            subscripts);
        if (!type->initialize(_session))
            return nullptr;

        scope->parent_element(type);
        for (auto s : subscripts)
            s->parent_element(type);

        _session.elements().add(type);

        return type;
    }

    module* element_builder::make_module(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto module_element = new compiler::module(
            _session.scope_manager().current_module(),
            parent_scope,
            scope);
        _session.elements().add(module_element);
        scope->parent_element(module_element);
        return module_element;
    }

    module_reference* element_builder::make_module_reference(
            compiler::block* parent_scope,
            compiler::element* expr) {
        auto module_reference = new compiler::module_reference(
            _session.scope_manager().current_module(),
            parent_scope,
            expr);
        _session.elements().add(module_reference);
        if (expr != nullptr)
            expr->parent_element(module_reference);
        return module_reference;
    }

    compiler::block* element_builder::make_block(
            compiler::block* parent_scope,
            element_type_t type) {
        auto block_element = new compiler::block(
            _session.scope_manager().current_module(),
            parent_scope,
            type);
        _session.elements().add(block_element);
        return block_element;
    }

    break_element* element_builder::make_break(
            compiler::block* parent_scope,
            compiler::label* label) {
        auto break_e = new compiler::break_element(
            _session.scope_manager().current_module(),
            parent_scope,
            label);
        _session.elements().add(break_e);
        if (label != nullptr)
            label->parent_element(break_e);
        return break_e;
    }

    continue_element* element_builder::make_continue(
            compiler::block* parent_scope,
            compiler::label* label) {
        auto continue_e = new compiler::continue_element(
            _session.scope_manager().current_module(),
            parent_scope,
            label);
        _session.elements().add(continue_e);
        if (label != nullptr)
            label->parent_element(continue_e);
        return continue_e;
    }

    spread* element_builder::make_spread_operator(
            compiler::block* parent_scope,
            compiler::element* expression) {
        auto spread_op = new compiler::spread(
            _session.scope_manager().current_module(),
            parent_scope,
            expression);
        _session.elements().add(spread_op);
        if (expression != nullptr)
            expression->parent_element(spread_op);
        return spread_op;
    }

    defer_element* element_builder::make_defer(
            compiler::block* parent_scope,
            compiler::element* expression) {
        auto defer_e = new compiler::defer_element(
            _session.scope_manager().current_module(),
            parent_scope,
            expression);
        _session.elements().add(defer_e);
        if (expression != nullptr)
            expression->parent_element(defer_e);
        return defer_e;
    }

    while_element* element_builder::make_while(
            compiler::block* parent_scope,
            compiler::binary_operator* predicate,
            compiler::block* body) {
        auto while_e = new compiler::while_element(
            _session.scope_manager().current_module(),
            parent_scope,
            predicate,
            body);
        _session.elements().add(while_e);
        if (predicate != nullptr)
            predicate->parent_element(while_e);
        if (body != nullptr)
            body->parent_element(while_e);
        return while_e;
    }

    with* element_builder::make_with(
            compiler::block* parent_scope,
            compiler::identifier_reference* ref,
            block* body) {
        auto with = new compiler::with(
            _session.scope_manager().current_module(),
            parent_scope,
            ref,
            body);
        _session.elements().add(with);
        if (ref != nullptr)
            ref->parent_element(with);
        if (body != nullptr)
            body->parent_element(with);
        return with;
    }

    cast* element_builder::make_cast(
            compiler::block* parent_scope,
            compiler::type_reference* type,
            element* expr) {
        auto cast = new compiler::cast(
            _session.scope_manager().current_module(),
            parent_scope,
            type,
            expr);
        _session.elements().add(cast);
        if (expr != nullptr)
            expr->parent_element(cast);
        return cast;
    }

    for_element* element_builder::make_for(
            compiler::block* parent_scope,
            compiler::declaration* induction_decl,
            compiler::element* expression,
            compiler::block* body) {
        auto for_element = new compiler::for_element(
            _session.scope_manager().current_module(),
            parent_scope,
            induction_decl,
            expression,
            body);
        _session.elements().add(for_element);
        if (induction_decl != nullptr)
            induction_decl->parent_element(for_element);
        if (expression != nullptr)
            expression->parent_element(for_element);
        if (body != nullptr)
            body->parent_element(for_element);
        return for_element;
    }

    transmute* element_builder::make_transmute(
            compiler::block* parent_scope,
            compiler::type_reference* type,
            element* expr) {
        auto transmute = new compiler::transmute(
            _session.scope_manager().current_module(),
            parent_scope,
            type,
            expr);
        _session.elements().add(transmute);
        if (expr != nullptr)
            expr->parent_element(transmute);
        return transmute;
    }

    procedure_instance* element_builder::make_procedure_instance(
            compiler::block* parent_scope,
            compiler::type* procedure_type,
            compiler::block* scope) {
        auto instance = new compiler::procedure_instance(
            _session.scope_manager().current_module(),
            parent_scope,
            procedure_type,
            scope);
        scope->parent_element(instance);
        _session.elements().add(instance);
        return instance;
    }

    tuple_type* element_builder::make_tuple_type(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::tuple_type(
            _session.scope_manager().current_module(),
            parent_scope,
            scope);
        if (!type->initialize(_session))
            return nullptr;
        scope->parent_element(type);
        _session.elements().add(type);
        return type;
    }

    module_type* element_builder::make_module_type(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::module_type(
            _session.scope_manager().current_module(),
            parent_scope,
            scope);
        if (!type->initialize(_session))
            return nullptr;
        scope->parent_element(type);
        _session.elements().add(type);
        return type;
    }

    string_type* element_builder::make_string_type(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::string_type(
            _session.scope_manager().current_module(),
            parent_scope,
            scope);
        if (!type->initialize(_session))
            return nullptr;
        scope->parent_element(type);
        _session.elements().add(type);
        return type;
    }

    namespace_element* element_builder::make_namespace(
            compiler::block* parent_scope,
            element* expr) {
        auto ns = new compiler::namespace_element(
            _session.scope_manager().current_module(),
            parent_scope,
            expr);
        if (expr != nullptr)
            expr->parent_element(ns);
        _session.elements().add(ns);
        return ns;
    }

    composite_type* element_builder::make_union_type(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type_name = fmt::format("__union_{}__", common::id_pool::instance()->allocate());
        auto symbol = make_symbol(parent_scope, type_name);
        auto type = new compiler::composite_type(
            _session.scope_manager().current_module(),
            parent_scope,
            composite_types_t::union_type,
            scope,
            symbol);
        symbol->parent_element(type);
        scope->parent_element(type);
        _session.elements().add(type);
        return type;
    }

    composite_type* element_builder::make_struct_type(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type_name = fmt::format("__struct_{}__", common::id_pool::instance()->allocate());
        auto symbol = make_symbol(parent_scope, type_name);
        auto type = new compiler::composite_type(
            _session.scope_manager().current_module(),
            parent_scope,
            composite_types_t::struct_type,
            scope,
            symbol);
        symbol->parent_element(type);
        scope->parent_element(type);
        _session.elements().add(type);
        return type;
    }

    procedure_call* element_builder::make_procedure_call(
            compiler::block* parent_scope,
            compiler::identifier_reference* reference,
            compiler::argument_list* args) {
        auto proc_call = new compiler::procedure_call(
            _session.scope_manager().current_module(),
            parent_scope,
            reference,
            args);
        _session.elements().add(proc_call);
        args->parent_element(proc_call);
        reference->parent_element(proc_call);
        return proc_call;
    }

    argument_list* element_builder::make_argument_list(compiler::block* parent_scope) {
        auto list = new compiler::argument_list(
            _session.scope_manager().current_module(),
            parent_scope);
        _session.elements().add(list);
        return list;
    }

    unary_operator* element_builder::make_unary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* rhs) {
        auto unary_operator = new compiler::unary_operator(
            _session.scope_manager().current_module(),
            parent_scope,
            type,
            rhs);
        rhs->parent_element(unary_operator);
        _session.elements().add(unary_operator);
        return unary_operator;
    }

    binary_operator* element_builder::make_binary_operator(
            compiler::block* parent_scope,
            operator_type_t type,
            element* lhs,
            element* rhs) {
        auto binary_operator = new compiler::binary_operator(
            _session.scope_manager().current_module(),
            parent_scope,
            type,
            lhs,
            rhs);
        lhs->parent_element(binary_operator);
        rhs->parent_element(binary_operator);
        _session.elements().add(binary_operator);
        return binary_operator;
    }

    label* element_builder::make_label(
            compiler::block* parent_scope,
            const std::string& name) {
        auto label = new compiler::label(
            _session.scope_manager().current_module(),
            parent_scope,
            name);
        _session.elements().add(label);
        return label;
    }

    assembly_label* element_builder::make_assembly_label(
            compiler::block* parent_scope,
            const std::string& name) {
        auto label = new compiler::assembly_label(
            _session.scope_manager().current_module(),
            parent_scope,
            name);
        _session.elements().add(label);
        return label;
    }

    field* element_builder::make_field(
            compiler::type* type,
            compiler::block* parent_scope,
            compiler::declaration* declaration,
            uint64_t offset,
            uint8_t padding) {
        auto field = new compiler::field(
            _session.scope_manager().current_module(),
            parent_scope,
            declaration,
            offset,
            padding);
        declaration->parent_element(field);
        field->parent_element(type);
        _session.elements().add(field);
        return field;
    }

    float_literal* element_builder::make_float(
            compiler::block* parent_scope,
            double value) {
        auto literal = new compiler::float_literal(
            _session.scope_manager().current_module(),
            parent_scope,
            value);
        _session.elements().add(literal);
        return literal;
    }

    boolean_literal* element_builder::make_bool(
            compiler::block* parent_scope,
            bool value) {
        auto boolean_literal = new compiler::boolean_literal(
            _session.scope_manager().current_module(),
            parent_scope,
            value);
        _session.elements().add(boolean_literal);
        return boolean_literal;
    }

    expression* element_builder::make_expression(
            compiler::block* parent_scope,
            element* expr) {
        auto expression = new compiler::expression(
            _session.scope_manager().current_module(),
            parent_scope,
            expr);
        if (expr != nullptr)
            expr->parent_element(expression);
        _session.elements().add(expression);
        return expression;
    }

    integer_literal* element_builder::make_integer(
            compiler::block* parent_scope,
            uint64_t value) {
        auto literal = new compiler::integer_literal(
            _session.scope_manager().current_module(),
            parent_scope,
            value);
        _session.elements().add(literal);
        return literal;
    }

    composite_type* element_builder::make_enum_type(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type_name = fmt::format("__enum_{}__", common::id_pool::instance()->allocate());
        auto symbol = make_symbol(parent_scope, type_name);
        auto type = new compiler::composite_type(
            _session.scope_manager().current_module(),
            parent_scope,
            composite_types_t::enum_type,
            scope,
            symbol);
        symbol->parent_element(type);
        scope->parent_element(type);
        _session.elements().add(type);
        return type;
    }

    initializer* element_builder::make_initializer(
            compiler::block* parent_scope,
            element* expr) {
        auto initializer = new compiler::initializer(
            _session.scope_manager().current_module(),
            parent_scope,
            expr);
        if (expr != nullptr)
            expr->parent_element(initializer);
        _session.elements().add(initializer);
        return initializer;
    }

    return_element* element_builder::make_return(compiler::block* parent_scope) {
        auto return_element = new compiler::return_element(
            _session.scope_manager().current_module(),
            parent_scope);
        _session.elements().add(return_element);
        return return_element;
    }

    numeric_type* element_builder::make_numeric_type(
            compiler::block* parent_scope,
            const std::string& name,
            int64_t min,
            uint64_t max,
            bool is_signed,
            type_number_class_t number_class) {
        auto type = new compiler::numeric_type(
            _session.scope_manager().current_module(),
            parent_scope,
            make_symbol(parent_scope, name),
            min,
            max,
            is_signed,
            number_class);
        if (!type->initialize(_session))
            return nullptr;

        _session.elements().add(type);
        return type;
    }

    if_element* element_builder::make_if(
            compiler::block* parent_scope,
            element* predicate,
            element* true_branch,
            element* false_branch,
            bool is_else_if) {
        auto if_element = new compiler::if_element(
            _session.scope_manager().current_module(),
            parent_scope,
            predicate,
            true_branch,
            false_branch,
            is_else_if);
        if (predicate != nullptr)
            predicate->parent_element(if_element);
        if (true_branch != nullptr)
            true_branch->parent_element(if_element);
        if (false_branch != nullptr)
            false_branch->parent_element(if_element);
        _session.elements().add(if_element);
        return if_element;
    }

    comment* element_builder::make_comment(
            compiler::block* parent_scope,
            comment_type_t type,
            const std::string& value) {
        auto comment = new compiler::comment(
            _session.scope_manager().current_module(),
            parent_scope,
            type,
            value);
        _session.elements().add(comment);
        return comment;
    }

    raw_block* element_builder::make_raw_block(
            compiler::block* parent_scope,
            const std::string& value) {
        auto raw_block = new compiler::raw_block(
            _session.scope_manager().current_module(),
            parent_scope,
            value);
        _session.elements().add(raw_block);
        return raw_block;
    }

    compiler::directive* element_builder::make_directive(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr) {
        auto directive = new compiler::directive(
            _session.scope_manager().current_module(),
            parent_scope,
            name,
            expr);
        if (expr != nullptr)
            expr->parent_element(directive);
        _session.elements().add(directive);
        return directive;
    }

    attribute* element_builder::make_attribute(
            compiler::block* parent_scope,
            const std::string& name,
            element* expr) {
        auto attr = new compiler::attribute(
            _session.scope_manager().current_module(),
            parent_scope,
            name,
            expr);
        if (expr != nullptr)
            expr->parent_element(attr);
        _session.elements().add(attr);
        return attr;
    }

    compiler::symbol_element* element_builder::make_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces) {
        auto symbol = new compiler::symbol_element(
            _session.scope_manager().current_module(),
            parent_scope,
            name,
            namespaces);
        _session.elements().add(symbol);
        symbol->cache_fully_qualified_name();
        return symbol;
    }

    compiler::symbol_element* element_builder::make_temp_symbol(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces) {
        return new compiler::symbol_element(
            _session.scope_manager().current_module(),
            parent_scope,
            name,
            namespaces);
    }

    type_reference* element_builder::make_type_reference(
            compiler::block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::type* type) {
        auto& scope_manager = _session.scope_manager();
        auto reference = new compiler::type_reference(
            scope_manager.current_module(),
            parent_scope,
            symbol,
            type);
        _session.elements().add(reference);
        reference->location(symbol.location);
        return reference;
    }

    identifier_reference* element_builder::make_identifier_reference(
            compiler::block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::identifier* identifier) {
        auto& scope_manager = _session.scope_manager();

        auto& unresolveds = scope_manager.unresolved_identifier_references();
        auto reference = new compiler::identifier_reference(
            scope_manager.current_module(),
            parent_scope,
            symbol,
            identifier);
        _session.elements().add(reference);
        if (!reference->resolved())
            unresolveds.emplace_back(reference);
        reference->location(symbol.location);
        return reference;
    }

    identifier* element_builder::make_identifier(
            compiler::block* parent_scope,
            compiler::symbol_element* symbol,
            initializer* expr) {
        auto identifier = new compiler::identifier(
            _session.scope_manager().current_module(),
            parent_scope,
            symbol,
            expr);

        if (expr != nullptr) {
            expr->parent_element(identifier);
        }

        symbol->parent_element(identifier);
        identifier->location(symbol->location());

        _session.elements().add(identifier);

        return identifier;
    }

    type_info* element_builder::make_type_info_type(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::type_info(
            _session.scope_manager().current_module(),
            parent_scope,
            scope);
        if (!type->initialize(_session))
            return nullptr;
        scope->parent_element(type);
        _session.elements().add(type);
        return type;
    }

    any_type* element_builder::make_any_type(
            compiler::block* parent_scope,
            compiler::block* scope) {
        auto type = new compiler::any_type(
            _session.scope_manager().current_module(),
            parent_scope,
            scope);
        if (!type->initialize(_session))
            return nullptr;
        scope->parent_element(type);
        _session.elements().add(type);
        return type;
    }

    unknown_type* element_builder::make_unknown_type_from_find_result(
            compiler::block* scope,
            const type_find_result_t& result) {
        auto symbol = make_symbol(
            scope,
            result.type_name.name,
            result.type_name.namespaces);
        auto unknown_type = make_unknown_type(
            scope,
            symbol,
            result.is_pointer,
            result.is_array,
            result.array_subscripts);
        return unknown_type;
    }

    intrinsic* element_builder::make_copy_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args) {
        auto intrinsic = new compiler::copy_intrinsic(
            _session.scope_manager().current_module(),
            parent_scope,
            args);
        _session.elements().add(intrinsic);
        args->parent_element(intrinsic);
        return intrinsic;
    }

    intrinsic* element_builder::make_fill_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args) {
        auto intrinsic = new compiler::fill_intrinsic(
            _session.scope_manager().current_module(),
            parent_scope,
            args);
        _session.elements().add(intrinsic);
        args->parent_element(intrinsic);
        return intrinsic;
    }

    intrinsic* element_builder::make_free_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args) {
        auto intrinsic = new compiler::free_intrinsic(
            _session.scope_manager().current_module(),
            parent_scope,
            args);
        _session.elements().add(intrinsic);
        args->parent_element(intrinsic);
        return intrinsic;
    }

    intrinsic* element_builder::make_alloc_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args) {
        auto intrinsic = new compiler::alloc_intrinsic(
            _session.scope_manager().current_module(),
            parent_scope,
            args);
        _session.elements().add(intrinsic);
        args->parent_element(intrinsic);
        return intrinsic;
    }

    intrinsic* element_builder::make_address_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args) {
        auto intrinsic = new compiler::address_of_intrinsic(
            _session.scope_manager().current_module(),
            parent_scope,
            args);
        _session.elements().add(intrinsic);
        args->parent_element(intrinsic);
        return intrinsic;
    }

    intrinsic* element_builder::make_align_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args) {
        auto intrinsic = new compiler::align_of_intrinsic(
            _session.scope_manager().current_module(),
            parent_scope,
            args);
        _session.elements().add(intrinsic);
        args->parent_element(intrinsic);
        return intrinsic;
    }

    intrinsic* element_builder::make_size_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args) {
        auto intrinsic = new compiler::size_of_intrinsic(
            _session.scope_manager().current_module(),
            parent_scope,
            args);
        _session.elements().add(intrinsic);
        args->parent_element(intrinsic);
        return intrinsic;
    }

    intrinsic* element_builder::make_type_of_intrinsic(
            compiler::block* parent_scope,
            compiler::argument_list* args) {
        auto intrinsic = new compiler::type_of_intrinsic(
            _session.scope_manager().current_module(),
            parent_scope,
            args);
        _session.elements().add(intrinsic);
        args->parent_element(intrinsic);
        return intrinsic;
    }

    declaration* element_builder::make_declaration(
            compiler::block* parent_scope,
            compiler::identifier* identifier,
            compiler::binary_operator* assignment) {
        auto decl_element = new compiler::declaration(
            _session.scope_manager().current_module(),
            parent_scope,
            identifier,
            assignment);
        _session.elements().add(decl_element);

        if (identifier != nullptr)
            identifier->parent_element(decl_element);

        if (assignment != nullptr)
            assignment->parent_element(decl_element);

        return decl_element;
    }

    nil_literal* element_builder::nil_literal() {
        if (_nil_literal == nullptr)
            _nil_literal = make_nil(_session.program().block());
        return _nil_literal;
    }

    boolean_literal* element_builder::true_literal() {
        if (_true_literal == nullptr)
            _true_literal = make_bool(_session.program().block(), true);
        return _true_literal;
    }

    boolean_literal* element_builder::false_literal() {
        if (_false_literal == nullptr)
            _false_literal = make_bool(_session.program().block(), false);
        return _false_literal;
    }

    array_constructor* element_builder::make_array_constructor(
            compiler::block* parent_scope,
            compiler::argument_list* args) {
        element_list_t subscripts {};
        subscripts.push_back(make_integer(
            parent_scope,
            args->size()));

        auto array_ctor = new compiler::array_constructor(
            _session.scope_manager().current_module(),
            parent_scope,
            args,
            subscripts);
        _session.elements().add(array_ctor);

        if (args != nullptr)
            args->parent_element(array_ctor);

        return array_ctor;
    }

    rune_type* element_builder::make_rune_type(compiler::block* parent_scope) {
        auto type = new compiler::rune_type(
            _session.scope_manager().current_module(),
            parent_scope);
        if (!type->initialize(_session))
            return nullptr;
        _session.elements().add(type);
        return type;
    }

    bool_type* element_builder::make_bool_type(compiler::block* parent_scope) {
        auto type = new compiler::bool_type(
            _session.scope_manager().current_module(),
            parent_scope);
        if (!type->initialize(_session))
            return nullptr;
        _session.elements().add(type);
        return type;
    }

    assignment* element_builder::make_assignment(compiler::block* parent_scope) {
        auto assignment_element = new compiler::assignment(
            _session.scope_manager().current_module(),
            parent_scope);
        _session.elements().add(assignment_element);
        return assignment_element;
    }

    compiler::nil_literal* element_builder::make_nil(compiler::block* parent_scope) {
        auto nil_literal = new compiler::nil_literal(
            _session.scope_manager().current_module(),
            parent_scope);
        _session.elements().add(nil_literal);
        return nil_literal;
    }

};