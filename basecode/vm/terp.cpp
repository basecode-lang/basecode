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

#include <sstream>
#include <climits>
#include <iomanip>
#include <fmt/format.h>
#include <common/bytes.h>
#include <common/hex_formatter.h>
#include "ffi.h"
#include "terp.h"
#include "basic_block.h"

#define NEXT(next_vpc) {                    \
        lhs.qw = rhs.qw = result.qw = 0;    \
        _vpc = next_vpc;                    \
        pc.qw = _vpc->address;              \
        if (single_step) return true;       \
        else goto *_vpc->handler;           \
    }
#define RESUME(next_vpc) {                  \
        lhs.qw = rhs.qw = result.qw = 0;    \
        _vpc = next_vpc;                    \
        pc.qw = _vpc->address;              \
        goto *_vpc->handler;                \
    }
#define OPERAND(n, t) {             \
        auto& v = _vpc->values[n];  \
        if (v.is_register) {        \
            t = *v.data.r;          \
        } else {                    \
            t = v.data.d;           \
        }                           \
    }
#define OPERAND_WITH_OFFSET(a, o) {             \
        OPERAND(a, lhs)                         \
        OPERAND(o, rhs)                         \
        if (rhs.qw > 0) {                       \
            if (_vpc->values[o].is_negative) {  \
                lhs.qw -= rhs.qw;               \
            } else {                            \
                lhs.qw += rhs.qw;               \
            }                                   \
            rhs.qw = 0;                         \
        }                                       \
    }
#define TARGET(n, s) {                      \
        auto t = _vpc->values[n].data.r;    \
        switch (_vpc->size) {               \
            case op_sizes::byte: {          \
                t->b = s.b;                 \
                break;                      \
            }                               \
            case op_sizes::word: {          \
                t->w = s.w;                 \
                break;                      \
            }                               \
            case op_sizes::dword: {         \
                t->dw = s.dw;               \
                break;                      \
            }                               \
            default:                        \
            case op_sizes::qword: {         \
                t->qw = s.qw;               \
                break;                      \
            }                               \
        }                                   \
    }

namespace basecode::vm {

    std::string instruction_t::disassemble() const {
        std::stringstream stream;

        auto op_name = op_code_name(op);
        if (!op_name.empty()) {
            std::stringstream mnemonic;

            mnemonic << op_name;

            switch (size) {
                case op_sizes::byte:
                    mnemonic << ".B";
                    break;
                case op_sizes::word:
                    mnemonic << ".W";
                    break;
                case op_sizes::dword:
                    mnemonic << ".DW";
                    break;
                case op_sizes::qword:
                    mnemonic << ".QW";
                    break;
                default: {
                    break;
                }
            }

            stream << std::left << std::setw(10) << mnemonic.str();

            register_value_alias_t alias {};
            std::stringstream operands_stream;

            for (size_t i = 0; i < operands_count; i++) {
                if (i > 0 && i < operands_count) {
                    operands_stream << ", ";
                }

                const auto& operand = operands[i];
                alias.qw = operand.value.u;

                if (operand.is_reg()) {
                    if (operand.is_range()) {
                        auto range = static_cast<uint16_t>(operand.value.u);
                        auto start = (range >> 8) & 0xff;
                        auto end = range & 0xff;
                        if (operand.is_integer()) {
                            operands_stream << fmt::format("I{}-I{}", start, end);
                        } else {
                            operands_stream << fmt::format("F{}-F{}", start, end);
                        }
                    } else {
                        if (operand.is_integer()) {
                            switch (operand.value.r) {
                                case registers_t::sp: {
                                    operands_stream << "SP";
                                    break;
                                }
                                case registers_t::fp: {
                                    operands_stream << "FP";
                                    break;
                                }
                                case registers_t::pc: {
                                    operands_stream << "PC";
                                    break;
                                }
                                case registers_t::fr: {
                                    operands_stream << "FR";
                                    break;
                                }
                                case registers_t::sr: {
                                    operands_stream << "SR";
                                    break;
                                }
                                default: {
                                    if (operand.fixup[0].named_ref != nullptr) {
                                        operands_stream << fmt::format(
                                            "{}{{I{}}}",
                                            operand.fixup[0].named_ref->name,
                                            operand.value.r);
                                    } else {
                                        operands_stream << fmt::format("I{}", operand.value.r);
                                    }
                                    break;
                                }
                            }
                        } else {
                            if (operand.fixup[0].named_ref != nullptr) {
                                operands_stream << fmt::format(
                                    "{}{{F{}}}",
                                    operand.fixup[0].named_ref->name,
                                    operand.value.r);
                            } else {
                                operands_stream << fmt::format("F{}", operand.value.r);
                            }
                        }
                    }
                } else {
                    auto has_name = operand.fixup[0].named_ref != nullptr;
                    if (has_name) {
                        operands_stream << operand.fixup[0].named_ref->name << "{";
                    }

                    if (operand.is_negative())
                        operands_stream << "-";

                    auto operand_format_spec = op_size_format_spec(operand.size);
                    switch (operand.size) {
                        case op_sizes::byte:
                            operands_stream << fmt::format(operand_format_spec, alias.b);
                            break;
                        case op_sizes::word:
                            operands_stream << fmt::format(operand_format_spec, alias.w);
                            break;
                        case op_sizes::dword:
                            operands_stream << fmt::format(operand_format_spec, alias.dw);
                            break;
                        case op_sizes::qword:
                            operands_stream << fmt::format(operand_format_spec, alias.qw);
                            break;
                        default: {
                            break;
                        }
                    }

                    if (has_name) {
                        operands_stream << "}";
                    }
                }
            }

            stream << std::left << std::setw(24) << operands_stream.str();
        } else {
            stream << "UNKNOWN";
        }

        return stream.str();
    }

    size_t instruction_t::encoding_size() const {
        size_t encoding_size = base_size;

        for (size_t i = 0; i < operands_count; i++) {
            const auto& operand = operands[i];
            encoding_size += 1;

            if ((operand.is_reg())) {
                if (operand.is_range()) {
                    encoding_size += sizeof(uint16_t);
                } else {
                    encoding_size += sizeof(uint8_t);
                }
            } else {
                auto working_size = operand.size;
                if (operand.fixup[0].named_ref != nullptr) {
                    working_size = operand.fixup[0].named_ref->size;
                }
                switch (working_size) {
                    case op_sizes::none:
                        break;
                    case op_sizes::byte:
                        encoding_size += sizeof(uint8_t);
                        break;
                    case op_sizes::word:
                        encoding_size += sizeof(uint16_t);
                        break;
                    case op_sizes::dword:
                        if (operands[i].is_integer())
                            encoding_size += sizeof(uint32_t);
                        else
                            encoding_size += sizeof(float);
                        break;
                    case op_sizes::qword:
                        if (operands[i].is_integer())
                            encoding_size += sizeof(uint64_t);
                        else
                            encoding_size += sizeof(double);
                        break;
                }
            }
        }

        encoding_size = align(encoding_size, alignment);

        return encoding_size;
    }

    size_t instruction_t::align(uint64_t value, size_t size) const {
        auto offset = value % size;
        return offset ? value + (size - offset) : value;
    }

