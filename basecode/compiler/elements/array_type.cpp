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

#include "block.h"
#include "program.h"
#include "array_type.h"
#include "identifier.h"
#include "declaration.h"
#include "pointer_type.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    std::string array_type::name_for_array(
            compiler::type* entry_type,
            const element_list_t& subscripts) {
        std::stringstream stream;
        stream << fmt::format("__array_{}", entry_type->symbol()->name());
        for (auto s : subscripts) {
            if (s->element_type() == element_type_t::spread_operator) {
                stream << "_SD";
            } else {
                uint64_t size = 0;
                if (s->as_integer(size)) {
                    stream << fmt::format("_S{}", size);
                }
            }
        }
        stream << "__";
        return stream.str();
    }

    ///////////////////////////////////////////////////////////////////////////

    array_type::array_type(
            compiler::module* module,
            block* parent_scope,
            compiler::block* scope,
            compiler::type_reference* entry_type,
            const element_list_t& subscripts) : compiler::composite_type(
                                                    module,
                                                    parent_scope,
                                                    composite_types_t::struct_type,
                                                    scope,
                                                    nullptr,
                                                    element_type_t::array_type),
                                                _subscripts(subscripts),
                                                _entry_type_ref(entry_type) {
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

    int32_t array_type::find_index(common::id_t id) {
        for (size_t i = 0; i < _subscripts.size(); i++) {
            if (_subscripts[i]->id() == id)
                return static_cast<int32_t>(i);
        }
        return -1;
    }

    const element_list_t& array_type::subscripts() const {
        return _subscripts;
    }

    bool array_type::on_type_check(compiler::type* other) {
        // XXX: temporary!
        return true;
    }

    compiler::type_reference* array_type::entry_type_ref() {
        return _entry_type_ref;
    }

    type_access_model_t array_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

    void array_type::on_owned_elements(element_list_t& list) {
        if (_entry_type_ref != nullptr)
            list.emplace_back(_entry_type_ref);

        for (auto e : _subscripts)
            list.emplace_back(e);
    }

    bool array_type::on_initialize(compiler::session& session) {
        auto& scope_manager = session.scope_manager();
        auto& builder = session.builder();

        auto type_symbol = builder.make_symbol(
            parent_scope(),
            name_for_array(_entry_type_ref->type(), _subscripts));
        symbol(type_symbol);
        type_symbol->parent_element(this);

        auto block_scope = scope();

        auto u8_type = scope_manager.find_type(qualified_symbol_t("u8"));
        auto u32_type = scope_manager.find_type(qualified_symbol_t("u32"));
        auto ptr_type = builder.make_pointer_type(
            block_scope,
            qualified_symbol_t("u8"),
            u8_type);
        auto type_info_type = scope_manager.find_type(qualified_symbol_t("type"));

        auto u8_type_ref = builder.make_type_reference(
            block_scope,
            u8_type->symbol()->qualified_symbol(),
            u8_type);
        auto u32_type_ref = builder.make_type_reference(
            block_scope,
            u32_type->symbol()->qualified_symbol(),
            u32_type);
        auto ptr_type_ref = builder.make_type_reference(
            block_scope,
            qualified_symbol_t("^u8"),
            ptr_type);
        auto type_info_ref = builder.make_type_reference(
            block_scope,
            type_info_type->symbol()->qualified_symbol(),
            type_info_type);

        auto flags_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "flags"),
            nullptr);
        flags_identifier->type_ref(u8_type_ref);
        block_scope->identifiers().add(flags_identifier);
        auto flags_field = builder.make_field(
            this,
            block_scope,
            builder.make_declaration(block_scope, flags_identifier, nullptr),
            0);

        auto length_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "length"),
            nullptr);
        length_identifier->type_ref(u32_type_ref);
        block_scope->identifiers().add(length_identifier);
        auto length_field = builder.make_field(
            this,
            block_scope,
            builder.make_declaration(block_scope, length_identifier, nullptr),
            flags_field->end_offset());

        auto capacity_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "capacity"),
            nullptr);
        capacity_identifier->type_ref(u32_type_ref);
        block_scope->identifiers().add(capacity_identifier);
        auto capacity_field = builder.make_field(
            this,
            block_scope,
            builder.make_declaration(block_scope, capacity_identifier, nullptr),
            length_field->end_offset());

        auto element_type_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "element_type"),
            nullptr);
        element_type_identifier->type_ref(type_info_ref);
        block_scope->identifiers().add(element_type_identifier);
        auto element_type_field = builder.make_field(
            this,
            block_scope,
            builder.make_declaration(block_scope, element_type_identifier, nullptr),
            capacity_field->end_offset());

        auto data_identifier = builder.make_identifier(
            block_scope,
            builder.make_symbol(block_scope, "data"),
            nullptr);
        data_identifier->type_ref(ptr_type_ref);
        block_scope->identifiers().add(data_identifier);
        auto data_field = builder.make_field(
            this,
            block_scope,
            builder.make_declaration(block_scope, data_identifier, nullptr),
            element_type_field->end_offset());

        auto& field_map = fields();
        field_map.add(flags_field);
        field_map.add(length_field);
        field_map.add(capacity_field);
        field_map.add(element_type_field);
        field_map.add(data_field);

        return composite_type::on_initialize(session);
    }

    std::string array_type::name(const std::string& alias) const {
        auto entry_type_name = !alias.empty() ? alias : _entry_type_ref->name();
        std::stringstream stream;

        for (auto s : _subscripts) {
            if (s->element_type() == element_type_t::spread_operator) {
                stream << "[...]";
                break;
            } else {
                uint64_t size = 0;
                if (s->as_integer(size)) {
                    stream << fmt::format("[{}]", size);
                }
            }
        }

        stream << entry_type_name;
        return stream.str();
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

};