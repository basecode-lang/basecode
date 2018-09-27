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

#include "vm_types.h"

namespace basecode::vm {

    class symbol {
    public:
        symbol(
            const std::string& name,
            symbol_type_t type,
            uint64_t offset,
            size_t size = 0);

        void value(void* v);

        size_t size() const;

        void value(double v);

        void value(uint64_t v);

        uint64_t offset() const;

        std::string name() const;

        symbol_type_t type() const;

        common::id_t pending_address_from_id() const;

        void pending_address_from_id(common::id_t value);

    private:
        size_t _size;
        uint64_t _offset;
        std::string _name;
        symbol_type_t _type;
        common::id_t _pending_address_from_id = 0;

        union {
            double float_value;
            uint64_t int_value;
            void* byte_array_value;
        } _value;
    };

};