    size_t instruction_t::decode(common::result& r, uint64_t address) {
        if (address % alignment != 0) {
            r.error(
                "B003",
                fmt::format(
                    "instruction alignment violation: alignment = {} bytes, address = ${:016X}",
                    alignment,
                    address));
            return 0;
        }

        auto encoding_ptr = reinterpret_cast<uint8_t*>(address);
        uint8_t encoding_size = *encoding_ptr;

        auto op_code = *(encoding_ptr + 1);
        if (op_code == 0 || op_code > static_cast<uint8_t>(op_codes::exit))
            return 0;

        op = static_cast<op_codes>(op_code);

        auto op_size_and_operands_count = static_cast<uint8_t>(*(encoding_ptr + 2));
        size = static_cast<op_sizes>(common::get_upper_nybble(op_size_and_operands_count));
        operands_count = common::get_lower_nybble(op_size_and_operands_count);

        size_t offset = base_size;
        for (size_t i = 0; i < operands_count; i++) {
            auto& operand = operands[i];

            operand.type = static_cast<operand_encoding_t::flags>(*(encoding_ptr + offset));
            operand.size_from_flags();

            ++offset;

            if (operand.is_reg()) {
                if (operand.is_range()) {
                    auto constant_value_ptr = reinterpret_cast<uint16_t*>(encoding_ptr + offset);
                    operand.value.u = *constant_value_ptr;
                    offset += sizeof(uint16_t);
                } else {
                    operand.value.r = *(encoding_ptr + offset);
                    ++offset;
                }
            } else {
                switch (operand.size) {
                    case op_sizes::byte: {
                        uint8_t* constant_value_ptr = encoding_ptr + offset;
                        operand.value.u = *constant_value_ptr;
                        offset += sizeof(uint8_t);
                        break;
                    }
                    case op_sizes::word: {
                        auto constant_value_ptr = reinterpret_cast<uint16_t*>(encoding_ptr + offset);
                        operand.value.u = *constant_value_ptr;
                        offset += sizeof(uint16_t);
                        break;
                    }
                    case op_sizes::dword: {
                        if (operand.is_integer()) {
                            auto constant_value_ptr = reinterpret_cast<uint32_t*>(encoding_ptr + offset);
                            operand.value.u = *constant_value_ptr;
                            offset += sizeof(uint32_t);
                        } else {
                            auto constant_value_ptr = reinterpret_cast<float*>(encoding_ptr + offset);
                            operand.value.f = *constant_value_ptr;
                            offset += sizeof(float);
                        }
                        break;
                    }
                    case op_sizes::qword: {
                        if (operand.is_integer()) {
                            auto constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                            operand.value.u = *constant_value_ptr;
                            offset += sizeof(uint64_t);
                        } else {
                            auto constant_value_ptr = reinterpret_cast<double*>(encoding_ptr + offset);
                            operand.value.d = *constant_value_ptr;
                            offset += sizeof(double);
                        }
                        break;
                    }
                    case op_sizes::none: {
                        if (operand.is_integer()) {
                            r.error(
                                "B010",
                                "constant integers cannot have a size of 'none'.");
                        } else {
                            r.error(
                                "B010",
                                "constant floats cannot have a size of 'none', 'byte', or 'word'.");
                        }
                        break;
                    }
                }
            }
        }

        return encoding_size;
    }

    size_t instruction_t::encode(common::result& r, uint64_t address) {
        if (address % alignment != 0) {
            r.error(
                "B003",
                fmt::format(
                    "instruction alignment violation: alignment = {} bytes, address = ${:016X}",
                    alignment,
                    address));
            return 0;
        }

        uint8_t encoding_size = base_size;
        size_t offset = base_size;

        auto encoding_ptr = reinterpret_cast<uint8_t*>(address);
        *(encoding_ptr + 1) = static_cast<uint8_t>(op);

        uint8_t size_type_and_operand_count = 0;
        size_type_and_operand_count = common::set_upper_nybble(
            size_type_and_operand_count,
            static_cast<uint8_t>(size));
        size_type_and_operand_count = common::set_lower_nybble(
            size_type_and_operand_count,
            operands_count);
        *(encoding_ptr + 2) = size_type_and_operand_count;

        for (size_t i = 0; i < operands_count; i++) {
            auto& operand = this->operands[i];

            operand.size_to_flags();

            *(encoding_ptr + offset) = static_cast<uint8_t>(operand.type);
            ++offset;
            ++encoding_size;

            if (operand.is_reg()) {
                if (operand.is_range()) {
                    auto constant_value_ptr = reinterpret_cast<uint16_t*>(encoding_ptr + offset);
                    *constant_value_ptr = static_cast<uint16_t>(operand.value.u);
                    offset += sizeof(uint16_t);
                    encoding_size += sizeof(uint16_t);
                } else {
                    *(encoding_ptr + offset) = operand.value.r;
                    ++offset;
                    ++encoding_size;
                }
            } else {
                switch (operand.size) {
                    case op_sizes::byte: {
                        uint8_t* constant_value_ptr = encoding_ptr + offset;
                        *constant_value_ptr = static_cast<uint8_t>(operand.value.u);
                        offset += sizeof(uint8_t);
                        encoding_size += sizeof(uint8_t);
                        break;
                    }
                    case op_sizes::word: {
                        auto constant_value_ptr = reinterpret_cast<uint16_t*>(encoding_ptr + offset);
                        *constant_value_ptr = static_cast<uint16_t>(operand.value.u);
                        offset += sizeof(uint16_t);
                        encoding_size += sizeof(uint16_t);
                        break;
                    }
                    case op_sizes::dword: {
                        if (operands[i].is_integer()) {
                            auto constant_value_ptr = reinterpret_cast<uint32_t*>(encoding_ptr + offset);
                            *constant_value_ptr = static_cast<uint32_t>(operand.value.u);
                            offset += sizeof(uint32_t);
                            encoding_size += sizeof(uint32_t);
                        } else {
                            auto constant_value_ptr = reinterpret_cast<float*>(encoding_ptr + offset);
                            *constant_value_ptr = operand.value.f;
                            offset += sizeof(float);
                            encoding_size += sizeof(float);
                        }
                        break;
                    }
                    case op_sizes::qword: {
                        if (operand.is_integer()) {
                            auto constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                            *constant_value_ptr = operand.value.u;
                            offset += sizeof(uint64_t);
                            encoding_size += sizeof(uint64_t);
                        } else {
                            auto constant_value_ptr = reinterpret_cast<double*>(encoding_ptr + offset);
                            *constant_value_ptr = operand.value.d;
                            offset += sizeof(double);
                            encoding_size += sizeof(double);
                        }
                        break;
                    }
                    case op_sizes::none:
                        if (operand.is_integer()) {
                            r.error(
                                "B009",
                                "constant integers cannot have a size of 'none'.");
                        } else {
                            r.error(
                                "B009",
                                "constant floats cannot have a size of 'none', 'byte', or 'word'.");
                        }
                        break;
                }
            }
        }

        encoding_size = static_cast<uint8_t>(align(encoding_size, alignment));
        *encoding_ptr = encoding_size;

        return encoding_size;
    }

    void instruction_t::patch_branch_address(uint64_t address, uint8_t index) {
        operands[index].value.u = align(address, alignment);
    }

    ///////////////////////////////////////////////////////////////////////////

