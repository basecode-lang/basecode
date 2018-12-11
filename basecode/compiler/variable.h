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

#include <string>
#include <cstdint>
#include <vm/terp.h>
#include <fmt/format.h>
#include <vm/assembler.h>
#include <common/defer.h>
#include "compiler_types.h"

namespace basecode::compiler {

    struct variable_register_t {
        explicit variable_register_t(vm::assembler* assembler);

        bool reserve();

        void release();

        bool matches(vm::register_t* other_reg);

        vm::register_t reg;
        bool allocated = false;
        bool no_release = false;
        vm::assembler* assembler = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    struct root_and_offset_t {
        int64_t offset = 0;
        std::string path {};
        variable* root = nullptr;
        compiler::identifier* identifier = nullptr;
    };

    class session;

    class variable {
    public:
        using flags_value_t = uint8_t;
        enum flags_t : uint8_t {
            f_none      = 0b00000000,
            f_addressed = 0b00000001,
            f_read      = 0b00000010,
            f_written   = 0b00000100,
            f_copied    = 0b00001000,
            f_activated = 0b00010000,
        };

        variable(
            compiler::session& session,
            compiler::element* element);

        bool field(
            compiler::element* element,
            variable_handle_t& handle,
            bool activate = true);

        bool field(
            const std::string& name,
            variable_handle_t& handle,
            compiler::element* element = nullptr,
            bool activate = true);

        bool read();

        bool write(
            vm::op_sizes size,
            uint64_t value);

        bool write();

        bool activate();

        bool deactivate();

        bool initialize();

        variable* parent();

        compiler::field* field();

        bool is_activated() const;

        bool write(variable* value);

        compiler::element* element();

        emit_context_t& emit_context();

        const vm::register_t& value_reg() const;

        // XXX: temporary for testing
        const emit_result_t& emit_result() const;

        bool address(bool include_offset = false);

        const vm::register_t& address_reg() const;

        const infer_type_result_t& type_result() const;

        bool copy(variable* value, uint64_t size_in_bytes);

    private:
        bool flag(variable::flags_t f) const;

        void flag(variable::flags_t f, bool value);

        bool walk_to_root_and_calculate_offset(root_and_offset_t& rot);

    private:
        emit_result_t _result {};
        root_and_offset_t _rot {};
        emit_context_t _context {};
        variable_register_t _value;
        variable* _parent = nullptr;
        compiler::session& _session;
        variable_register_t _address;
        infer_type_result_t _type {};
        compiler::field* _field = nullptr;
        compiler::element* _element = nullptr;
        flags_value_t _flags = flags_t::f_none;
    };

};

