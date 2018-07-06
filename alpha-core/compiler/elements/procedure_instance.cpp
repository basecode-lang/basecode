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

#include "procedure_instance.h"

namespace basecode::compiler {

    procedure_instance::procedure_instance(
            element* parent,
            compiler::type* procedure_type,
            block* scope) : element(parent, element_type_t::proc_instance),
                            _scope(scope),
                            _procedure_type(procedure_type) {
    }

    bool procedure_instance::on_emit(
            common::result& r,
            vm::assembler& assembler) {
        return element::on_emit(r, assembler);
    }

    block* procedure_instance::scope() {
        return _scope;
    }

    compiler::type* procedure_instance::procedure_type() {
        return _procedure_type;
    }

};