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
#include "program.h"
#include "map_type.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    std::string map_type::name_for_map(
            compiler::type_reference* key_type,
            compiler::type_reference* value_type) {
        return fmt::format("__map_{}_{}__", key_type->name(), value_type->name());
    }

    map_type::map_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::type_reference* key_type,
            compiler::type_reference* value_type) : compiler::composite_type(
                                                        module,
                                                        parent_scope,
                                                        composite_types_t::struct_type,
                                                        scope,
                                                        nullptr,
                                                        element_type_t::map_type),
                                                    _key_type(key_type),
                                                    _value_type(value_type) {
    }

    compiler::type_reference* map_type::key_type() {
        return _key_type;
    }

    compiler::type_reference* map_type::value_type() {
        return _value_type;
    }

    type_access_model_t map_type::on_access_model() const {
        return type_access_model_t::pointer;
    }

    bool map_type::on_initialize(compiler::session& session) {
//        auto& scope_manager = session.scope_manager();
        auto& builder = session.builder();

        auto type_symbol = builder.make_symbol(
            parent_scope(),
            name_for_map(_key_type, _value_type));
        symbol(type_symbol);
        type_symbol->parent_element(this);

        // XXX: need to add fields to the map type

        return composite_type::on_initialize(session);
    }

    std::string map_type::name(const std::string& alias) const {
        return alias.empty() ? name_for_map(_key_type, _value_type) : alias;
    }

};