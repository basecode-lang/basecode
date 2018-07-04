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

#include <vm/assembler.h>
#include "numeric_type.h"
#include "initializer.h"
#include "block.h"

namespace basecode::compiler {

    block::block(
        block* parent,
        element_type_t type) : element(parent, type) {
    }

    bool block::emit(
            common::result& r,
            vm::assembler& assembler) {
        return !r.is_failed();
    }

    void block::add_symbols(
            common::result& r,
            vm::segment_t* segment,
            const identifier_list_t& list) {
        for (auto var : list) {
            switch (var->type()->element_type()) {
                case element_type_t::bool_type:
                case element_type_t::numeric_type: {
                    auto symbol = segment->symbol(
                        var->name(),
                        vm::integer_symbol_type_for_size(var->type()->size_in_bytes()));
                    uint64_t value;
                    if (var->as_integer(value)) {
                        symbol->value.int_value = value;
                    }
                    break;
                }
                case element_type_t::array_type: {
                    break;
                }
                case element_type_t::string_type: {
                    break;
                }
                case element_type_t::composite_type: {
                    break;
                }
                default:
                    break;
            }
        }
    }

    bool block::define_data(
            common::result& r,
            vm::assembler& assembler) {
        if (_identifiers.empty())
            return true;

        auto constant_init = _identifiers.constants(true);
        if (!constant_init.empty()) {
            auto section = assembler.segment(
                fmt::format("rodata_{}", id()),
                vm::segment_type_t::constant,
                assembler.location_counter());
            section->initialized = true;
            add_symbols(r, section, constant_init);
        }

        auto constant_uninit = _identifiers.constants(false);
        if (!constant_uninit.empty()) {
            auto section = assembler.segment(
                fmt::format("bss_{}", id()),
                vm::segment_type_t::constant,
                assembler.location_counter());
            add_symbols(r, section, constant_uninit);
        }

        auto global_init = _identifiers.globals(true);
        if (!global_init.empty()) {
            auto section = assembler.segment(
                fmt::format("data_{}", id()),
                vm::segment_type_t::data,
                assembler.location_counter());
            add_symbols(r, section, global_init);
        }

        auto global_uninit = _identifiers.globals(false);
        if (!global_uninit.empty()) {
            auto section = assembler.segment(
                fmt::format("bss_data_{}", id()),
                vm::segment_type_t::data,
                assembler.location_counter());
            add_symbols(r, section, global_uninit);
        }

        return !r.is_failed();
    }

    type_map_t& block::types() {
        return _types;
    }

    block_list_t& block::blocks() {
        return _blocks;
    }

    comment_list_t& block::comments() {
        return _comments;
    }

    statement_list_t& block::statements() {
        return _statements;
    }

    identifier_map_t& block::identifiers() {
        return _identifiers;
    }

};