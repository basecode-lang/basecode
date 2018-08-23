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

#include <common/defer.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "program.h"
#include "intrinsic.h"
#include "identifier.h"
#include "argument_list.h"
#include "symbol_element.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    intrinsic::intrinsic(
        compiler::module* module,
        compiler::block* parent_scope,
        compiler::identifier_reference* reference,
        compiler::argument_list* args) : element(module, parent_scope, element_type_t::intrinsic),
                                         _arguments(args),
                                         _reference(reference) {
    }

    bool intrinsic::on_infer_type(
            const compiler::session& session,
            type_inference_result_t& result) {
        return false;
    }

    compiler::argument_list* intrinsic::arguments() {
        return _arguments;
    }

    compiler::identifier_reference* intrinsic::reference() {
        return _reference;
    }

    void intrinsic::on_owned_elements(element_list_t& list) {
        if (_arguments != nullptr)
            list.emplace_back(_arguments);
    }

    void intrinsic::reference(compiler::identifier_reference* value) {
        _reference = value;
    }

};