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

#include <vm/terp.h>
#include <fmt/format.h>
#include <vm/assembler.h>
#include <common/defer.h>
#include <common/bytes.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
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
#include "boolean_literal.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "module_reference.h"
#include "namespace_element.h"
#include "procedure_instance.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    program::program() : element(nullptr, nullptr, element_type_t::program),
                         _builder(this),
                         _ast_evaluator(&_builder, this) {
    }

    program::~program() {
    }

    bool program::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();

        auto instruction_block = assembler.make_basic_block();
        instruction_block->jump_direct("_initializer");

        std::map<vm::section_t, element_list_t> vars_by_section {};
        /* auto bss  = */vars_by_section.insert(std::make_pair(vm::section_t::bss,     element_list_t()));
        auto ro   = vars_by_section.insert(std::make_pair(vm::section_t::ro_data, element_list_t()));
        auto data = vars_by_section.insert(std::make_pair(vm::section_t::data,    element_list_t()));
        /* auto text = */vars_by_section.insert(std::make_pair(vm::section_t::text,    element_list_t()));

        auto identifiers = elements().find_by_type(element_type_t::identifier);
        for (auto identifier : identifiers) {
            auto var = dynamic_cast<compiler::identifier*>(identifier);
            auto var_type = var->type();
            if (var_type == nullptr
            ||  var_type->element_type() == element_type_t::namespace_type)
                continue;

            if (within_procedure_scope(var->parent_scope())
            ||  var->is_parent_element(element_type_t::field))
                continue;

            switch (var->type()->element_type()) {
                case element_type_t::bool_type:
                case element_type_t::numeric_type: {
                    if (var->is_constant()) {
                        auto& list = ro.first->second;
                        list.emplace_back(var);
                    }
                    else {
                        auto& list = data.first->second;
                        list.emplace_back(var);
                    }
                    break;
                }
                case element_type_t::any_type:
                case element_type_t::array_type:
                case element_type_t::tuple_type:
                case element_type_t::string_type:
                case element_type_t::composite_type: {
                    if (var->initializer() != nullptr) {
                        if (var->is_constant()) {
                            auto& list = ro.first->second;
                            list.emplace_back(var);
                        } else {
                            auto& list = data.first->second;
                            list.emplace_back(var);
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        auto& ro_list = ro.first->second;
        for (const auto& it : _interned_string_literals) {
            compiler::string_literal* str = it.second.front();
            if (!str->is_parent_element(element_type_t::argument_list))
                continue;
            ro_list.emplace_back(str);
        }

        for (const auto& section : vars_by_section) {
            instruction_block->section(section.first);
            instruction_block->current_entry()->blank_lines(1);

            for (auto e : section.second) {
                switch (e->element_type()) {
                    case element_type_t::string_literal: {
                        auto string_literal = dynamic_cast<compiler::string_literal*>(e);
                        instruction_block->memo();
                        instruction_block->align(4);
                        auto it = _interned_string_literals.find(string_literal->value());
                        if (it != _interned_string_literals.end()) {
                            auto current_entry = instruction_block->current_entry();
                            string_literal_list_t& str_list = it->second;
                            for (auto str : str_list) {
                                auto var_label = instruction_block->make_label(str->label_name());
                                current_entry->label(var_label);

                                auto var = session.emit_context().allocate_variable(
                                    var_label->name(),
                                    session.program().find_type({.name = "string"}),
                                    identifier_usage_t::heap,
                                    nullptr);
                                if (var != nullptr)
                                    var->address_offset = 4;
                            }
                            current_entry->blank_lines(1);
                        }
                        instruction_block->current_entry()->comment(
                            fmt::format("\"{}\"", string_literal->value()),
                            session.emit_context().indent);
                        instruction_block->string(string_literal->escaped_value());

                        break;
                    }
                    case element_type_t::identifier: {
                        auto var = dynamic_cast<compiler::identifier*>(e);
                        auto init = var->initializer();

                        instruction_block->memo();
                        instruction_block->current_entry()->blank_lines(1);

                        auto type_alignment = static_cast<uint8_t>(var->type()->alignment());
                        if (type_alignment > 1)
                            instruction_block->align(type_alignment);
                        auto var_label = instruction_block->make_label(var->symbol()->name());
                        instruction_block->current_entry()->label(var_label);
                        session.emit_context().allocate_variable(
                            var_label->name(),
                            var->type(),
                            identifier_usage_t::heap);

                        switch (var->type()->element_type()) {
                            case element_type_t::bool_type: {
                                bool value = false;
                                var->as_bool(value);

                                if (init == nullptr)
                                    instruction_block->reserve_byte(1);
                                else
                                    instruction_block->bytes({static_cast<uint8_t>(value ? 1 : 0)});
                                break;
                            }
                            case element_type_t::numeric_type: {
                                uint64_t value = 0;
                                if (var->type()->number_class() == type_number_class_t::integer) {
                                    var->as_integer(value);
                                } else {
                                    double temp = 0;
                                    if (var->as_float(temp)) {
                                        vm::register_value_alias_t alias;
                                        alias.qwf = temp;
                                        value = alias.qw;
                                    }
                                }

                                auto symbol_type = vm::integer_symbol_type_for_size(
                                    var->type()->size_in_bytes());
                                switch (symbol_type) {
                                    case vm::symbol_type_t::u8:
                                        if (init == nullptr)
                                            instruction_block->reserve_byte(1);
                                        else
                                            instruction_block->bytes({static_cast<uint8_t>(value)});
                                        break;
                                    case vm::symbol_type_t::u16:
                                        if (init == nullptr)
                                            instruction_block->reserve_word(1);
                                        else
                                            instruction_block->words({static_cast<uint16_t>(value)});
                                        break;
                                    case vm::symbol_type_t::f32:
                                    case vm::symbol_type_t::u32:
                                        if (init == nullptr)
                                            instruction_block->reserve_dword(1);
                                        else
                                            instruction_block->dwords({static_cast<uint32_t>(value)});
                                        break;
                                    case vm::symbol_type_t::f64:
                                    case vm::symbol_type_t::u64:
                                        if (init == nullptr)
                                            instruction_block->reserve_qword(1);
                                        else
                                            instruction_block->qwords({value});
                                        break;
                                    case vm::symbol_type_t::bytes:
                                        break;
                                    default:
                                        break;
                                }
                                break;
                            }
                            case element_type_t::string_type: {
                                if (init != nullptr) {
                                    auto string_literal = dynamic_cast<compiler::string_literal*>(
                                        init->expression());
                                    if (string_literal != nullptr) {
                                        instruction_block->current_entry()->comment(
                                            fmt::format("\"{}\"", string_literal->value()),
                                            session.emit_context().indent);
                                        instruction_block->string(string_literal->value());
                                    }
                                }
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        assembler.push_block(instruction_block);

        auto procedure_types = elements().find_by_type(element_type_t::proc_type);
        procedure_type_list_t proc_list {};
        for (auto p : procedure_types) {
            auto procedure_type = dynamic_cast<compiler::procedure_type*>(p);
            if (procedure_type->parent_scope()->element_type() == element_type_t::proc_instance_block) {
                proc_list.emplace_back(procedure_type);
            }
        }

        for (auto p : procedure_types) {
            auto procedure_type = dynamic_cast<compiler::procedure_type*>(p);
            if (procedure_type->parent_scope()->element_type() != element_type_t::proc_instance_block) {
                proc_list.emplace_back(procedure_type);
            }
        }

        for (auto procedure_type : proc_list) {
            procedure_type->emit(session);
        }

        auto top_level_block = assembler.make_basic_block();
        top_level_block->align(vm::instruction_t::alignment);
        top_level_block->current_entry()->blank_lines(1);
        top_level_block->memo();
        top_level_block->current_entry()->label(top_level_block->make_label("_initializer"));

        block_list_t implicit_blocks {};
        auto module_blocks = elements().find_by_type(element_type_t::module_block);
        for (auto block : module_blocks) {
            implicit_blocks.emplace_back(dynamic_cast<compiler::block*>(block));
        }

        assembler.push_block(top_level_block);
        for (auto block : implicit_blocks)
            block->emit(session);

        auto finalizer_block = assembler.make_basic_block();
        finalizer_block->align(vm::instruction_t::alignment);
        finalizer_block->current_entry()->blank_lines(1);
        finalizer_block->exit();
        finalizer_block->current_entry()->label(finalizer_block->make_label("_finalizer"));

        assembler.pop_block();
        assembler.pop_block();

        return true;
    }

    bool program::compile(compiler::session& session) {
        auto& assembler = session.assembler();
        // XXX: temporary!
        auto& r = session.result();

        _block = push_new_block();
        _block->parent_element(this);

        _top_level_stack.push(_block);

        initialize_core_types(session);

        for (auto source_file : session.source_files()) {
            auto module = compile_module(session, source_file);
            if (module == nullptr)
                return false;
        }

        auto directives = elements().find_by_type(element_type_t::directive);
        for (auto directive : directives) {
            auto directive_element = dynamic_cast<compiler::directive*>(directive);
            if (!directive_element->execute(session, this))
                return false;
        }

        if (!resolve_unknown_identifiers(session))
            return false;

        if (!resolve_unknown_types(session))
            return false;

        if (!type_check(session))
            return false;

        if (!r.is_failed()) {
            auto& listing = assembler.listing();
            listing.add_source_file("top_level.basm");
            listing.select_source_file("top_level.basm");

            emit(session);

            assembler.apply_addresses(r);
            assembler.resolve_labels(r);
            if (assembler.assemble(r)) {
                session.run();
            }
        }

        _top_level_stack.pop();

        return !r.is_failed();
    }

    compiler::block* program::block() {
        return _block;
    }

    compiler::module* program::compile_module(
            compiler::session& session,
            common::source_file* source_file) {
        auto is_root = session.current_source_file() == nullptr;

        session.push_source_file(source_file);
        defer({
            session.pop_source_file();
        });

        compiler::module* module = (compiler::module*)nullptr;
        auto module_node = session.parse(source_file);
        if (module_node != nullptr) {
            module = dynamic_cast<compiler::module*>(_ast_evaluator.evaluate(
                session,
                module_node.get()));
            if (module != nullptr) {
                module->parent_element(this);
                module->is_root(is_root);
            }
        }

        return module;
    }

    element_map& program::elements() {
        return _elements;
    }

    element_builder& program::builder() {
        return _builder;
    }

    compiler::block* program::pop_scope() {
        if (_scope_stack.empty())
            return nullptr;
        auto top = _scope_stack.top();
        _scope_stack.pop();
        return top;
    }

    void program::disassemble(
            compiler::session& session,
            FILE* file) {
        auto& assembler = session.assembler();

        auto root_block = assembler.root_block();
        if (root_block == nullptr)
            return;
        root_block->disassemble();
        if (file != nullptr) {
            fmt::print(file, "\n");
            assembler.listing().write(file);
        }
    }

    compiler::module* program::current_module() {
        if (_module_stack.empty())
            return nullptr;
        return _module_stack.top();
    }

    compiler::block* program::current_top_level() {
        if (_top_level_stack.empty())
            return nullptr;
        return _top_level_stack.top();
    }

    compiler::block* program::current_scope() const {
        if (_scope_stack.empty())
            return nullptr;
        return _scope_stack.top();
    }

    void program::initialize_core_types(compiler::session& session) {
        auto parent_scope = current_scope();

        compiler::numeric_type::make_types(session, parent_scope);
        add_type_to_scope(_builder.make_module_type(
            session,
            parent_scope,
            _builder.make_block(parent_scope, element_type_t::block)));
        add_type_to_scope(_builder.make_namespace_type(session, parent_scope));
        add_type_to_scope(_builder.make_bool_type(session, parent_scope));
        add_type_to_scope(_builder.make_string_type(
            session,
            parent_scope,
            _builder.make_block(parent_scope, element_type_t::block)));

        add_type_to_scope(_builder.make_type_info_type(
            session,
            parent_scope,
            _builder.make_block(parent_scope, element_type_t::block)));
        add_type_to_scope(_builder.make_tuple_type(
            session,
            parent_scope,
            _builder.make_block(parent_scope, element_type_t::block)));
        add_type_to_scope(_builder.make_any_type(
            session,
            parent_scope,
            _builder.make_block(parent_scope, element_type_t::block)));
    }

    compiler::block* program::push_new_block(element_type_t type) {
        auto parent_scope = current_scope();
        auto scope_block = _builder.make_block(parent_scope, type);

        if (parent_scope != nullptr) {
            scope_block->parent_element(parent_scope);
            parent_scope->blocks().push_back(scope_block);
        }

        push_scope(scope_block);
        return scope_block;
    }

    void program::push_scope(compiler::block* block) {
        _scope_stack.push(block);
    }

    bool program::visit_blocks(
            common::result& r,
            const block_visitor_callable& callable,
            compiler::block* root_block) {
        std::function<bool (compiler::block*)> recursive_execute =
            [&](compiler::block* scope) -> bool {
                if (!callable(scope))
                    return false;
                for (auto block : scope->blocks()) {
                    if (!recursive_execute(block))
                        return false;
                }
                return true;
            };
        return recursive_execute(root_block != nullptr ? root_block : _top_level_stack.top());
    }

    bool program::type_check(compiler::session& session) {
        auto identifiers = elements().find_by_type(element_type_t::identifier);
        for (auto identifier : identifiers) {
            auto var = dynamic_cast<compiler::identifier*>(identifier);
            auto init = var->initializer();
            if (init == nullptr)
                continue;
            auto rhs_type = init->infer_type(this);
            if (!var->type()->type_check(rhs_type)) {
                session.error(
                    init,
                    "C051",
                    fmt::format(
                        "type mismatch: cannot assign {} to {}.",
                        rhs_type->symbol()->name(),
                        var->type()->symbol()->name()),
                    var->location());
            }
        }

        auto binary_ops = elements().find_by_type(element_type_t::binary_operator);
        for (auto op : binary_ops) {
            auto binary_op = dynamic_cast<compiler::binary_operator*>(op);
            if (binary_op->operator_type() != operator_type_t::assignment)
                continue;

            // XXX: revisit this for destructuring/multiple assignment
            auto var = dynamic_cast<compiler::identifier*>(binary_op->lhs());
            auto rhs_type = binary_op->rhs()->infer_type(this);
            if (!var->type()->type_check(rhs_type)) {
                session.error(
                    binary_op,
                    "C051",
                    fmt::format(
                        "type mismatch: cannot assign {} to {}.",
                        rhs_type->symbol()->name(),
                        var->type()->symbol()->name()),
                    binary_op->rhs()->location());
            }
        }

        return !session.result().is_failed();
    }

    void program::add_type_to_scope(compiler::type* type) {
        current_scope()->types().add(type);
    }

    bool program::resolve_unknown_types(compiler::session& session) {
        auto it = _identifiers_with_unknown_types.begin();
        while (it != _identifiers_with_unknown_types.end()) {
            auto var = *it;

            if (var->type() != nullptr
            &&  var->type()->element_type() != element_type_t::unknown_type) {
                it = _identifiers_with_unknown_types.erase(it);
                continue;
            }

            compiler::type* identifier_type = nullptr;
            if (var->is_parent_element(element_type_t::binary_operator)) {
                auto binary_operator = dynamic_cast<compiler::binary_operator*>(var->parent_element());
                if (binary_operator->operator_type() == operator_type_t::assignment) {
                    identifier_type = binary_operator->rhs()->infer_type(this);
                    var->type(identifier_type);
                }
            } else {
                if (var->initializer() == nullptr) {
                    auto unknown_type = dynamic_cast<compiler::unknown_type*>(var->type());

                    type_find_result_t find_result {};
                    find_result.type_name = unknown_type->symbol()->qualified_symbol();
                    find_result.is_array = unknown_type->is_array();
                    find_result.is_pointer = unknown_type->is_pointer();
                    find_result.array_size = unknown_type->array_size();

                    identifier_type = _builder.make_complete_type(
                        session,
                        find_result,
                        var->parent_scope());
                    if (identifier_type != nullptr) {
                        var->type(identifier_type);
                        _elements.remove(unknown_type->id());
                    }
                } else {
                    identifier_type = var
                        ->initializer()
                        ->expression()
                        ->infer_type(this);
                    var->type(identifier_type);
                }
            }

            if (identifier_type != nullptr) {
                var->inferred_type(true);
                it = _identifiers_with_unknown_types.erase(it);
            } else {
                ++it;
                session.error(
                    var,
                    "P004",
                    fmt::format(
                        "unable to resolve type for identifier: {}",
                        var->symbol()->name()),
                    var->symbol()->location());
            }
        }

        return _identifiers_with_unknown_types.empty();
    }

    bool program::resolve_unknown_identifiers(compiler::session& session) {
        auto it = _unresolved_identifier_references.begin();
        while (it != _unresolved_identifier_references.end()) {
            auto unresolved_reference = *it;
            if (unresolved_reference->resolved()) {
                it = _unresolved_identifier_references.erase(it);
                continue;
            }

            auto identifier = find_identifier(
                unresolved_reference->symbol(),
                unresolved_reference->parent_scope());
            if (identifier == nullptr) {
                ++it;
                session.error(
                    unresolved_reference,
                    "P004",
                    fmt::format(
                        "unable to resolve identifier: {}",
                        unresolved_reference->symbol().name),
                    unresolved_reference->symbol().location);
                continue;
            }

            unresolved_reference->identifier(identifier);

            it = _unresolved_identifier_references.erase(it);
        }

        return _unresolved_identifier_references.empty();
    }

    bool program::find_identifier_type(
            compiler::session& session,
            type_find_result_t& result,
            const syntax::ast_node_shared_ptr& type_node,
            compiler::block* parent_scope) {
        if (type_node == nullptr)
            return false;

        _builder.make_qualified_symbol(result.type_name, type_node->lhs.get());
        result.array_size = 0;
        result.is_array = type_node->is_array();
        result.is_spread = type_node->is_spread();
        result.is_pointer = type_node->is_pointer();
        _builder.make_complete_type(session, result, parent_scope);
        return result.type != nullptr;
    }

    element* program::walk_parent_scopes(
            compiler::block* scope,
            const scope_visitor_callable& callable) const {
        while (scope != nullptr) {
            auto* result = callable(scope);
            if (result != nullptr)
                return result;
            scope = scope->parent_scope();
        }
        return nullptr;
    }

    element* program::walk_parent_elements(
            compiler::element* element,
            const element_visitor_callable& callable) const {
        auto current = element;
        while (current != nullptr) {
            auto* result = callable(current);
            if (result != nullptr)
                return result;
            current = current->parent_element();
        }
        return nullptr;
    }

    compiler::module* program::find_module(compiler::element* element) const {
        return dynamic_cast<compiler::module*>(walk_parent_elements(
            element,
            [](compiler::element* each) -> compiler::element* {
                if (each->element_type() == element_type_t::module)
                    return each;
                return nullptr;
            }));
    }

    compiler::type* program::find_type(
            const qualified_symbol_t& symbol,
            compiler::block* scope) const {
        if (symbol.is_qualified()) {
            return dynamic_cast<compiler::type*>(walk_qualified_symbol(
                symbol,
                const_cast<compiler::program*>(this)->current_top_level(),
                [&](compiler::block* scope) -> compiler::element* {
                    auto matching_type = scope->types().find(symbol.name);
                    if (matching_type != nullptr)
                        return matching_type;

                    auto type_identifier = find_identifier(symbol, scope);
                    if (type_identifier != nullptr)
                        return type_identifier->type();

                    return nullptr;
                }));
        } else {
            return dynamic_cast<compiler::type*>(walk_parent_scopes(
                scope != nullptr ? scope : current_scope(),
                [&](compiler::block* scope) -> compiler::element* {
                    auto type = scope->types().find(symbol.name);
                    if (type != nullptr)
                        return type;
                    auto type_identifier = find_identifier(symbol, scope);
                    if (type_identifier != nullptr)
                        return type_identifier->type();
                    return nullptr;
                }));
        }
    }

    bool program::within_procedure_scope(compiler::block* parent_scope) const {
        auto block_scope = parent_scope == nullptr ? current_scope() : parent_scope;
        while (block_scope != nullptr) {
            if (block_scope->element_type() == element_type_t::proc_type_block
            ||  block_scope->element_type() == element_type_t::proc_instance_block)
                return true;
            block_scope = block_scope->parent_scope();
        }
        return false;
    }

    compiler::identifier* program::find_identifier(
            const qualified_symbol_t& symbol,
            compiler::block* scope) const {
        if (symbol.is_qualified()) {
            return dynamic_cast<compiler::identifier*>(walk_qualified_symbol(
                symbol,
                scope,
                [&symbol](compiler::block* scope) {
                    return scope->identifiers().find(symbol.name);
                }));
        } else {
            return dynamic_cast<compiler::identifier*>(walk_parent_scopes(
                scope != nullptr ? scope : current_scope(),
                [&](compiler::block* scope) -> compiler::element* {
                    auto var = scope->identifiers().find(symbol.name);
                    if (var != nullptr)
                        return var;
                    for (auto import : scope->imports()) {
                        auto identifier_reference = dynamic_cast<compiler::identifier_reference*>(import->expression());
                        auto qualified_symbol = identifier_reference->symbol();
                        qualified_symbol.namespaces.push_back(qualified_symbol.name);
                        qualified_symbol.name = symbol.name;
                        qualified_symbol.fully_qualified_name = make_fully_qualified_name(qualified_symbol);
                        var = find_identifier(qualified_symbol, import->module()->scope());
                        if (var != nullptr)
                            return var;
                    }
                    return nullptr;
                }));
        }
    }

    element* program::walk_qualified_symbol(
            const qualified_symbol_t& symbol,
            compiler::block* scope,
            const namespace_visitor_callable& callable) const {
        auto non_const_this = const_cast<compiler::program*>(this);
        auto block_scope = scope != nullptr ? scope : non_const_this->current_top_level();
        for (const auto& namespace_name : symbol.namespaces) {
            auto var = block_scope->identifiers().find(namespace_name);
            if (var == nullptr || var->initializer() == nullptr)
                return nullptr;
            auto expr = var->initializer()->expression();
            if (expr->element_type() == element_type_t::namespace_e) {
                auto ns = dynamic_cast<namespace_element*>(expr);
                block_scope = dynamic_cast<compiler::block*>(ns->expression());
            } else if (expr->element_type() == element_type_t::module_reference) {
                auto module_reference = dynamic_cast<compiler::module_reference*>(expr);
                block_scope = module_reference->module()->scope();
            } else {
                return nullptr;
            }
        }
        return callable(block_scope);
    }

    compiler::type* program::find_pointer_type(
            compiler::type* base_type,
            compiler::block* scope) {
        return find_type(
            qualified_symbol_t {
                .name = compiler::pointer_type::name_for_pointer(base_type)
            },
            scope);
    }

    compiler::type* program::find_array_type(
            compiler::type* entry_type,
            size_t size,
            compiler::block* scope) {
        return find_type(
            qualified_symbol_t {
                .name = compiler::array_type::name_for_array(entry_type, size)
            },
            scope);
    }

};