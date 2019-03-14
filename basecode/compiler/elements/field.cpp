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

#include <common/bytes.h>
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
            uint8_t padding,
            bool is_variadic): element(module, parent_scope, element_type_t::field),
                               _padding(padding),
                               _offset(offset),
                               _is_variadic(is_variadic),
                               _declaration(decl) {
    }

    uint8_t field::padding() const {
        return _padding;
    }

    bool field::is_variadic() const {
        return _is_variadic;
    }

    uint64_t field::alignment() const {
        if (_declaration != nullptr) {
            auto type_ref = _declaration->identifier()->type_ref();
            if (type_ref != nullptr)
                return type_ref->type()->alignment();
        }
        return 0;
    }

    uint64_t field::end_offset() const {
        return common::align(_offset + size_in_bytes(), alignment());
    }

    void field::offset(uint64_t value) {
        _offset = value;
    }

    size_t field::size_in_bytes() const {
        size_t size = 0;
        if (_declaration != nullptr) {
            auto type_ref = _declaration->identifier()->type_ref();
            if (type_ref != nullptr)
                size = type_ref->type()->size_in_bytes();
        }
        size += _padding;
        return size;
    }

    uint64_t field::start_offset() const {
        return common::align(_offset, alignment());
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

    bool field::on_as_identifier(compiler::identifier*& value) const {
        value = nullptr;

        if (_declaration == nullptr)
            return false;

        return _declaration->as_identifier(value);
    }

}