    terp::terp(
            vm::ffi* ffi,
            vm::allocator* allocator,
            size_t heap_size,
            size_t stack_size) : _ffi(ffi),
                                 _heap_size(heap_size),
                                 _stack_size(stack_size),
                                 _allocator(allocator) {
    }

    terp::~terp() {
        delete _heap;
        _heap = nullptr;
    }

    void terp::reset() {
        heap_vector(heap_vectors_t::bss_start, 0);
        heap_vector(heap_vectors_t::bss_length, 0);
        heap_vector(heap_vectors_t::program_end, 0);

        _allocator->reset();
        _thread.loaded = false;
        _thread.slots.clear();

        restart();
    }

    void terp::restart() {
        _registers.r[register_pc].qw = heap_vector(heap_vectors_t::program_start);
        _registers.r[register_sp].qw = heap_vector(heap_vectors_t::top_of_stack);
        _registers.r[register_fp].qw = 0;
        _registers.r[register_fr].qw = 0;
        _registers.r[register_sr].qw = 0;

        for (size_t i = 0; i < number_general_purpose_registers; i++) {
            _registers.r[i].qw = 0;
        }

        if (_thread.loaded)
            _vpc = &_thread.slots[0];
        else
            _vpc = nullptr;

        while(!_call_stack.empty())
            _call_stack.pop();

        _exited = false;
    }

    uint64_t terp::pop() {
        uint64_t value = read(op_sizes::qword, _registers.r[register_sp].qw);
        _registers.r[register_sp].qw += sizeof(uint64_t);
        return value;
    }

    uint64_t terp::peek() const {
        return read(op_sizes::qword, _registers.r[register_sp].qw);
    }

    void terp::initialize_allocator() {
        auto address = heap_vector(heap_vectors_t::free_space_start);
        auto size = heap_vector(heap_vectors_t::bottom_of_stack) - address;
        _allocator->initialize(address, size);
    }

    void terp::dump_state(uint8_t count) {
        fmt::print("\n-------------------------------------------------------------\n");
        fmt::print(
            "PC =${:08x} | SP =${:08x} | FP =${:08x} | FR =${:08x}\n",
            _registers.r[register_pc].qw,
            _registers.r[register_sp].qw,
            _registers.r[register_fp].qw,
            _registers.r[register_fr].qw);

        fmt::print("-------------------------------------------------------------\n");

        uint8_t index = register_integer_start;
        for (size_t y = 0; y < count; y++) {
            fmt::print(
                "I{:02}=${:08x} | I{:02}=${:08x} | I{:02}=${:08x} | I{:02}=${:08x}\n",
                index,
                _registers.r[index].qw,
                index + 1,
                _registers.r[index + 1].qw,
                index + 2,
                _registers.r[index + 2].qw,
                index + 3,
                _registers.r[index + 3].qw);
            index += 4;
        }

        fmt::print("-------------------------------------------------------------\n");

        index = register_float_start;
        for (size_t y = 0; y < count; y++) {
            fmt::print(
                "F{:02}=${:08x} | F{:02}=${:08x} | F{:02}=${:08x} | F{:02}=${:08x}\n",
                index,
                _registers.r[index].qw,
                index + 1,
                _registers.r[index + 1].qw,
                index + 2,
                _registers.r[index + 2].qw,
                index + 3,
                _registers.r[index + 3].qw);
            index += 4;
        }

        fmt::print("\n");
    }

