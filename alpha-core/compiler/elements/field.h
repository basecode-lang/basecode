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
#include "type.h"
#include "element.h"
#include "initializer.h"

namespace basecode::compiler {

    class field : public element {
    public:
        field(
            compiler::module* module,
            block* parent_scope,
            compiler::identifier* identifier,
            uint64_t offset,
            uint8_t padding = 0);

        uint8_t padding() const;

        uint64_t end_offset() const;

        size_t size_in_bytes() const;

        uint64_t start_offset() const;

        compiler::identifier* identifier();

    protected:
        void on_owned_elements(element_list_t& list) override;

    private:
        uint8_t _padding = 0;
        uint64_t _offset = 0;
        compiler::identifier* _identifier;
    };

};