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

    intrinsic* intrinsic::intrinsic_for_call(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            const std::string& name) {
        auto& builder = session.builder();

        if (name == "size_of") {
            return builder.make_size_of_intrinsic(
                parent_scope,
                args);
        } else if (name == "alloc") {
            return builder.make_alloc_intrinsic(
                parent_scope,
                args);
        } else if (name == "free") {
            return builder.make_free_intrinsic(
                parent_scope,
                args);
        }

        return nullptr;
    }

    intrinsic::intrinsic(
        compiler::module* module,
        compiler::block* parent_scope,
        compiler::argument_list* args) : element(module, parent_scope, element_type_t::intrinsic),
                                         _arguments(args) {
    }

    compiler::argument_list* intrinsic::arguments() {
        return _arguments;
    }

    void intrinsic::on_owned_elements(element_list_t& list) {
        if (_arguments != nullptr)
            list.emplace_back(_arguments);
    }

};