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
#include <vm/instruction_block.h>
#include "block.h"
#include "for_element.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    for_element::for_element(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::identifier_reference* ref,
            compiler::block* body) : element(module, parent_scope, element_type_t::for_e),
                                     _body(body),
                                     _ref(ref) {
    }

    compiler::block* for_element::body() {
        return _body;
    }

    compiler::identifier_reference* for_element::ref() {
        return _ref;
    }

    bool for_element::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();
        block->comment("XXX: implement for", 4);
        return true;
    }

    void for_element::on_owned_elements(element_list_t& list) {
        if (_ref != nullptr)
            list.emplace_back(_ref);

        if (_body != nullptr)
            list.emplace_back(_body);
    }

};