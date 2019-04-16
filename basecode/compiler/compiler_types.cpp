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
#include "elements/identifier.h"
#include "elements/type_reference.h"
#include "elements/identifier_reference.h"

namespace basecode::compiler {

    element_list_t clone(
            compiler::session& session,
            const element_list_t& list) {
        element_list_t copy {};
        for (auto e : list)
            copy.push_back(e->clone(session));
        return copy;
    }

    type_reference_list_t clone(
            compiler::session& session,
            const type_reference_list_t& list) {
        type_reference_list_t copy {};
        for (auto e : list)
            copy.push_back(dynamic_cast<compiler::type_reference*>(e->clone(session)));
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