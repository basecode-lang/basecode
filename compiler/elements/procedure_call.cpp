// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include "procedure_call.h"

namespace basecode::compiler {

    procedure_call::procedure_call(
        compiler::element* parent,
        compiler::identifier* identifier,
        compiler::argument_list* args) : element(parent, element_type_t::proc_call),
                                         _arguments(args),
                                         _identifier(identifier) {
    }

    compiler::identifier* procedure_call::identifier() {
        return _identifier;
    }

    compiler::argument_list* procedure_call::arguments() {
        return _arguments;
    }

};