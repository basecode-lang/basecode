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

#include "assembler.h"

namespace basecode::vm {

    symbol_t::symbol_t(
        const std::string& name,
        symbol_type_t type,
        uint64_t address,
        size_t size) : size(size),
                       address(address),
                       name(name),
                       type(type) {
    }

    ///////////////////////////////////////////////////////////////////////////

    segment_t::segment_t(
        const std::string& name,
        segment_type_t type,
        uint64_t address) : address(address),
                            name(name),
                            type(type) {
    }

    symbol_t* segment_t::symbol(
            const std::string& name,
            symbol_type_t type,
            size_t size) {
        auto type_size = size == 0 ? size_of_symbol_type(type) : size;

        _symbols.insert(std::make_pair(
            name,
            symbol_t(name, type, address + offset, type_size)));

        offset += type_size;

        return symbol(name);
    }

    size_t segment_t::size() const {
        return offset;
    }

    symbol_t* segment_t::symbol(const std::string& name) {
        auto it = _symbols.find(name);
        if (it == _symbols.end())
            return nullptr;
        return &it->second;
    }

    ///////////////////////////////////////////////////////////////////////////

    assembler::assembler(vm::terp* terp) : _terp(terp) {
        _location_counter = _terp->heap_vector(heap_vectors_t::program_start);
    }

    bool assembler::assemble(
            common::result& r,
            std::istream& source) {
        return false;
    }

    segment_t* assembler::segment(
            const std::string& name,
            segment_type_t type,
            uint64_t address) {
        _segments.insert(std::make_pair(
            name,
            segment_t(name, type, address)));
        return segment(name);
    }

    instruction_emitter& assembler::emitter() {
        return _emitter;
    }

    void assembler::define_data(float value) {
        auto heap_address = reinterpret_cast<uint32_t*>(_terp->heap() + _location_counter);
        *heap_address = static_cast<uint32_t>(value);
        _location_counter += sizeof(uint32_t);
    }

    void assembler::define_data(double value) {
        auto heap_address = reinterpret_cast<uint64_t*>(_terp->heap() + _location_counter);
        *heap_address = static_cast<uint64_t>(value);
        _location_counter += sizeof(uint64_t);
    }

    void assembler::define_data(uint8_t value) {
        auto heap_address =_terp->heap() + _location_counter;
        *heap_address = value;
        _location_counter += sizeof(uint8_t);
    }

    void assembler::define_data(uint16_t value) {
        auto heap_address = reinterpret_cast<uint16_t*>(_terp->heap() + _location_counter);
        *heap_address = static_cast<uint16_t>(value);
        _location_counter += sizeof(uint16_t);
    }

    void assembler::define_data(uint32_t value) {
        auto heap_address = reinterpret_cast<uint32_t*>(_terp->heap() + _location_counter);
        *heap_address = static_cast<uint32_t>(value);
        _location_counter += sizeof(uint32_t);
    }

    void assembler::define_data(uint64_t value) {
        auto heap_address = reinterpret_cast<uint64_t*>(_terp->heap() + _location_counter);
        *heap_address = static_cast<uint64_t>(value);
        _location_counter += sizeof(uint64_t);
    }

    uint64_t assembler::location_counter() const {
        return _location_counter;
    }

    void assembler::location_counter(uint64_t value) {
        _location_counter = value;
    }

    segment_t* assembler::segment(const std::string& name) {
        auto it = _segments.find(name);
        if (it == _segments.end())
            return nullptr;
        return &it->second;
    }

    void assembler::define_string(const std::string& value) {
        auto heap_address =_terp->heap() + _location_counter;
        for (auto c : value)
            *heap_address++ = static_cast<uint8_t>(c);
        _location_counter += value.length();
    }

};