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

#include <common/source_file.h>
#include "vm_types.h"
#include "label_map.h"
#include "assembly_listing.h"
#include "register_allocator.h"

namespace basecode::vm {

    class assembler {
    public:
        assembler(
            vm::terp* terp,
            vm::register_allocator* allocator);

        bool assemble(
            common::result& r,
            const label_map& labels);

        void disassemble();

        bool assemble_from_source(
            common::result& r,
            label_map& labels,
            common::term_stream_builder* term_builder,
            common::source_file& source_file,
            vm::basic_block* block,
            void* data);

        vm::assembly_listing& listing();

        vm::basic_block_list_t& blocks();

        bool initialize(common::result& r);

        assembler_named_ref_t* find_named_ref(
            const std::string& name,
            vm::assembler_named_ref_type_t type);

        assembler_named_ref_t* make_named_ref(
            assembler_named_ref_type_t type,
            const std::string& name,
            vm::op_sizes size = vm::op_sizes::qword);

        bool has_local(const std::string& name) const;

        const assembly_symbol_resolver_t& resolver() const;

        void resolver(const assembly_symbol_resolver_t& resolver);

        const int64_t frame_offset(const std::string& name) const;

        const vm::assembler_local_t* local(const std::string& name) const;

    private:
        void disassemble(
            basic_block* block,
            uint64_t& address);

        bool resolve_labels(
            common::result& r,
            const label_map& labels);

        bool apply_addresses(common::result& r);

    private:
        vm::terp* _terp = nullptr;
        basic_block_list_t _blocks {};
        uint64_t _location_counter = 0;
        vm::assembly_listing _listing {};
        assembly_symbol_resolver_t _resolver;
        vm::register_allocator* _register_allocator = nullptr;
        std::unordered_map<std::string, int64_t> _frame_offsets {};
        std::unordered_map<std::string, assembler_local_t> _locals {};
        std::unordered_map<std::string, assembler_named_ref_t> _named_refs {};
    };

}

