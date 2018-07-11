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

#include <any>
#include <cstdint>
#include <vm/terp.h>
#include <vm/assembler.h>
#include <common/result.h>
#include <common/id_pool.h>
#include "element_types.h"

namespace basecode::compiler {

    enum class emit_context_type_t {
        empty,
        if_element,
        procedure_type
    };

    enum class emit_access_type_t {
        read,
        write
    };

    struct procedure_type_data_t {
        std::string identifier_name;
    };

    struct if_data_t {
        enum class logical_group_t {
            no_group,
            and_group,
            or_group,
        };
        std::string true_branch_label;
        std::string false_branch_label;
        logical_group_t group_type = logical_group_t::no_group;
    };

    struct emit_context_t {
        template <typename T>
        T* top() {
            if (data_stack.empty())
                return nullptr;
            try {
                return std::any_cast<T>(&data_stack.top());
            } catch (const std::bad_any_cast& e) {
                return nullptr;
            }
        }

        void pop() {
            if (data_stack.empty())
                return;
            data_stack.pop();
        }

        void push_if(
                const std::string& true_label_name,
                const std::string& false_label_name) {
            data_stack.push(std::any(if_data_t {
                .true_branch_label = true_label_name,
                .false_branch_label = false_label_name,
            }));
        }

        void pop_access() {
            if (access_stack.empty())
                return;
            access_stack.pop();
        }

        void clear_scratch_registers() {
            while (!scratch_registers.empty())
                scratch_registers.pop();
        }

        emit_access_type_t current_access() const {
            if (access_stack.empty())
                return emit_access_type_t::read;
            return access_stack.top();
        }

        void push_access(emit_access_type_t type) {
            access_stack.push(type);
        }

        bool has_scratch_register() const {
            return !scratch_registers.empty();
        }

        vm::i_registers_t pop_scratch_register() {
            if (scratch_registers.empty())
                return vm::i_registers_t::i0;

            auto reg = scratch_registers.top();
            scratch_registers.pop();
            return reg;
        }

        void push_scratch_register(vm::i_registers_t reg) {
            scratch_registers.push(reg);
        }

        void push_procedure_type(const std::string& name) {
            data_stack.push(std::any(procedure_type_data_t {
                .identifier_name = name
            }));
        }

        std::stack<std::any> data_stack {};
        std::stack<emit_access_type_t> access_stack {};
        std::stack<vm::i_registers_t> scratch_registers {};
    };

    class element {
    public:
        element(
            element* parent,
            element_type_t type);

        virtual ~element();

        bool emit(
            common::result& r,
            vm::assembler& assembler,
            emit_context_t& context);

        element* parent();

        common::id_t id() const;

        bool is_constant() const;

        bool fold(common::result& r);

        attribute_map_t& attributes();

        bool as_bool(bool& value) const;

        bool as_float(double& value) const;

        element_type_t element_type() const;

        virtual std::string label_name() const;

        bool as_integer(uint64_t& value) const;

        bool as_string(std::string& value) const;

        compiler::type* infer_type(const compiler::program* program);

    protected:
        virtual bool on_emit(
            common::result& r,
            vm::assembler& assembler,
            emit_context_t& context);

        virtual bool on_is_constant() const;

        virtual bool on_fold(common::result& r);

        virtual bool on_as_bool(bool& value) const;

        virtual bool on_as_float(double& value) const;

        virtual bool on_as_integer(uint64_t& value) const;

        virtual bool on_as_string(std::string& value) const;

        virtual compiler::type* on_infer_type(const compiler::program* program);

    private:
        common::id_t _id;
        element* _parent = nullptr;
        attribute_map_t _attributes {};
        element_type_t _element_type = element_type_t::element;
    };

};

