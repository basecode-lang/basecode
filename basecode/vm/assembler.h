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
            const assembly_symbol_resolver_t& resolver);

        instruction_block* pop_block();

        vm::assembly_listing& listing();

        segment_list_t segments() const;

        bool assemble(common::result& r);

        bool initialize(common::result& r);

        instruction_block* current_block();

        bool allocate_reg(register_t& reg);

        void free_reg(const register_t& reg);

        instruction_block* make_basic_block();

        bool resolve_labels(common::result& r);

        bool apply_addresses(common::result& r);

        void push_block(instruction_block* block);

        label* find_label(const std::string& name);

        void disassemble(instruction_block* block);

        label_ref_t* find_label_ref(common::id_t id);

        vm::segment* segment(const std::string& name);

        vm::label* make_label(const std::string& name);

        instruction_block* block(common::id_t id) const;

        label_ref_t* make_label_ref(const std::string& label_name);

        template <typename T>
        T* find_in_blocks(const std::function<T* (instruction_block*)>& callable) {
            for (auto block : _blocks) {
                auto found = callable(block);
                if (found != nullptr)
                    return found;
            }
            return nullptr;
        }

    private:
        std::vector<label_ref_t*> label_references();

        void register_block(instruction_block* block);

    private:
        vm::terp* _terp = nullptr;
        uint64_t _location_counter = 0;
        vm::assembly_listing _listing {};
        std::vector<instruction_block*> _blocks {};
        register_allocator_t _register_allocator {};
        std::stack<instruction_block*> _block_stack {};
        std::unordered_map<std::string, vm::label*> _labels {};
        std::unordered_map<std::string, vm::segment> _segments {};
        std::unordered_map<common::id_t, label_ref_t> _unresolved_labels {};
        std::unordered_map<std::string, common::id_t> _label_to_unresolved_ids {};
        std::unordered_map<common::id_t, vm::instruction_block*> _block_registry {};
    };

};

