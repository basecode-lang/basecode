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

#pragma once

#include <map>
#include <string>
#include <memory>
#include <unordered_map>
#include "element.h"

namespace basecode::compiler {

    class field : public element {
    public:
        field(
            compiler::module* module,
            block* parent_scope,
            compiler::declaration* decl,
            uint64_t offset,
            uint8_t padding = 0,
            bool is_variadic = false);

        field_tag_t tag() const;

        uint8_t padding() const;

        bool is_variadic() const;

        void tag(field_tag_t tag);

        uint64_t alignment() const;

        void offset(uint64_t value);

        uint64_t end_offset() const;

        size_t size_in_bytes() const;

        uint64_t start_offset() const;

        compiler::identifier* identifier();

        compiler::declaration* declaration();

    protected:
        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        void on_owned_elements(element_list_t& list) override;

        bool on_as_identifier(compiler::identifier*& value) const override;

    private:
        field_tag_t _tag{};
        uint8_t _padding = 0;
        uint64_t _offset = 0;
        bool _is_variadic = false;
        compiler::declaration* _declaration;
    };

}