    bool terp::run(
            common::result& r,
            bool single_step) {
        const void* op_handlers[] = {
            &&_nop,
            &&_nop,
            &&_alloc,
            &&_free,
            &&_size,
            &&_load,
            &&_store,
            &&_copy,
            &&_convert,
            &&_fill,
            &&_clr,
            &&_move,
            &&_moves,
            &&_movez,
            &&_push,
            &&_pushm,
            &&_pop,
            &&_popm,
            &&_dup,
            &&_inc,
            &&_dec,
            &&_add,
            &&_sub,
            &&_mul,
            &&_div,
            &&_mod,
            &&_neg,
            &&_shr,
            &&_shl,
            &&_ror,
            &&_rol,
            &&_pow,
            &&_and,
            &&_or,
            &&_xor,
            &&_not,
            &&_bis,
            &&_bic,
            &&_test,
            &&_cmp,
            &&_bz,
            &&_bnz,
            &&_tbz,
            &&_tbnz,
            &&_bne,
            &&_beq,
            &&_bs,
            &&_bo,
            &&_bcc,
            &&_bcs,
            &&_ba,
            &&_bae,
            &&_bb,
            &&_bbe,
            &&_bg,
            &&_bl,
            &&_bge,
            &&_ble,
            &&_seta,
            &&_setna,
            &&_setae,
            &&_setnae,
            &&_setb,
            &&_setnb,
            &&_setbe,
            &&_setnbe,
            &&_setc,
            &&_setnc,
            &&_setg,
            &&_setng,
            &&_setge,
            &&_setnge,
            &&_setl,
            &&_setnl,
            &&_setle,
            &&_setnle,
            &&_sets,
            &&_setns,
            &&_seto,
            &&_setno,
            &&_setz,
            &&_setnz,
            &&_jsr,
            &&_rts,
            &&_jmp,
            &&_swi,
            &&_swap,
            &&_trap,
            &&_ffi,
            &&_meta,
            &&_exit
        };

        if (!_thread.loaded) {
            instruction_t temp_inst;
            boost::unordered_map<uint64_t, ssize_t> address_map{};

            auto address = heap_vector(heap_vectors_t::program_start);
            auto program_end_address = heap_vector(heap_vectors_t::program_end);

            while (address < program_end_address) {
                address_map.insert(std::make_pair(
                    address,
                    _thread.slots.size()));

                auto size = temp_inst.decode(r, address);
                if (size == 0)
                    return false;

                //fmt::print("${:016X}: {}\n", address, temp_inst.disassemble());

                dtt_slot_t slot{};
                slot.address = address;
                slot.encoding_size = size;
                slot.size = temp_inst.size;
                slot.op_code = temp_inst.op;
                slot.branch_target = nullptr;
                slot.values_count = temp_inst.operands_count;
                slot.handler = op_handlers[static_cast<uint8_t>(temp_inst.op)];

                address += size;

                for (auto i = 0; i < temp_inst.operands_count; i++) {
                    auto& value = slot.values[i];
                    const auto& operand = temp_inst.operands[i];

                    if (operand.is_reg()) {
                        const auto type = operand.is_integer() ?
                                          register_type_t::integer :
                                          register_type_t::floating_point;
                        auto index = register_index(
                            static_cast<registers_t>(operand.value.r),
                            type);
                        value.is_register = true;
                        if (operand.is_range())
                            value.data.d.w = static_cast<uint16_t>(operand.value.u);
                        else
                            value.data.r = &_registers.r[index];
                    } else {
                        value.is_register = false;
                        value.data.d.qw = operand.value.u;
                    }

                    value.size = operand.size;
                    value.is_range = operand.is_range();
                    value.is_integer = operand.is_integer();
                    value.is_negative = operand.is_negative();
                }

                _thread.slots.emplace_back(slot);
            }

            auto index = 0;
            for (auto& slot : _thread.slots) {
                address = 0;

                switch (slot.op_code) {
                    case op_codes::bne:
                    case op_codes::beq:
                    case op_codes::bs:
                    case op_codes::bo:
                    case op_codes::bcc:
                    case op_codes::bcs:
                    case op_codes::ba:
                    case op_codes::bae:
                    case op_codes::bb:
                    case op_codes::bbe:
                    case op_codes::bg:
                    case op_codes::bl:
                    case op_codes::bge:
                    case op_codes::ble:
                    case op_codes::jmp:
                    case op_codes::jsr: {
                        address = slot.values[0].data.d.qw;
                        break;
                    }
                    case op_codes::bz:
                    case op_codes::bnz: {
                        address = slot.values[1].data.d.qw;
                        break;
                    }
                    case op_codes::tbz:
                    case op_codes::tbnz: {
                        address = slot.values[2].data.d.qw;
                        break;
                    }
                    default:
                        break;
                }

                if (address != 0) {
                    auto it = address_map.find(address);
                    if (it != std::end(address_map)) {
                        slot.branch_target = &_thread.slots[it->second];
                    }
                }

                index++;
            }

            _thread.loaded = true;
            _vpc = &_thread.slots[0];
        }

        op_sizes size;
        register_value_alias_t lhs {};
        register_value_alias_t rhs {};
        register_value_alias_t result {};
        auto& pc = _registers.r[register_pc];

        // N.B. this starts execution
        RESUME(_vpc)
        return false;

        _nop:
        {
            NEXT(++_vpc)
        }

        _alloc:
        {
            OPERAND(1, rhs)
            rhs.qw *= op_size_in_bytes(_vpc->size);
            lhs.qw = _allocator->alloc(rhs.qw);
            if (lhs.qw == 0) {
                execute_trap(trap_out_of_memory);
                return false;
            }
            TARGET(0, lhs)
            _registers.set_flags(
                lhs.qw == 0,
                false,
                false,
                is_negative(lhs));
            NEXT(++_vpc)
        }

        _free:
        {
            OPERAND(0, rhs)
            auto freed_size = _allocator->free(rhs.qw);
            _registers.set_flags(
                freed_size == 0,
                false,
                false,
                is_negative(rhs));
            NEXT(++_vpc)
        }

        _size:
        {
            OPERAND(1, rhs)
            lhs.qw = _allocator->size(rhs.qw);
            TARGET(0, lhs)
            _registers.set_flags(
                lhs.qw == 0,
                false,
                false,
                is_negative(lhs));
            NEXT(++_vpc)
        }

        _load:
        {
            OPERAND_WITH_OFFSET(1, 2)
            lhs.qw = read(_vpc->size, lhs.qw);
            TARGET(0, lhs)
            _registers.set_flags(
                is_zero(lhs),
                false,
                false,
                is_negative(lhs));
            NEXT(++_vpc)
        }

        _store:
        {
            OPERAND_WITH_OFFSET(0, 2)
            OPERAND(1, rhs)
            write(_vpc->size, lhs.qw, rhs.qw);
            _registers.set_flags(
                is_zero(rhs),
                false,
                false,
                is_negative(rhs));
            NEXT(++_vpc)
        }

        _copy:
        {
            OPERAND(0, lhs)
            OPERAND(1, rhs)
            OPERAND(2, result)
            memcpy(
                reinterpret_cast<void*>(lhs.qw),
                reinterpret_cast<void*>(rhs.qw),
                result.qw * op_size_in_bytes(_vpc->size));
            _registers.set_flags(false, false, false, false);
            NEXT(++_vpc)
        }

        _convert:
        {
            OPERAND(1, rhs)

            // XXX: how to handle NaN & Inf for integers
            if (_vpc->values[0].is_integer) {
                if (!_vpc->values[1].is_integer) {
                    switch (_vpc->size) {
                        case op_sizes::dword:
                            result.dw = static_cast<uint64_t>(rhs.dwf);
                            break;
                        case op_sizes::qword:
                            result.qw = static_cast<uint64_t>(rhs.qwf);
                            break;
                        default:
                            // XXX: this is an error
                            break;
                    }
                }
            } else {
                if (_vpc->values[1].is_integer) {
                    switch (_vpc->size) {
                        case op_sizes::dword:
                            result.dwf = static_cast<float>(rhs.dw);
                            break;
                        case op_sizes::qword:
                            result.qwf = static_cast<double>(rhs.qw);
                            break;
                        default:
                            // XXX: this is an error
                            break;
                    }
                } else {
                    switch (_vpc->size) {
                        case op_sizes::dword:
                            result.dwf = static_cast<float>(rhs.qwf);
                            break;
                        case op_sizes::qword:
                            result.qwf = rhs.dwf;
                            break;
                        default:
                            // XXX: this is an error
                            break;
                    }

                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _fill:
        {
            OPERAND(0, lhs)
            OPERAND(1, result)
            OPERAND(2, rhs)
            rhs.qw *= op_size_in_bytes(_vpc->size);

            switch (_vpc->size) {
                case op_sizes::byte:
                    memset(
                        reinterpret_cast<void*>(lhs.qw),
                        result.b,
                        rhs.qw);
                    break;
                case op_sizes::word:
                    memset(
                        reinterpret_cast<void*>(lhs.qw),
                        result.w,
                        rhs.qw);
                    break;
                case op_sizes::dword:
                    memset(
                        reinterpret_cast<void*>(lhs.qw),
                        result.dw,
                        rhs.qw);
                    break;
                case op_sizes::qword:
                default:
                    // XXX: this is an error
                    break;
            }

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                false);

            NEXT(++_vpc)
        }

        _clr:
        {
            TARGET(0, result)
            _registers.set_flags(true, false, false, false);
            NEXT(++_vpc)
        }

        _move:
        {
            OPERAND_WITH_OFFSET(1, 2)
            TARGET(0, lhs)
            _registers.set_flags(
                is_zero(lhs),
                false,
                false,
                is_negative(lhs));
            NEXT(++_vpc)
        }

        _moves:
        {
            OPERAND_WITH_OFFSET(1, 2)

            auto previous_size = static_cast<op_sizes>(static_cast<uint8_t>(_vpc->size) - 1);
            auto new_size = static_cast<uint32_t>(op_size_in_bytes(previous_size) * 8);
            lhs.qw = common::sign_extend(lhs.qw, new_size);

            TARGET(0, lhs)

            _registers.set_flags(
                is_zero(lhs),
                false,
                false,
                is_negative(lhs));

            NEXT(++_vpc)
        }

        _movez:
        {
            OPERAND_WITH_OFFSET(1, 2)

            switch (_vpc->size) {
                case op_sizes::none:
                case op_sizes::byte: {
                    break;
                }
                case op_sizes::word: {
                    lhs.w &= 0b0000000011111111;
                    break;
                }
                case op_sizes::dword: {
                    lhs.dw &= 0b00000000000000001111111111111111;
                    break;
                }
                case op_sizes::qword: {
                    lhs.qw &= 0b0000000000000000000000000000000011111111111111111111111111111111;
                    break;
                }
            }

            TARGET(0, lhs)

            _registers.set_flags(
                is_zero(lhs),
                false,
                false,
                is_negative(lhs));

            NEXT(++_vpc)
        }

        _push:
        {
            OPERAND(0, lhs)
            push(lhs.qw);
            _registers.set_flags(
                is_zero(lhs),
                false,
                false,
                is_negative(lhs));
            NEXT(++_vpc)
        }

        _pushm:
        {
            register_value_alias_t* alias = nullptr;

            for (uint8_t i = 0; i < _vpc->values_count; i++) {
                const auto& value = _vpc->values[i];
                auto type = value.is_integer ?
                            register_type_t::integer :
                            register_type_t::floating_point;

                if (value.is_range) {
                    auto range = value.data.d.w;
                    auto start = (range >> 8) & 0xff;
                    auto end = range & 0xff;
                    auto delta = end < start ? -1 : 1;
                    auto count = std::abs(end - start);

                    for (auto reg = static_cast<uint8_t>(start);
                         count >= 0;
                         reg += delta, --count) {
                        auto reg_index = register_index(
                            static_cast<registers_t>(reg),
                            type);
                        alias = &_registers.r[reg_index];
                        push(alias->qw);
                    }
                } else if (value.is_register) {
                    alias = value.data.r;
                    push(alias->qw);
                }
            }

            _registers.set_flags(
                alias->qw == 0,
                false,
                false,
                is_negative(*alias));

            NEXT(++_vpc)
        }

        _pop:
        {
            result.qw = pop();
            TARGET(0, result)
            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));
            NEXT(++_vpc)
        }

        _popm:
        {
            register_value_alias_t* alias = nullptr;

            for (uint8_t i = 0; i < _vpc->values_count; i++) {
                const auto& value = _vpc->values[i];
                const auto type = value.is_integer ?
                            register_type_t::integer :
                            register_type_t::floating_point;

                if (value.is_range) {
                    auto range = value.data.d.w;
                    auto start = (range >> 8) & 0xff;
                    auto end = range & 0xff;
                    auto delta = end < start ? -1 : 1;
                    auto count = std::abs(end - start);

                    for (auto reg = static_cast<uint8_t>(start);
                         count >= 0;
                         reg += delta, --count) {
                        auto reg_index = register_index(
                            static_cast<registers_t>(reg),
                            type);
                        alias = &_registers.r[reg_index];
                        alias->qw = pop();
                    }
                } else if (value.is_register) {
                    alias = value.data.r;
                    alias->qw = pop();
                }
            }

            _registers.set_flags(
                alias->qw == 0,
                false,
                false,
                is_negative(*alias));

            NEXT(++_vpc)
        }

        _dup:
        {
            result.qw = peek();
            push(result.qw);

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _inc:
        {
            OPERAND(0, lhs)
            rhs.qw = 1;

            if (_vpc->values[0].is_integer) {
                switch (_vpc->size) {
                    case op_sizes::byte: {
                        result.b = lhs.b + rhs.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.w = lhs.w + rhs.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.dw = lhs.dw + rhs.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.qw = lhs.qw + rhs.qw;
                        break;
                    }
                }
            } else {
                if (_vpc->size == op_sizes::dword)
                    result.dwf = lhs.dwf + 1.0F;
                else
                    result.qwf = lhs.qwf + 1.0;
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                has_carry(lhs.qw, rhs.qw),
                has_overflow(lhs, rhs, result),
                is_negative(result));

            NEXT(++_vpc)
        }

        _dec:
        {
            OPERAND(0, lhs)
            rhs.qw = 1;

            if (_vpc->values[0].is_integer) {
                switch (_vpc->size) {
                    case op_sizes::byte: {
                        result.b = lhs.b - rhs.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.w = lhs.w - rhs.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.dw = lhs.dw - rhs.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.qw = lhs.qw - rhs.qw;
                        break;
                    }
                }
            } else {
                if (_vpc->size == op_sizes::dword)
                    result.dwf = lhs.dwf - 1.0F;
                else
                    result.qwf = lhs.qwf - 1.0;
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                has_carry(lhs.qw, rhs.qw),
                has_overflow(lhs, rhs, result),
                is_negative(result),
                true);

            NEXT(++_vpc)
        }

        _add:
        {
            size = _vpc->size;

            const auto& value1 = _vpc->values[1];
            const auto& value2 = _vpc->values[2];

            OPERAND(1, lhs)
            OPERAND(2, rhs)

            if (value1.is_integer
	        &&  value2.is_integer) {
                switch (size) {
                    case op_sizes::byte: {
                        result.b = lhs.b + rhs.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.w = lhs.w + rhs.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.dw = lhs.dw + rhs.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.qw = lhs.qw + rhs.qw;
                        break;
                    }
                }
            } else {
                if (size == op_sizes::dword)
                    result.dwf = lhs.dwf + rhs.dwf;
                else
                    result.qwf = lhs.qwf + rhs.qwf;
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                has_carry(lhs.qw, rhs.qw),
                has_overflow(lhs, rhs, result),
                is_negative(result));

            NEXT(++_vpc)
        }

        _sub:
        {
            size = _vpc->size;

            const auto& value1 = _vpc->values[1];
            const auto& value2 = _vpc->values[2];

            OPERAND(1, lhs)
            OPERAND(2, rhs)

            auto carry_flag = false;

            if (value1.is_integer
            &&  value2.is_integer) {
                switch (size) {
                    case op_sizes::byte:
                        carry_flag = lhs.b < rhs.b;
                        result.b = lhs.b - rhs.b;
                        break;
                    case op_sizes::word:
                        carry_flag = lhs.w < rhs.w;
                        result.w = lhs.w - rhs.w;
                        break;
                    case op_sizes::dword:
                        carry_flag = lhs.dw < rhs.dw;
                        result.dw = lhs.dw - rhs.dw;
                        break;
                    case op_sizes::qword:
                        carry_flag = lhs.qw < rhs.qw;
                        result.qw = lhs.qw - rhs.qw;
                        break;
                    default:
                        return false;
                }
            } else {
                if (size == op_sizes::dword)
                    result.dwf = lhs.dwf - rhs.dwf;
                else
                    result.qwf = lhs.qwf - rhs.qwf;
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                carry_flag,
                !carry_flag && has_overflow(lhs, rhs, result),
                is_negative(result),
                true);

            NEXT(++_vpc)
        }

        _mul:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            if (_vpc->values[1].is_integer
            &&  _vpc->values[2].is_integer) {
                switch (_vpc->size) {
                    case op_sizes::byte: {
                        result.b = lhs.b * rhs.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.w = lhs.w * rhs.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.dw = lhs.dw * rhs.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.qw = lhs.qw * rhs.qw;
                        break;
                    }
                }
            } else {
                if (_vpc->size == op_sizes::dword)
                    result.dwf = lhs.dwf * rhs.dwf;
                else
                    result.qwf = lhs.qwf * rhs.qwf;
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                has_carry(lhs.qw, rhs.qw),
                has_overflow(lhs, rhs, result),
                is_negative(result));

            NEXT(++_vpc)
        }

        _div:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            if (_vpc->values[1].is_integer
            &&  _vpc->values[2].is_integer) {
                switch (_vpc->size) {
                    case op_sizes::byte: {
                        if (rhs.b != 0)
                            result.b = lhs.b / rhs.b;
                        break;
                    }
                    case op_sizes::word: {
                        if (rhs.w != 0)
                            result.w = lhs.w / rhs.w;
                        break;
                    }
                    case op_sizes::dword: {
                        if (rhs.dw != 0)
                            result.dw = lhs.dw / rhs.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        if (rhs.qw != 0)
                            result.qw = lhs.qw / rhs.qw;
                        break;
                    }
                }
            } else {
                if (_vpc->size == op_sizes::dword) {
                    if (rhs.dwf != 0) {
                        result.dwf = lhs.dwf / rhs.dwf;
                    }
                } else {
                    if (rhs.qwf != 0) {
                        result.qwf = lhs.qwf / rhs.qwf;
                    }
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                has_carry(lhs.qw, rhs.qw),
                has_overflow(lhs, rhs, result),
                is_negative(result));

            NEXT(++_vpc)
        }

        _mod:
        {
            size = _vpc->size;

            OPERAND(1, lhs)
            OPERAND(2, rhs)

            switch (size) {
                case op_sizes::byte: {
                    if (lhs.b != 0 && rhs.b != 0)
                        result.b = lhs.b % rhs.b;
                    break;
                }
                case op_sizes::word: {
                    if (lhs.w != 0 && rhs.w != 0)
                        result.w = lhs.w % rhs.w;
                    break;
                }
                case op_sizes::dword: {
                    if (lhs.dw != 0 && rhs.dw != 0)
                        result.dw = lhs.dw % rhs.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    if (lhs.qw != 0 && rhs.qw != 0)
                        result.qw = lhs.qw % rhs.qw;
                    break;
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _neg:
        {
            OPERAND(1, lhs)

            if (_vpc->values[1].is_integer) {
                switch (_vpc->size) {
                    case op_sizes::byte: {
                        int8_t negated_result = -lhs.b;
                        result.b = static_cast<uint64_t>(negated_result);
                        break;
                    }
                    case op_sizes::word: {
                        int16_t negated_result = -lhs.w;
                        result.w = static_cast<uint64_t>(negated_result);
                        break;
                    }
                    case op_sizes::dword: {
                        int32_t negated_result = -lhs.dw;
                        result.dw = static_cast<uint64_t>(negated_result);
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        int64_t negated_result = -lhs.qw;
                        result.qw = static_cast<uint64_t>(negated_result);
                        break;
                    }
                }
            } else {
                if (_vpc->size == op_sizes::dword)
                    result.dwf = lhs.dwf * -1.0F;
                else
                    result.qwf = lhs.qwf * -1.0;
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _shr:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    result.b = lhs.b >> rhs.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs.w >> rhs.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs.dw >> rhs.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs.qw >> rhs.qw;
                    break;
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _shl:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    result.b = lhs.b << rhs.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs.w << rhs.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs.dw << rhs.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs.qw << rhs.qw;
                    break;
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _ror:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)
            result.qw = common::rotr(lhs.qw, rhs.b);
            TARGET(0, result)
            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));
            NEXT(++_vpc)
        }

        _rol:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)
            result.qw = common::rotl(lhs.qw, rhs.b);
            TARGET(0, result)
            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));
            NEXT(++_vpc)
        }

        _pow:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            if (_vpc->values[1].is_integer
            &&  _vpc->values[2].is_integer) {
                switch (_vpc->size) {
                    case op_sizes::byte: {
                        result.b = common::power(lhs.b, rhs.b);
                        break;
                    }
                    case op_sizes::word: {
                        result.w = common::power(lhs.w, rhs.w);
                        break;
                    }
                    case op_sizes::dword: {
                        result.dw = common::power(lhs.dw, rhs.dw);
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.qw = common::power(lhs.qw, rhs.qw);
                        break;
                    }
                }
            } else {
                if (_vpc->size == op_sizes::dword) {
                    result.dwf = std::pow(lhs.dwf, rhs.dwf);
                } else {
                    result.qwf = std::pow(lhs.qwf, rhs.qwf);
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _and:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    result.b = lhs.b & rhs.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs.w & rhs.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs.dw & rhs.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs.qw & rhs.qw;
                    break;
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _or:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    result.b = lhs.b | rhs.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs.w | rhs.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs.dw | rhs.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs.qw | rhs.qw;
                    break;
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _xor:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    result.b = lhs.b ^ rhs.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs.w ^ rhs.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs.dw ^ rhs.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs.qw ^ rhs.qw;
                    break;
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _not:
        {
            OPERAND(1, result)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    result.b = ~result.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = ~result.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = ~result.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = ~result.qw;
                    break;
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _bis:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    auto masked_value = static_cast<uint8_t>(1) << rhs.b;
                    result.b = lhs.b | masked_value;
                    break;
                }
                case op_sizes::word: {
                    auto masked_value = static_cast<uint16_t>(1) << rhs.b;
                    result.w = lhs.w | masked_value;
                    break;
                }
                case op_sizes::dword: {
                    auto masked_value = static_cast<uint32_t>(1) << rhs.b;
                    result.dw = lhs.dw | masked_value;
                    break;
                }
                default:
                case op_sizes::qword: {
                    auto masked_value = static_cast<uint64_t>(1) << rhs.b;
                    result.qw = lhs.qw | masked_value;
                    break;
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _bic:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    auto masked_value = ~(static_cast<uint8_t>(1) << rhs.b);
                    result.b = lhs.b & masked_value;
                    break;
                }
                case op_sizes::word: {
                    auto masked_value = ~(static_cast<uint16_t>(1) << rhs.b);
                    result.w = lhs.w & masked_value;
                    break;
                }
                case op_sizes::dword: {
                    auto masked_value = ~(static_cast<uint32_t>(1) << rhs.b);
                    result.dw = lhs.dw & masked_value;
                    break;
                }
                default:
                case op_sizes::qword: {
                    auto masked_value = ~(static_cast<uint64_t>(1) << rhs.b);
                    result.qw = lhs.qw & masked_value;
                    break;
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _test:
        {
            OPERAND(1, lhs)
            OPERAND(2, rhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    result.b = lhs.b & rhs.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs.w & rhs.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs.dw & rhs.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs.qw & rhs.qw;
                    break;
                }
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _cmp:
        {
            size = _vpc->size;

            OPERAND(0, lhs)
            OPERAND(1, rhs)

            bool carry_flag = false;

            switch (size) {
                case op_sizes::byte:
                    carry_flag = lhs.b < rhs.b;
                    result.b = lhs.b - rhs.b;
                    break;
                case op_sizes::word:
                    carry_flag = lhs.w < rhs.w;
                    result.w = lhs.w - rhs.w;
                    break;
                case op_sizes::dword:
                    carry_flag = lhs.dw < rhs.dw;
                    result.dw = lhs.dw - rhs.dw;
                    break;
                case op_sizes::qword:
                    carry_flag = lhs.qw < rhs.qw;
                    result.qw = lhs.qw - rhs.qw;
                    break;
                default:
                    return false;
            }

            _registers.set_flags(
                is_zero(result),
                carry_flag,
                !carry_flag && has_overflow(lhs, rhs, result),
                is_negative(result),
                true);

            NEXT(++_vpc)
        }

        _bz:
        {
            OPERAND(0, lhs)

            auto zero_flag = is_zero(lhs);
            _registers.set_flags(
                zero_flag,
                false,
                false,
                is_negative(lhs));

            if (zero_flag) {
                OPERAND(1, pc)
                NEXT(_vpc->branch_target)
            }

            NEXT(++_vpc)
        }

        _bnz:
        {
            OPERAND(0, lhs)

            auto zero_flag = is_zero(lhs);

            _registers.set_flags(
                zero_flag,
                false,
                false,
                is_negative(lhs));

            if (!zero_flag) {
                OPERAND(1, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _tbz:
        {
            OPERAND(0, lhs)
            OPERAND(1, rhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    result.b = lhs.b & rhs.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs.w & rhs.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs.dw & rhs.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs.qw & rhs.qw;
                    break;
                }
            }

            auto zero_flag = is_zero(result);
            _registers.set_flags(
                zero_flag,
                false,
                false,
                is_negative(result));

            if (zero_flag) {
                OPERAND(2, pc)
                NEXT(_vpc->branch_target)
            }

            NEXT(++_vpc)
        }

        _tbnz:
        {
            OPERAND(0, lhs)
            OPERAND(1, rhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    result.b = lhs.b & rhs.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs.w & rhs.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs.dw & rhs.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs.qw & rhs.qw;
                    break;
                }
            }

            auto zero_flag = is_zero(result);
            _registers.set_flags(
                zero_flag,
                false,
                false,
                is_negative(result));

            if (!zero_flag) {
                OPERAND(2, pc)
                NEXT(_vpc->branch_target)
            }

            NEXT(++_vpc)
        }

        _bne:
        {
            // ZF = 0
            if (!_registers.flags(register_file_t::flags_t::zero)) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _beq:
        {
            // ZF = 1
            if (_registers.flags(register_file_t::flags_t::zero)) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _bs:
        {
            // SF = 1
            if (_registers.flags(register_file_t::flags_t::negative)) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _bo:
        {
            // OF = 1
            if (_registers.flags(register_file_t::flags_t::overflow)) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _bcc:
        {
            // CF = 0
            if (!_registers.flags(register_file_t::flags_t::carry)) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _bb:
        _bae:
        _bcs:
        {
            // CF = 1
            if (_registers.flags(register_file_t::flags_t::carry)) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _ba:
        {
            // CF = 0 and ZF = 0
            if (!_registers.flags(register_file_t::flags_t::zero)
            &&  !_registers.flags(register_file_t::flags_t::carry)) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _bbe:
        {
            // CF = 1 or ZF = 1
            if (_registers.flags(register_file_t::flags_t::carry)
            ||  _registers.flags(register_file_t::flags_t::zero)) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _bg:
        {
            // ZF = 0 and SF = OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            auto zf = _registers.flags(register_file_t::flags_t::zero);

            if (!zf && sf == of) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _bl:
        {
            // SF <> OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);

            if (sf != of) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _bge:
        {
            // SF = OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);

            if (sf == of) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _ble:
        {
            // ZF = 1 or SF <> OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            auto zf = _registers.flags(register_file_t::flags_t::zero);

            if (zf || sf != of) {
                OPERAND(0, pc)
                NEXT(_vpc->branch_target);
            }

            NEXT(++_vpc)
        }

        _seta:
        _setnbe:
        {
            // CF = 0 and ZF = 0
            auto zero_flag = _registers.flags(register_file_t::flags_t::zero);
            auto carry_flag = _registers.flags(register_file_t::flags_t::carry);
            result.b = !zero_flag && !carry_flag ? 1 : 0;
            TARGET(0, result)

            NEXT(++_vpc)
        }

        _setna:
        _setbe:
        {
            // CF = 1 or ZF = 1
            auto zero_flag = _registers.flags(register_file_t::flags_t::zero);
            auto carry_flag = _registers.flags(register_file_t::flags_t::carry);
            result.b = (zero_flag || carry_flag) ? 1 : 0;
            TARGET(0, result)

            NEXT(++_vpc)
        }

        _setb:
        _setc:
        _setnae:
        {
            // CF = 1
            result.b = _registers.flags(register_file_t::flags_t::carry) ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _setnb:
        _setae:
        _setnc:
        {
            // CF = 0
            result.b = !_registers.flags(register_file_t::flags_t::carry) ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _setg:
        _setnle:
        {
            // ZF = 0 and SF = OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            auto zf = _registers.flags(register_file_t::flags_t::zero);
            result.b = !zf && sf == of ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _setl:
        _setnge:
        {
            // SF != OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            result.b = sf != of ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _setnl:
        _setge:
        {
            // SF = OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            result.b = sf == of ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _setle:
        _setng:
        {
            // ZF = 1 or SF != OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            auto zf = _registers.flags(register_file_t::flags_t::zero);
            result.b = zf || sf != of ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _sets:
        {
            // SF = 1
            result.b = _registers.flags(register_file_t::flags_t::negative) ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _setns:
        {
            // SF = 0
            result.b = !_registers.flags(register_file_t::flags_t::negative) ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _seto:
        {
            // OF = 1
            result.b = _registers.flags(register_file_t::flags_t::overflow) ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _setno:
        {
            // OF = 0
            result.b = !_registers.flags(register_file_t::flags_t::overflow) ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _setz:
        {
            result.b = _registers.flags(register_file_t::flags_t::zero) ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _setnz:
        {
            result.b = !_registers.flags(register_file_t::flags_t::zero) ? 1 : 0;
            TARGET(0, result)
            NEXT(++_vpc)
        }

        _jsr:
        {
            push(pc.qw + _vpc->encoding_size);
            _call_stack.push(_vpc + 1);
            OPERAND(0, pc)
            NEXT(_vpc->branch_target)
        }

        _rts:
        {
            pc.qw = pop();
            auto branch_target = _call_stack.top();
            _call_stack.pop();
            NEXT(branch_target)
        }

        _jmp:
        {
            OPERAND(0, pc)
            NEXT(_vpc->branch_target)
        }

        _swi:
        {
            OPERAND(0, lhs)
            size_t swi_offset = sizeof(uint64_t) * lhs.qw;
            uint64_t swi_address = read(op_sizes::qword, swi_offset);
            if (swi_address != 0) {
                push(pc.qw);
                pc.qw = swi_address;
            }
            NEXT(++_vpc)
        }

        _swap:
        {
            OPERAND(1, lhs)

            switch (_vpc->size) {
                case op_sizes::byte: {
                    uint8_t upper_nybble = common::get_upper_nybble(lhs.b);
                    uint8_t lower_nybble = common::get_lower_nybble(lhs.b);
                    result.b = common::set_upper_nybble(result.b, lower_nybble);
                    result.b = common::set_lower_nybble(result.b, upper_nybble);
                    break;
                }
                case op_sizes::word:
                    result.w = common::endian_swap_word(lhs.w);
                    break;
                case op_sizes::dword:
                    result.dw = common::endian_swap_dword(lhs.dw);
                    break;
                case op_sizes::qword:
                default:
                    result.qw = common::endian_swap_qword(lhs.qw);
                    break;
            }

            TARGET(0, result)

            _registers.set_flags(
                is_zero(result),
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _trap:
        {
            OPERAND(0, lhs)
            execute_trap(lhs.b);
            NEXT(++_vpc)
        }

        _ffi:
        {
            OPERAND(0, lhs)
            if (_vpc->values_count > 1)
                OPERAND(1, rhs);

            auto func = _ffi->find_function(lhs.qw);
            if (func == nullptr) {
                execute_trap(trap_invalid_ffi_call);
                return false;
            }

            _ffi->reset();
            _ffi->calling_convention(func->calling_mode);

            vm::function_value_list_t* arguments = nullptr;
            if (func->is_variadic()) {
                auto it = func->call_site_arguments.find(static_cast<common::id_t>(rhs.qw));
                if (it == func->call_site_arguments.end()) {
                    execute_trap(trap_invalid_ffi_call);
                    return false;
                }
                arguments = &it->second;
            } else {
                arguments = &func->arguments;
            }

            for (const auto& arg : *arguments) {
                auto value = pop();
                _ffi->push(arg, value);
            }

            result.qw = _ffi->call(func);
            if (func->return_value.type != ffi_types_t::void_type) {
                push(result.qw);
            }

            _registers.set_flags(
                result.qw == 0,
                false,
                false,
                is_negative(result));

            NEXT(++_vpc)
        }

        _meta:
        {
            OPERAND(0, lhs)
            OPERAND(1, rhs)
            NEXT(++_vpc)
        }

        _exit:
        {
            _exited = true;
            return !r.is_failed();
        }
    }

    bool terp::has_exited() const {
        return _exited;
    }

    size_t terp::heap_size() const {
        return _heap_size;
    }

    bool terp::bounds_check_address(
            common::result& r,
            const register_value_alias_t& address) {
        auto heap_bottom = _heap_address;
        auto heap_top = _heap_address + _heap_size;

        if (address.qw < heap_bottom
        ||  address.qw > heap_top) {
            execute_trap(trap_invalid_address);
            r.error(
                "B004",
                fmt::format(
                    "invalid address: ${:016X}; bottom: ${:016X}; top: ${:016X}",
                    address.qw,
                    heap_bottom,
                    heap_top));
            return false;
        }

        return true;
    }

    size_t terp::stack_size() const {
        return _stack_size;
    }

    void terp::push(uint64_t value) {
        _registers.r[register_sp].qw -= sizeof(uint64_t);
        write(op_sizes::qword, _registers.r[register_sp].qw, value);
    }

    bool terp::initialize(common::result& r) {
        if (_heap != nullptr)
            return true;

        _heap = new uint8_t[_heap_size];
        _heap_address = reinterpret_cast<uint64_t>(_heap);

        heap_vector(
            heap_vectors_t::top_of_stack,
            _heap_address + _heap_size);
        heap_vector(
            heap_vectors_t::bottom_of_stack,
            _heap_address + (_heap_size - _stack_size));
        heap_vector(
            heap_vectors_t::program_start,
            _heap_address + program_start);

        _ffi->clear();
        reset();

        return !r.is_failed();
    }

    void terp::register_trap(
            uint8_t index,
            const terp::trap_callable& callable) {
        _traps.insert(std::make_pair(index, callable));
    }

    void terp::remove_trap(uint8_t index) {
        _traps.erase(index);
    }

    void terp::execute_trap(uint8_t index) {
        auto it = _traps.find(index);
        if (it == _traps.end())
            return;
        it->second(this);
    }

    void terp::swi(uint8_t index, uint64_t address) {
        size_t swi_address = interrupt_vector_table_start + (sizeof(uint64_t) * index);
        write(op_sizes::qword, swi_address, address);
    }

    const register_file_t& terp::register_file() const {
        return _registers;
    }

    void terp::dump_heap(uint64_t offset, size_t size) {
        auto program_memory = common::hex_formatter::dump_to_string(
            reinterpret_cast<const void*>(_heap + offset),
            size);
        fmt::print("{}\n", program_memory);
    }

    void terp::heap_free_space_begin(uint64_t address) {
        heap_vector(heap_vectors_t::free_space_start, address);
        initialize_allocator();
    }

    uint64_t terp::heap_vector(heap_vectors_t vector) const {
        uint64_t heap_vector_address =
            _heap_address +
            heap_vector_table_start +
            (sizeof(uint64_t) * static_cast<uint8_t>(vector));
        return read(op_sizes::qword, heap_vector_address);
    }

    const meta_information_t& terp::meta_information() const {
        return _meta_information;
    }

    // XXX: need to add support for both big and little endian
    uint64_t terp::read(op_sizes size, uint64_t address) const {
        uint64_t result = 0;
        auto heap_ptr = reinterpret_cast<uint8_t*>(address);
        auto result_ptr = reinterpret_cast<uint8_t*>(&result);
        switch (size) {
            case op_sizes::none: {
                break;
            }
            case op_sizes::byte: {
                *(result_ptr + 0) = *heap_ptr;
                break;
            }
            case op_sizes::word: {
                *(result_ptr + 0) = heap_ptr[0];
                *(result_ptr + 1) = heap_ptr[1];
                break;
            }
            case op_sizes::dword: {
                *(result_ptr + 0) = heap_ptr[0];
                *(result_ptr + 1) = heap_ptr[1];
                *(result_ptr + 2) = heap_ptr[2];
                *(result_ptr + 3) = heap_ptr[3];
                break;
            }
            case op_sizes::qword: {
                *(result_ptr + 0) = heap_ptr[0];
                *(result_ptr + 1) = heap_ptr[1];
                *(result_ptr + 2) = heap_ptr[2];
                *(result_ptr + 3) = heap_ptr[3];
                *(result_ptr + 4) = heap_ptr[4];
                *(result_ptr + 5) = heap_ptr[5];
                *(result_ptr + 6) = heap_ptr[6];
                *(result_ptr + 7) = heap_ptr[7];
                break;
            }
        }
        return result;
    }

    void terp::heap_vector(heap_vectors_t vector, uint64_t address) {
        size_t heap_vector_address =
            _heap_address +
            heap_vector_table_start +
            (sizeof(uint64_t) * static_cast<uint8_t>(vector));
        write(op_sizes::qword, heap_vector_address, address);
    }

    // XXX: need to add support for both big and little endian
    void terp::write(op_sizes size, uint64_t address, uint64_t value) {
        auto heap_ptr = reinterpret_cast<uint8_t*>(address);
        auto value_ptr = reinterpret_cast<uint8_t*>(&value);
        switch (size) {
            case op_sizes::none: {
                break;
            }
            case op_sizes::byte: {
                *heap_ptr = static_cast<uint8_t>(value);
                break;
            }
            case op_sizes::word: {
                *(heap_ptr + 0) = value_ptr[0];
                *(heap_ptr + 1) = value_ptr[1];
                break;
            }
            case op_sizes::dword: {
                *(heap_ptr + 0) = value_ptr[0];
                *(heap_ptr + 1) = value_ptr[1];
                *(heap_ptr + 2) = value_ptr[2];
                *(heap_ptr + 3) = value_ptr[3];
                break;
            }
            case op_sizes::qword: {
                *(heap_ptr + 0) = value_ptr[0];
                *(heap_ptr + 1) = value_ptr[1];
                *(heap_ptr + 2) = value_ptr[2];
                *(heap_ptr + 3) = value_ptr[3];
                *(heap_ptr + 4) = value_ptr[4];
                *(heap_ptr + 5) = value_ptr[5];
                *(heap_ptr + 6) = value_ptr[6];
                *(heap_ptr + 7) = value_ptr[7];
                break;
            }
        }
    }

    std::string terp::disassemble(common::result& r, uint64_t address) {
        instruction_t inst;
        std::stringstream stream;
        while (true) {
            auto size = inst.decode(r, address);
            if (size == 0)
                break;

            stream << fmt::format("${:016X}: ", address)
                   << inst.disassemble()
                   << fmt::format(" (${:02X} bytes)\n", size);

            if (inst.op == op_codes::exit)
                break;

            address += size;
        }
        return stream.str();
    }

}
