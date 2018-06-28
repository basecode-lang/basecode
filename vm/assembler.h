// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include "instruction_emitter.h"

namespace basecode::vm {

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

    struct segment_t {
        uint64_t size;
        uint64_t address;
        segment_type_t type;
    };

    struct symbol_t {
        uint64_t address;
        std::string name;
        segment_type_t type;
    };

    class assembler {
    public:
        explicit assembler(vm::terp* terp);

        void symbol(
            const std::string& name,
            segment_type_t type,
            uint64_t address);

        bool assemble(
            common::result& r,
            std::istream& source);

        void define_data(float value);

        void define_data(double value);

        instruction_emitter& emitter();

        void define_data(uint8_t value);

        void define_data(uint16_t value);

        void define_data(uint32_t value);

        void define_data(uint64_t value);

        uint64_t location_counter() const;

        void location_counter(uint64_t value);

        segment_t* segment(segment_type_t type);

        symbol_t* symbol(const std::string& name);

        void define_string(const std::string& value);

        void segment(segment_type_t type, uint64_t address);

    private:
        vm::terp* _terp = nullptr;
        instruction_emitter _emitter;
        uint64_t _location_counter = 0;
        std::unordered_map<std::string, symbol_t> _symbols {};
        std::unordered_map<segment_type_t, segment_t> _segments {};
    };

};

