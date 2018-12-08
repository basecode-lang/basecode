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
#include "stack_frame.h"
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
            stack_frame_t* stack_frame);

        instruction_block* pop_block();

        vm::assembly_listing& listing();

        bool in_procedure_scope() const;

        segment_list_t segments() const;

        bool assemble(common::result& r);

        bool initialize(common::result& r);

        instruction_block* current_block();

        bool allocate_reg(register_t& reg);

        void free_reg(const register_t& reg);

        instruction_block* make_basic_block();

        bool resolve_labels(common::result& r);

        bool apply_addresses(common::result& r);

        instruction_block* make_procedure_block();

        std::vector<instruction_block*>& blocks();

        void push_block(instruction_block* block);

        label* find_label(const std::string& name);

        void disassemble(instruction_block* block);

        label_ref_t* find_label_ref(common::id_t id);

        vm::segment* segment(const std::string& name);

        vm::label* make_label(const std::string& name);

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

        void add_new_block(instruction_block* block);

        bool is_float_register(const std::string& value) const;

        bool is_integer_register(const std::string& value) const;

    private:
        vm::terp* _terp = nullptr;
        uint64_t _location_counter = 0;
        vm::assembly_listing _listing {};
        uint32_t _procedure_block_count = 0;
        std::vector<instruction_block*> _blocks {};
        register_allocator_t _register_allocator {};
        std::stack<instruction_block*> _block_stack {};
        std::unordered_map<std::string, vm::label*> _labels {};
        std::unordered_map<std::string, vm::segment> _segments {};
        std::unordered_map<common::id_t, label_ref_t> _unresolved_labels {};
        std::unordered_map<std::string, common::id_t> _label_to_unresolved_ids {};
    };

};

