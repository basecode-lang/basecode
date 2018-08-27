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
#include <vector>
#include <unordered_map>
#include <common/result.h>
#include <common/id_pool.h>
#include <boost/variant.hpp>
#include <common/source_file.h>
#include "terp.h"
#include "segment.h"
#include "assembly_listing.h"
#include "instruction_block.h"
#include "register_allocator.h"

namespace basecode::vm {

    class instruction_block;

    ///////////////////////////////////////////////////////////////////////////

    struct mnemonic_operand_t {
        enum flags : uint8_t {
            none             = 0b00000000,
            integer_register = 0b00000001,
            float_register   = 0b00000010,
            immediate        = 0b00000100,
            pc_register      = 0b00001000,
            sp_register      = 0b00010000,
            fp_register      = 0b00100000,
        };

        uint8_t types = flags::none;
        bool required = false;
    };

    struct mnemonic_t {
        size_t required_operand_count() const {
            size_t count = 0;
            for (const auto& op : operands)
                count += op.required ? 1 : 0;
            return count;
        }

        op_codes code;
        std::vector<mnemonic_operand_t> operands {};
    };

    inline static std::map<std::string, mnemonic_t> s_mnemonics = {
        {
            "NOP",
            mnemonic_t{
                op_codes::nop,
                {}
            }
        },
        {
            "ALLOC",
            mnemonic_t{
                op_codes::alloc,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "FREE",
            mnemonic_t{
                op_codes::free,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                }
            }
        },
        {
            "SIZE",
            mnemonic_t{
                op_codes::size,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register, true},
                }
            }
        },
        {
            "LOAD",
            mnemonic_t{
                op_codes::load,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "STORE",
            mnemonic_t{
                op_codes::store,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "COPY",
            mnemonic_t{
                op_codes::copy,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "CVRT",
            mnemonic_t{
                op_codes::convert,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true},
                }
            }
        },
        {
            "FILL",
            mnemonic_t{
                op_codes::fill,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "MOVE",
            mnemonic_t{
                op_codes::move,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "MOVES",
            mnemonic_t{
                op_codes::moves,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "MOVEZ",
            mnemonic_t{
                op_codes::movez,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "PUSH",
            mnemonic_t{
                op_codes::push,
                {
                    {mnemonic_operand_t::flags::integer_register
                     | mnemonic_operand_t::flags::float_register
                     | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "POP",
            mnemonic_t{
                op_codes::pop,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true}
                }
            }
        },
        {
            "DUP",
            mnemonic_t{
                op_codes::dup,
                {}
            }
        },
        {
            "INC",
            mnemonic_t{
                op_codes::inc,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true}
                }
            }
        },
        {
            "DEC",
            mnemonic_t{
                op_codes::dec,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true}
                }
            }
        },
        {
            "ADD",
            mnemonic_t{
                op_codes::add,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true},
                    {mnemonic_operand_t::flags::integer_register
                     | mnemonic_operand_t::flags::float_register
                     | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register
                     | mnemonic_operand_t::flags::float_register
                     | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "SUB",
            mnemonic_t{
                op_codes::sub,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true},
                    {mnemonic_operand_t::flags::integer_register
                     | mnemonic_operand_t::flags::float_register
                     | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register
                     | mnemonic_operand_t::flags::float_register
                     | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "MUL",
            mnemonic_t{
                op_codes::mul,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true},
                    {mnemonic_operand_t::flags::integer_register
                     | mnemonic_operand_t::flags::float_register
                     | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register
                     | mnemonic_operand_t::flags::float_register
                     | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "DIV",
            mnemonic_t{
                op_codes::div,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true},
                    {mnemonic_operand_t::flags::integer_register
                     | mnemonic_operand_t::flags::float_register
                     | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register
                     | mnemonic_operand_t::flags::float_register
                     | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "MOD",
            mnemonic_t{
                op_codes::mod,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "NEG",
            mnemonic_t{
                op_codes::neg,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "SHR",
            mnemonic_t{
                op_codes::shr,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "SHL",
            mnemonic_t{
                op_codes::shl,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "ROR",
            mnemonic_t{
                op_codes::ror,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "ROL",
            mnemonic_t{
                op_codes::rol,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "AND",
            mnemonic_t{
                op_codes::and_op,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "OR",
            mnemonic_t{
                op_codes::or_op,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "XOR",
            mnemonic_t{
                op_codes::xor_op,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "NOT",
            mnemonic_t{
                op_codes::not_op,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "BIS",
            mnemonic_t{
                op_codes::bis,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "BIC",
            mnemonic_t{
                op_codes::bic,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "TEST",
            mnemonic_t{
                op_codes::test,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "CMP",
            mnemonic_t{
                op_codes::cmp,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "BZ",
            mnemonic_t{
                op_codes::bz,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "BNZ",
            mnemonic_t{
                op_codes::bnz,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "TBZ",
            mnemonic_t{
                op_codes::tbz,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "TBNZ",
            mnemonic_t{
                op_codes::tbnz,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "BNE",
            mnemonic_t{
                op_codes::bne,
                {
                    {mnemonic_operand_t::flags::pc_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "BEQ",
            mnemonic_t{
                op_codes::beq,
                {
                    {mnemonic_operand_t::flags::pc_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "BG",
            mnemonic_t{
                op_codes::bg,
                {
                    {mnemonic_operand_t::flags::pc_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "BGE",
            mnemonic_t{
                op_codes::bge,
                {
                    {mnemonic_operand_t::flags::pc_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "BL",
            mnemonic_t{
                op_codes::bl,
                {
                    {mnemonic_operand_t::flags::pc_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "BLE",
            mnemonic_t{
                op_codes::ble,
                {
                    {mnemonic_operand_t::flags::pc_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "SETZ",
            mnemonic_t{
                op_codes::setz,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                }
            }
        },
        {
            "SETNZ",
            mnemonic_t{
                op_codes::setnz,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                }
            }
        },
        {
            "JSR",
            mnemonic_t{
                op_codes::jsr,
                {
                    {mnemonic_operand_t::flags::pc_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "RTS",
            mnemonic_t{
                op_codes::rts,
                {}
            }
        },
        {
            "JMP",
            mnemonic_t{
                op_codes::jmp,
                {
                    {mnemonic_operand_t::flags::pc_register | mnemonic_operand_t::flags::immediate, true},
                    {mnemonic_operand_t::flags::immediate, false},
                }
            }
        },
        {
            "SWI",
            mnemonic_t{
                op_codes::swi,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "SWAP",
            mnemonic_t{
                op_codes::swap,
                {
                    {mnemonic_operand_t::flags::integer_register, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "TRAP",
            mnemonic_t{
                op_codes::trap,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "FFI",
            mnemonic_t{
                op_codes::ffi,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "META",
            mnemonic_t{
                op_codes::meta,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::immediate, true},
                }
            }
        },
        {
            "EXIT",
            mnemonic_t{
                op_codes::exit,
                {}
            }
        },
    };

    inline static mnemonic_t* mnemonic(const std::string& code) {
        const auto it = s_mnemonics.find(code);
        if (it != s_mnemonics.end()) {
            return &it->second;
        }
        return nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////

    enum class directive_type_t : uint8_t {
        section,
        align,
        db,
        dw,
        dd,
        dq,
        rb,
        rw,
        rd,
        rq
    };

    using directive_param_variant_t = boost::variant<std::string, uint64_t>;

    struct directive_param_t {
        enum flags : uint8_t {
            none        = 0b00000000,
            string      = 0b00000001,
            number      = 0b00000010,
            repeating   = 0b10000000,
        };

        uint8_t type = flags::none;
        bool required = false;
    };

    struct directive_t {
        op_sizes size;
        directive_type_t type;
        std::vector<directive_param_t> params {};
    };

    inline static std::map<std::string, directive_t> s_directives = {
        {
            "SECTION",
            directive_t{
                op_sizes::none,
                directive_type_t::section,
                {
                    {directive_param_t::flags::string, true},
                }
            }
        },
        {
            "ALIGN",
            directive_t{
                op_sizes::byte,
                directive_type_t::align,
                {
                    {directive_param_t::flags::number, true},
                }
            }
        },
        {
            "DB",
            directive_t{
                op_sizes::byte,
                directive_type_t::db,
                {
                    {directive_param_t::flags::number | directive_param_t::flags::repeating, true},
                }
            }
        },
        {
            "DW",
            directive_t{
                op_sizes::word,
                directive_type_t::dw,
                {
                    {directive_param_t::flags::number | directive_param_t::flags::repeating, true},
                }
            }
        },
        {
            "DD",
            directive_t{
                op_sizes::dword,
                directive_type_t::dd,
                {
                    {directive_param_t::flags::number | directive_param_t::flags::repeating, true},
                }
            }
        },
        {
            "DQ",
            directive_t{
                op_sizes::qword,
                directive_type_t::dq,
                {
                    {directive_param_t::flags::number | directive_param_t::flags::repeating, true},
                }
            }
        },
        {
            "RB",
            directive_t{
                op_sizes::byte,
                directive_type_t::rb,
                {
                    {directive_param_t::flags::number, true},
                }
            }
        },
        {
            "RW",
            directive_t{
                op_sizes::word,
                directive_type_t::rw,
                {
                    {directive_param_t::flags::number, true},
                }
            }
        },
        {
            "RD",
            directive_t{
                op_sizes::dword,
                directive_type_t::rd,
                {
                    {directive_param_t::flags::number, true},
                }
            }
        },
        {
            "RQ",
            directive_t{
                op_sizes::qword,
                directive_type_t::rq,
                {
                    {directive_param_t::flags::number, true},
                }
            }
        },

    };

    inline static directive_t* directive(const std::string& code) {
        const auto it = s_directives.find(code);
        if (it != s_directives.end()) {
            return &it->second;
        }
        return nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////

    class assembler {
    public:
        explicit assembler(vm::terp* terp);

        virtual ~assembler();

        bool assemble(
            common::result& r,
            vm::instruction_block* block = nullptr);

        vm::segment* segment(
            const std::string& name,
            segment_type_t type);

        bool assemble_from_source(
            common::result& r,
            common::source_file& source_file);

        void pop_target_register();

        instruction_block* pop_block();

        instruction_block* root_block();

        vm::assembly_listing& listing();

        bool in_procedure_scope() const;

        segment_list_t segments() const;

        bool initialize(common::result& r);

        instruction_block* current_block();

        bool allocate_reg(register_t& reg);

        void free_reg(const register_t& reg);

        register_t* current_target_register();

        bool resolve_labels(common::result& r);

        bool apply_addresses(common::result& r);

        void push_block(instruction_block* block);

        vm::segment* segment(const std::string& name);

        void push_target_register(const register_t& reg);

        instruction_block* make_basic_block(instruction_block* parent_block = nullptr);

        instruction_block* make_procedure_block(instruction_block* parent_block = nullptr);

    private:
        void add_new_block(instruction_block* block);

        block_entry_t* current_entry(instruction_block* block);

    private:
        vm::terp* _terp = nullptr;
        uint64_t _location_counter = 0;
        vm::assembly_listing _listing {};
        uint32_t _procedure_block_count = 0;
        std::vector<instruction_block*> _blocks {};
        register_allocator_t _register_allocator {};
        std::stack<register_t> _target_registers {};
        std::stack<instruction_block*> _block_stack {};
        std::unordered_map<std::string, vm::segment> _segments {};
    };

};

