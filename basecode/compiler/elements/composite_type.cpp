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

#include <common/bytes.h>
#include <compiler/session.h>
#include "field.h"
#include "block.h"
#include "identifier.h"
#include "initializer.h"
#include "symbol_element.h"
#include "composite_type.h"
#include "type_reference.h"

namespace basecode::compiler {

    composite_type::composite_type(
            compiler::module* module,
            block* parent_scope,
            composite_types_t type,
            compiler::block* scope,
            compiler::symbol_element* symbol,
            element_type_t element_type) : compiler::type(
                                                module,
                                                parent_scope,
                                                element_type,
                                                symbol),
                                           _type(type),
                                           _scope(scope) {
    }

    bool composite_type::is_enum() const {
        return _type == composite_types_t::enum_type;
    }

    bool composite_type::is_union() const {
        return _type == composite_types_t::union_type;
    }

    field_map_t& composite_type::fields() {
        return _fields;
    }

    void composite_type::calculate_size() {
        size_t size = 0;
        switch (_type) {
            case composite_types_t::enum_type: {
                auto type_params = _type_parameters.as_list();
                if (type_params.empty()) {
                    size = 4;
                } else {
                    size = type_params.front()->size_in_bytes();
                }
                break;
            }
            case composite_types_t::union_type: {
                for (auto fld : _fields.as_list()) {
                    fld->offset(0);

                    auto type = fld->identifier()->type_ref()->type();
                    auto composite_type = dynamic_cast<compiler::composite_type*>(type);
                    if (composite_type != nullptr)
                        composite_type->calculate_size();

                    if (fld->size_in_bytes() > size) {
                        size = fld->size_in_bytes();
                    }
                }
                break;
            }
            case composite_types_t::struct_type: {
                for (auto fld : _fields.as_list()) {
                    fld->offset(size);

                    auto type = fld->identifier()->type_ref()->type();
                    auto composite_type = dynamic_cast<compiler::composite_type*>(type);
                    if (composite_type != nullptr)
                        composite_type->calculate_size();

                    size += fld->size_in_bytes();
                    size = common::align(size, fld->alignment());
                }
                break;
            }
        }
        size_in_bytes(size);
    }

    compiler::block* composite_type::scope() {
        return _scope;
    }

    bool composite_type::on_is_constant() const {
        return true;
    }

    type_map_t& composite_type::type_parameters() {
        return _type_parameters;
    }

    composite_types_t composite_type::type() const {
        return _type;
    }

    bool composite_type::is_composite_type() const {
        return true;
    }

    number_class_t composite_type::on_number_class() const {
        return number_class_t::integer;
    }

    bool composite_type::on_type_check(compiler::type* other) {
        return other != nullptr && other->id() == id();
    }

    bool composite_type::has_at_least_one_initializer() const {
        for (auto fld : _fields.as_list()) {
            auto init = fld->identifier()->initializer();
            if (init != nullptr) {
                auto expr = init->expression();
                if (expr != nullptr
                &&  expr->element_type() != element_type_t::uninitialized_literal) {
                    return true;
                }
            }

            auto type = fld->identifier()->type_ref()->type();
            auto composite_type = dynamic_cast<compiler::composite_type*>(type);
            if (composite_type != nullptr) {
                auto result = composite_type->has_at_least_one_initializer();
                if (result)
                    return true;
            }
        }
        return false;
    }

    void composite_type::on_owned_elements(element_list_t& list) {
        for (auto element : _fields.as_list())
            list.emplace_back(element);

        if (_scope != nullptr)
            list.emplace_back(_scope);
    }

    bool composite_type::on_initialize(compiler::session& session) {
        size_t size = 0;
        size_t align = 0;
        switch (_type) {
            case composite_types_t::enum_type: {
                auto type_params = _type_parameters.as_list();
                if (type_params.empty()) {
                    size = 4;
                } else {
                    size = type_params.front()->size_in_bytes();
                }
                align = size;
                break;
            }
            case composite_types_t::union_type: {
                for (auto fld : _fields.as_list()) {
                    if (fld->size_in_bytes() > size)
                        size = fld->size_in_bytes();
                }
                align = sizeof(uint64_t);
                break;
            }
            case composite_types_t::struct_type: {
                for (auto fld : _fields.as_list()) {
                    size += fld->size_in_bytes();
                    size = common::align(size, fld->alignment());
                }
                align = sizeof(uint64_t);
                break;
            }
        }
        size_in_bytes(size);
        alignment(align);
        return true;
    }

}