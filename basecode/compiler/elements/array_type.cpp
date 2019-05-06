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

#include <compiler/scope_manager.h>
#include <compiler/element_builder.h>
#include <compiler/type_name_builder.h>
#include "block.h"
#include "array_type.h"
#include "identifier.h"
#include "declaration.h"
#include "nil_literal.h"
#include "pointer_type.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "integer_literal.h"

namespace basecode::compiler {

    std::string array_type::name_for_array(
            compiler::type* entry_type,
            const element_list_t& subscripts) {
        type_name_builder builder {};
        builder
            .add_part("array")
            .add_part(std::string(entry_type->symbol()->name()));

        for (auto s : subscripts) {
            integer_result_t size;
            if (s->as_integer(size))
                builder.add_part(fmt::format("S{}", size.value));
        }

        return builder.format();
    }

    ///////////////////////////////////////////////////////////////////////////

    array_type::array_type(
            compiler::module* module,
            block* parent_scope,
            compiler::block* scope,
            compiler::type_reference* base_type_ref,
            const element_list_t& subscripts) : compiler::composite_type(
                                                    module,
                                                    parent_scope,
                                                    composite_types_t::struct_type,
                                                    scope,
                                                    nullptr,
                                                    element_type_t::array_type),
                                                _subscripts(subscripts),
                                                _base_type_ref(base_type_ref) {
    }

    bool array_type::on_type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        if (other == nullptr || !other->is_array_type())
            return false;

        auto other_array = dynamic_cast<compiler::array_type*>(other);
        return _base_type_ref->type()->type_check(
            other_array->base_type_ref()->type(),
            options);
    }

    size_t array_type::data_size() const {
        return number_of_elements() * _base_type_ref->type()->size_in_bytes();
    }

    bool array_type::on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) {
        auto index = find_index(e->id());
        if (index == -1) {
            return false;
        }
        replace(static_cast<size_t>(index), fold_result.element);
        return true;
    }

    compiler::element* array_type::replace(
            size_t index,
            compiler::element* item) {
        auto old = _subscripts[index];
        _subscripts[index] = item;
        return old;
    }

    bool array_type::is_array_type() const {
        return true;
    }

    size_t array_type::number_of_elements() const {
        size_t count = 0;
        for (auto s : _subscripts) {
            integer_result_t temp;
            if (s->as_integer(temp)) {
                if (count == 0)
                    count = temp.value;
                else
                    count *= temp.value;
            }
        }
        return count;
    }

    int32_t array_type::find_index(common::id_t id) {
        for (size_t i = 0; i < _subscripts.size(); i++) {
            if (_subscripts[i]->id() == id)
                return static_cast<int32_t>(i);
        }
        return -1;
    }

    bool array_type::are_subscripts_constant() const {
        for (auto e : _subscripts) {
            if (!e->is_constant())
                return false;
        }
        return true;
    }

    const element_list_t& array_type::subscripts() const {
        return _subscripts;
    }

    compiler::type_reference* array_type::base_type_ref() {
        return _base_type_ref;
    }

    void array_type::on_owned_elements(element_list_t& list) {
        if (_base_type_ref != nullptr)
            list.emplace_back(_base_type_ref);

        for (auto e : _subscripts)
            list.emplace_back(e);
    }

    bool array_type::on_initialize(compiler::session& session) {
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        auto it = session.strings().insert(name_for_array(_base_type_ref->type(), _subscripts));
        auto type_symbol = builder.make_symbol(parent_scope(), *it.first);
        symbol(type_symbol);
        type_symbol->parent_element(this);

        auto block_scope = scope();
        auto& field_map = fields();

        auto u32_type = scope_manager.find_type(qualified_symbol_t("u32"sv));
        auto u32_type_ref = builder.make_type_reference(
            block_scope,
            u32_type->symbol()->qualified_symbol(),
            u32_type,
            true);
        u32_type_ref->parent_element(block_scope);

        auto length_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "length"sv),
            builder.make_initializer(block_scope, builder.make_integer(block_scope, 0)));
        length_identifier->type_ref(u32_type_ref);
        length_identifier->parent_element(block_scope);
        block_scope->identifiers().add(length_identifier);
        auto length_field = builder.make_field(
            block_scope,
            this,
            builder.make_declaration(block_scope, length_identifier),
            0);
        length_field->parent_element(this);
        length_identifier->field(length_field);
        field_map.add(length_field);

        auto entry_ptr_type = builder.make_pointer_type(
            block_scope,
            qualified_symbol_t(),
            _base_type_ref->type());
        auto entry_ptr_type_ref = builder.make_type_reference(
            block_scope,
            entry_ptr_type->symbol()->qualified_symbol(),
            entry_ptr_type,
            true);
        entry_ptr_type_ref->parent_element(block_scope);

        auto data_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "data"sv),
            builder.make_initializer(block_scope, builder.nil_literal()));
        data_identifier->type_ref(entry_ptr_type_ref);
        data_identifier->parent_element(block_scope);
        block_scope->identifiers().add(data_identifier);

        auto data_field = builder.make_field(
            block_scope,
            this,
            builder.make_declaration(block_scope, data_identifier),
            4);
        data_field->parent_element(this);
        data_identifier->field(data_field);
        field_map.add(data_field);

        alignment(8);
        calculate_size();

        return composite_type::on_initialize(session);
    }

    compiler::element* array_type::find_subscript(common::id_t id) {
        auto it = std::find_if(
            _subscripts.begin(),
            _subscripts.end(),
            [&id](auto item) { return item->id() == id; });
        if (it == _subscripts.end())
            return nullptr;
        return *it;
    }

}