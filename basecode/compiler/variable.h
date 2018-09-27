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

//    struct variable_t {
//        bool init(compiler::session& session);
//
//        bool read(compiler::session& session);
//
//        bool write(compiler::session& session);
//
//        void make_live(compiler::session& session);
//
//        void make_dormant(compiler::session& session);
//
//        std::string name;
//        bool live = false;
//        bool written = false;
//        identifier_usage_t usage;
//        bool requires_read = false;
//        bool address_loaded = false;
//        variable_register_t value_reg;
//        compiler::type* type = nullptr;
//        variable_register_t address_reg;
//        vm::stack_frame_entry_t* frame_entry = nullptr;
//    };

    struct variable_register_t {
        explicit variable_register_t(vm::assembler* assembler);

        bool reserve();

        void release();

        bool matches(vm::register_t* other_reg);

        vm::register_t reg;
        bool allocated = false;
        vm::assembler* assembler = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

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

        bool read();

        bool write();

        bool activate();

        bool deactivate();

        bool initialize();

        variable* parent();

        compiler::field* field();

        bool is_activated() const;

        bool write(uint64_t value);

        compiler::element* element();

        const vm::register_t& value_reg() const;

        variable* field(const std::string& name);

        const vm::register_t& address_reg() const;

    private:
        bool flag(variable::flags_t f) const;

        void flag(variable::flags_t f, bool value);

    private:
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

