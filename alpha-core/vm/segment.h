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

#include <string>
#include <vector>
#include <unordered_map>
#include "symbol.h"

namespace basecode::vm {

    class segment;

    using segment_list_t = std::vector<segment*>;

    enum class segment_type_t {
        code,
        data,
        stack,
        constant,
    };

    static inline std::unordered_map<segment_type_t, std::string> s_segment_type_names = {
        {segment_type_t::code,     "code"},
        {segment_type_t::data,     "data"},
        {segment_type_t::stack,    "stack"},
        {segment_type_t::constant, "constant"}
    };

    static inline std::string segment_type_name(segment_type_t type) {
        auto it = s_segment_type_names.find(type);
        if (it == s_segment_type_names.end())
            return "unknown";
        return it->second;
    }

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

