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
#include <compiler/scope_manager.h>
#include <compiler/element_builder.h>
#include "identifier.h"
#include "initializer.h"
#include "numeric_type.h"
#include "float_literal.h"
#include "symbol_element.h"
#include "integer_literal.h"
#include "binary_operator.h"

namespace basecode::compiler {

    void numeric_type::make_types(
            compiler::session& session,
            compiler::block* parent_scope) {
        auto& builder = session.builder();
        auto& scope_manager = session.scope_manager();

        for (const auto& props : s_type_properties) {
            auto type = builder.make_numeric_type(
                parent_scope,
                props.name,
                props.min,
                props.max,
                props.is_signed,
                props.number_class);
            type->initialize(session);
            scope_manager.add_type_to_scope(type);
        }
    }

    string_view_list_t numeric_type::type_names() {
        string_view_list_t list {};
        for (const auto& props : s_type_properties)
            list.emplace_back(props.name);
        return list;
    }

    // float:  -3.4e+38  to 3.4e+38
    // double: -1.7e+308 to 1.7e+308
    std::string_view numeric_type::narrow_to_value(double value) {
        if (value < -3.4e+38 || value > 3.4e+38)
            return "f64"sv;
        else if (value >= -3.4e+38 && value <= 3.4e+38)
            return "f32"sv;
        else
            return "unknown"sv;
    }

    std::string_view numeric_type::narrow_to_value(uint64_t value, bool is_signed) {
        if (is_signed) {
            native_integer_t native_value {};
            native_value.qw = value;
            for (size_t i = 5; i < 9; i++) {
                const auto& props = s_type_properties[i];
                const auto signed_max = static_cast<int64_t>(props.max);
                if (native_value.sqw >= props.min && native_value.sqw <= signed_max)
                    return props.name;
            }
        } else {
            for (size_t i = 1; i < 5; i++) {
                const auto& props = s_type_properties[i];
                if (value <= props.max)
                    return props.name;
            }
        }
        return "unknown"sv;
    }

    numeric_type_properties_t* numeric_type::type_properties_for_value(uint64_t value) {
        auto type_name = numeric_type::narrow_to_value(value);
        if (type_name == "unknown"sv)
            return nullptr;
        auto it = s_types_map.find(type_name);
        if (it == s_types_map.end())
            return nullptr;
        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////

    numeric_type::numeric_type(
            compiler::module* module,
            block* parent_scope,
            compiler::symbol_element* symbol,
            int64_t min,
            uint64_t max,
            bool is_signed,
            number_class_t number_class) : compiler::type(
                                                    module,
                                                    parent_scope,
                                                    element_type_t::numeric_type,
                                                    symbol),
                                                _min(min),
                                                _max(max),
                                                _is_signed(is_signed),
                                                _number_class(number_class) {
    }

    bool numeric_type::on_type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        auto other_numeric_type = dynamic_cast<compiler::numeric_type*>(other);
        if (other_numeric_type == nullptr)
            return false;

        if (id() == other_numeric_type->id())
            return true;

        if (other_numeric_type->number_class() == number_class_t::floating_point) {
            return _number_class == number_class_t::floating_point;
        }

        if (other_numeric_type->is_signed() && !is_signed())
            return false;

        if (options.strict)
            return other_numeric_type->size_in_bytes() == size_in_bytes();
        else
            return other_numeric_type->size_in_bytes() <= size_in_bytes();
    }

    int64_t numeric_type::min() const {
        return _min;
    }

    uint64_t numeric_type::max() const {
        return _max;
    }

    bool numeric_type::is_signed() const {
        return _is_signed;
    }

    number_class_t numeric_type::on_number_class() const {
        return _number_class;
    }

    bool numeric_type::on_initialize(compiler::session& session) {
        auto it = s_types_map.find(symbol()->name());
        if (it == s_types_map.end())
            return false;
        alignment(it->second->size_in_bytes);
        size_in_bytes(it->second->size_in_bytes);
        return true;
    }

}