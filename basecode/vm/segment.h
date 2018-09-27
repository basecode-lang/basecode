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

#include "symbol.h"
#include "vm_types.h"

namespace basecode::vm {

    class segment {
    public:
        segment(
            const std::string& name,
            segment_type_t type);

        vm::symbol* symbol(
            const std::string& name,
            symbol_type_t type,
            size_t size = 0);

        size_t size() const;

        std::string name() const;

        bool initialized() const;

        segment_type_t type() const;

        void initialized(bool value);

        symbol_list_t symbols() const;

        vm::symbol* symbol(const std::string& name);

    private:
        std::string _name;
        segment_type_t _type;
        uint64_t _offset = 0;
        uint64_t _address = 0;
        bool _initialized = false;
        std::unordered_map<std::string, vm::symbol> _symbols {};
    };

};

