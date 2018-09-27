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

#include "type.h"
#include "field.h"
#include "identifier.h"
#include "declaration.h"
#include "initializer.h"
#include "type_reference.h"

namespace basecode::compiler {

    field::field(
            compiler::module* module,
            block* parent_scope,
            compiler::declaration* decl,
            uint64_t offset,
            uint8_t padding): element(module, parent_scope, element_type_t::field),
                              _padding(padding),
                              _offset(offset),
                              _declaration(decl) {
    }

    uint8_t field::padding() const {
        return _padding;
    }

    uint64_t field::end_offset() const {
        return _offset + size_in_bytes();
    }

    size_t field::size_in_bytes() const {
        size_t size = 0;
        if (_declaration != nullptr)  {
            auto type_ref = _declaration->identifier()->type_ref();
            if (type_ref != nullptr)
                size = type_ref->type()->size_in_bytes();
        }
        size += _padding;
        return size;
    }

    uint64_t field::start_offset() const {
        return _offset;
    }

    compiler::identifier* field::identifier() {
        return _declaration->identifier();
    }

    compiler::declaration* field::declaration() {
        return _declaration;
    }

    void field::on_owned_elements(element_list_t& list) {
        if (_declaration != nullptr)
            list.emplace_back(_declaration);
    }

};