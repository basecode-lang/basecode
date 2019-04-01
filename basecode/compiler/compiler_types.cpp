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
#include "elements/identifier_reference.h"

namespace basecode::compiler {

    std::string offset_result_t::label_name() const {
        if (base_ref == nullptr)
            return {};
        auto label = base_ref->identifier()->label_name();
        for (size_t i = 0; i < fields.size(); i++) {
            label += fmt::format(
                "_{}",
                fields[i]->identifier()->label_name());
        }
        return label;
    }

}