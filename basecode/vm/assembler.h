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
#include "segment.h"
#include "assembly_listing.h"
#include "register_allocator.h"

namespace basecode::vm {

    class assembler {
    public:
        explicit assembler(vm::terp* terp);

        virtual ~assembler();

        void disassemble();

        vm::segment* segment(
            const std::string& name,
            segment_type_t type);

        bool assemble_from_source(
            common::result& r,
            common::source_file& source_file,
            vm::basic_block* block,
            void* data);

        vm::assembly_listing& listing();

        segment_list_t segments() const;

        basic_block* make_basic_block();

        bool assemble(common::result& r);

        bool initialize(common::result& r);

        bool allocate_reg(register_t& reg);

        void free_reg(const register_t& reg);

        void disassemble(basic_block* block);

        bool resolve_labels(common::result& r);

        assembler_named_ref_t* find_named_ref(
            const std::string& name,
            vm::assembler_named_ref_type_t type);

        assembler_named_ref_t* make_named_ref(
            assembler_named_ref_type_t type,
            const std::string& name,
            vm::op_sizes size = vm::op_sizes::qword);

        bool apply_addresses(common::result& r);

        basic_block* block(common::id_t id) const;

        label* find_label(const std::string& name);

        vm::segment* segment(const std::string& name);

        bool has_local(const std::string& name) const;

        vm::label* make_label(const std::string& name);

        const assembly_symbol_resolver_t& resolver() const;

        void resolver(const assembly_symbol_resolver_t& resolver);

        const int64_t frame_offset(const std::string& name) const;

        const vm::assembler_local_t* local(const std::string& name) const;

    private:
        void free_locals();

        void register_block(basic_block* block);

    private:
        vm::terp* _terp = nullptr;
        uint64_t _location_counter = 0;
        vm::assembly_listing _listing {};
        assembly_symbol_resolver_t _resolver;
        std::vector<basic_block*> _blocks {};
        register_allocator_t _register_allocator {};
        std::unordered_map<std::string, vm::label*> _labels {};
        std::unordered_map<std::string, vm::segment> _segments {};
        std::unordered_map<std::string, int64_t> _frame_offsets {};
        std::unordered_map<std::string, assembler_local_t> _locals {};
        std::unordered_map<std::string, assembler_named_ref_t> _named_refs {};
        std::unordered_map<common::id_t, vm::basic_block*> _block_registry {};
    };

};

