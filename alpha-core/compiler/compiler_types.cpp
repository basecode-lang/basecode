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

#include "elements/type.h"
#include "compiler_types.h"
#include "elements/identifier.h"
#include "elements/symbol_element.h"

namespace basecode::compiler {

    std::string type_inference_result_t::name() const {
        if (identifier != nullptr && identifier->type_alias())
            return identifier->symbol()->name();

        if (type != nullptr)
            return type->symbol()->name();

        return "unknown";
    }

};