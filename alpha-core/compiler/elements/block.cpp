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
#include "block.h"
#include "initializer.h"
#include "numeric_type.h"

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
                case element_type_t::any_type:
                case element_type_t::type_info:
                case element_type_t::array_type:
                case element_type_t::string_type:
                case element_type_t::composite_type: {
                    auto size_in_bytes = var->type()->size_in_bytes();
                    auto symbol = segment->symbol(
                        var->name(),
                        vm::symbol_type_t::bytes,
                        size_in_bytes);
                    if (var->initializer() != nullptr)
                        symbol->pending_address_from_id = var->initializer()->id();
                    symbol->value.int_value = 0;
                    break;
                }
                default:
                    break;
            }
        }
    }

    bool block::define_data(
            common::result& r,
            string_set_t& interned_strings,
            vm::assembler& assembler) {
        // look for constant strings

        if (!_identifiers.empty()) {
            auto constant_init = _identifiers.constants(true);
            if (!constant_init.empty()) {
                auto section = assembler.segment(
                    fmt::format("rodata_{}", id()),
                    vm::segment_type_t::constant,
                    assembler.location_counter());
                section->initialized = true;
                add_symbols(r, section, constant_init);

                assembler.location_counter(assembler.location_counter() + section->offset);
            }

            auto constant_uninit = _identifiers.constants(false);
            if (!constant_uninit.empty()) {
                auto section = assembler.segment(
                    fmt::format("bss_{}", id()),
                    vm::segment_type_t::constant,
                    assembler.location_counter());
                add_symbols(r, section, constant_uninit);

                assembler.location_counter(assembler.location_counter() + section->offset);
            }

            auto global_init = _identifiers.globals(true);
            if (!global_init.empty()) {
                auto section = assembler.segment(
                    fmt::format("data_{}", id()),
                    vm::segment_type_t::data,
                    assembler.location_counter());
                add_symbols(r, section, global_init);

                assembler.location_counter(assembler.location_counter() + section->offset);
            }

            auto global_uninit = _identifiers.globals(false);
            if (!global_uninit.empty()) {
                auto section = assembler.segment(
                    fmt::format("bss_data_{}", id()),
                    vm::segment_type_t::data,
                    assembler.location_counter());
                add_symbols(r, section, global_uninit);

                assembler.location_counter(assembler.location_counter() + section->offset);
            }
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