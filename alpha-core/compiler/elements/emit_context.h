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

#include <stack>
#include <string>
#include <vm/terp.h>
#include <boost/any.hpp>
#include <vm/assembler.h>

namespace basecode::compiler {

    enum class emit_access_type_t {
        read,
        write
    };

    struct block_data_t {
        bool recurse = true;
    };

    struct if_data_t {
        std::string true_branch_label;
        std::string false_branch_label;
    };

    class program;

    struct emit_context_t {
        emit_context_t(
            vm::terp* terp,
            vm::assembler* assembler,
            compiler::program* program);

        template <typename T>
        T* top() {
            if (data_stack.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&data_stack.top());
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        void pop();

        void push_if(
            const std::string& true_label_name,
            const std::string& false_label_name);

        void pop_access();

        void push_block(bool recurse);

        void clear_scratch_registers();

        bool has_scratch_register() const;

        vm::i_registers_t pop_scratch_register();

        emit_access_type_t current_access() const;

        void push_access(emit_access_type_t type);

        void push_scratch_register(vm::i_registers_t reg);

        vm::terp* terp = nullptr;
        vm::assembler* assembler = nullptr;
        compiler::program* program = nullptr;
        std::stack<boost::any> data_stack {};
        std::stack<emit_access_type_t> access_stack {};
        std::stack<vm::i_registers_t> scratch_registers {};
    };

};

