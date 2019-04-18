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

#include <fmt/format.h>
#include "compiler_types.h"
#include "elements/field.h"
#include "elements/label.h"
#include "elements/identifier.h"
#include "elements/type_reference.h"
#include "elements/identifier_reference.h"

namespace basecode::compiler {

    field_map_t clone(
            compiler::session& session,
            compiler::block* new_scope,
            const field_map_t& fields) {
        field_map_t map {};
        for (const auto& fld : fields.as_list())
            map.add(fld->clone<compiler::field>(session, new_scope));
        return map;
    }

    element_list_t clone(
            compiler::session& session,
            compiler::block* new_scope,
            const element_list_t& list) {
        element_list_t copy {};
        for (auto e : list)
            copy.push_back(e->clone<compiler::element>(session, new_scope));
        return copy;
    }

    label_list_t clone(
            compiler::session& session,
            compiler::block* new_scope,
            const label_list_t& list) {
        label_list_t copy {};
        for (auto e : list)
            copy.push_back(e->clone<compiler::label>(session, new_scope));
        return copy;
    }

    type_reference_list_t clone(
            compiler::session& session,
            compiler::block* new_scope,
            const type_reference_list_t& list) {
        type_reference_list_t copy {};
        for (auto e : list)
            copy.push_back(e->clone<compiler::type_reference>(session, new_scope));
        return copy;
    }

    identifier_reference_list_t clone(
            compiler::session& session,
            compiler::block* new_scope,
            const identifier_reference_list_t& list) {
        identifier_reference_list_t copy {};
        for (auto e : list)
            copy.push_back(e->clone<compiler::identifier_reference>(session, new_scope));
        return copy;
    }

    ///////////////////////////////////////////////////////////////////////////

    std::string offset_result_t::label_name() const {
        if (base_ref == nullptr)
            return {};
        auto label = base_ref->identifier()->label_name();
        for (const auto field : fields) {
            label += fmt::format(
                "_{}",
                field->identifier()->label_name());
        }
        return label;
    }

}