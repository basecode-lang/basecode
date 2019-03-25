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

#define NEXT { \
        cache_entry = _icache.fetch(r); \
        inst = &cache_entry->inst; \
        _registers.r[register_pc].qw += cache_entry->size; \
        goto *op_handlers[static_cast<uint8_t>(inst->op)]; \
    } \

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

            std::stringstream operands_stream;
            for (size_t i = 0; i < operands_count; i++) {
                if (i > 0 && i < operands_count) {
                    operands_stream << ", ";
                }

                const auto& operand = operands[i];
                register_value_alias_t alias {};
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
        op = static_cast<op_codes>(*(encoding_ptr + 1));
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

    instruction_cache::instruction_cache(terp* terp) : _terp(terp) {
    }

    void instruction_cache::reset() {
        _cache.clear();
    }

    icache_entry_t* instruction_cache::fetch(common::result& r) {
        return fetch_at(r, _terp->register_file().r[register_pc].qw);
    }

    icache_entry_t* instruction_cache::fetch_at(common::result& r, uint64_t address) {
        auto it = _cache.find(address);
        if (it == _cache.end()) {
            auto result = _cache.insert(std::make_pair(address, icache_entry_t{}));
            auto entry = &result.first->second;
            entry->size = entry->inst.decode(r, address);
            return entry;
        } else {
            return &it->second;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    terp::terp(
            vm::ffi* ffi,
            vm::allocator* allocator,
            size_t heap_size,
            size_t stack_size) : _ffi(ffi),
                                 _heap_size(heap_size),
                                 _stack_size(stack_size),
                                 _icache(this),
                                 _allocator(allocator) {
    }

    terp::~terp() {
        delete _heap;
        _heap = nullptr;
    }

    void terp::reset() {
        _registers.r[register_pc].qw = heap_vector(heap_vectors_t::program_start);
        _registers.r[register_sp].qw = heap_vector(heap_vectors_t::top_of_stack);
        _registers.r[register_fp].qw = 0;
        _registers.r[register_fr].qw = 0;
        _registers.r[register_sr].qw = 0;

        for (size_t i = 0; i < number_general_purpose_registers; i++) {
            _registers.r[i].qw = 0;
        }

        _icache.reset();
        _allocator->reset();

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

    bool terp::run(common::result& r) {
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

        instruction_t* inst = nullptr;
        icache_entry_t* cache_entry = nullptr;

        // N.B. this starts execution
        NEXT
        return false;

        _nop:
        {
            NEXT
        }

        _alloc:
        {
            register_value_alias_t size{};
            get_operand_value(r, inst, 1, size);

            size.qw *= op_size_in_bytes(inst->size);

            register_value_alias_t address{};
            address.qw = _allocator->alloc(size.qw);
            if (address.qw == 0) {
                execute_trap(trap_out_of_memory);
                // XXX: how to handle errors
            }

            set_target_operand_value(r, inst->operands[0], op_sizes::qword, address);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, address.qw == 0);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(address, inst->size));
            NEXT

        }

        _free:
        {
            register_value_alias_t address{};
            get_operand_value(r, inst, 0, address);

            auto freed_size = _allocator->free(address.qw);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::negative, false);
            _registers.flags(register_file_t::flags_t::zero, freed_size != 0);

            NEXT

        }

        _size:
        {
            register_value_alias_t address{};
            get_operand_value(r, inst, 1, address);

            register_value_alias_t block_size{};
            block_size.qw = _allocator->size(address.qw);
            set_target_operand_value(r, inst->operands[0], inst->size, block_size);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, block_size.qw == 0);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(block_size, inst->size));

            NEXT

        }

        _load:
        {
            register_value_alias_t address{};
            get_address_with_offset(r, inst, 1, 2, address);

            register_value_alias_t loaded_data{};
            loaded_data.qw = read(inst->size, address.qw);
            auto zero_flag = is_zero(inst->size, loaded_data);
            set_target_operand_value(r, inst->operands[0], inst->size, loaded_data);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(loaded_data, inst->size));

            NEXT

        }

        _store:
        {
            register_value_alias_t address{};
            get_address_with_offset(r, inst, 0, 2, address);

            register_value_alias_t data{};
            get_operand_value(r, inst, 1, data);

            auto zero_flag = is_zero(inst->size, data);
            write(inst->size, address.qw, data.qw);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(data, inst->size));

            NEXT

        }

        _copy:
        {
            register_value_alias_t source_address{};
            register_value_alias_t target_address{};

            get_operand_value(r, inst, 0, target_address);
            if (!bounds_check_address(r, target_address))
                return false;

            get_operand_value(r, inst, 1, source_address);
            if (!bounds_check_address(r, source_address))
                return false;

            register_value_alias_t length{};
            get_operand_value(r, inst, 2, length);

            memcpy(
                reinterpret_cast<void*>(target_address.qw),
                reinterpret_cast<void*>(source_address.qw),
                length.qw * op_size_in_bytes(inst->size));

            _registers.flags(register_file_t::flags_t::zero, false);
            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::negative, false);

            NEXT

        }

        _convert:
        {
            register_value_alias_t target_value{};
            get_operand_value(r, inst, 0, target_value);

            register_value_alias_t value{};
            get_operand_value(r, inst, 1, value);

            auto casted_value = value;

            // XXX: how to handle NaN & Inf for integers
            if (inst->operands[0].is_integer()) {
                if (!inst->operands[1].is_integer()) {
                    switch (inst->size) {
                        case op_sizes::dword:
                            casted_value.dw = static_cast<uint64_t>(value.dwf);
                            break;
                        case op_sizes::qword:
                            casted_value.qw = static_cast<uint64_t>(value.qwf);
                            break;
                        default:
                            // XXX: this is an error
                            break;
                    }
                }
            } else {
                if (inst->operands[1].is_integer()) {
                    switch (inst->size) {
                        case op_sizes::dword:
                            casted_value.dwf = static_cast<float>(value.dw);
                            break;
                        case op_sizes::qword:
                            casted_value.qwf = static_cast<double>(value.qw);
                            break;
                        default:
                            // XXX: this is an error
                            break;
                    }
                } else {
                    switch (inst->size) {
                        case op_sizes::dword:
                            casted_value.dwf = static_cast<float>(value.qwf);
                            break;
                        case op_sizes::qword:
                            casted_value.qwf = value.dwf;
                            break;
                        default:
                            // XXX: this is an error
                            break;
                    }

                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, casted_value);

            NEXT

        }

        _fill:
        {
            register_value_alias_t address{};
            get_operand_value(r, inst, 0, address);
            if (!bounds_check_address(r, address))
                return false;

            register_value_alias_t value{};
            get_operand_value(r, inst, 1, value);

            register_value_alias_t length{};
            get_operand_value(r, inst, 2, length);

            length.qw *= op_size_in_bytes(inst->size);

            switch (inst->size) {
                case op_sizes::byte:
                    memset(
                        reinterpret_cast<void*>(address.qw),
                        value.b,
                        length.qw);
                    break;
                case op_sizes::word:
                    memset(
                        reinterpret_cast<void*>(address.qw),
                        value.w,
                        length.qw);
                    break;
                case op_sizes::dword:
                    memset(
                        reinterpret_cast<void*>(address.qw),
                        value.dw,
                        length.qw);
                    break;
                case op_sizes::qword:
                default:
                    // XXX: this is an error
                    break;
            }

            _registers.flags(register_file_t::flags_t::zero, false);
            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::negative, false);

            NEXT

        }

        _clr:
        {
            register_value_alias_t value{};
            value.qw = 0;

            set_target_operand_value(r, inst->operands[0], inst->size, value);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::zero, true);
            _registers.flags(register_file_t::flags_t::negative, false);

            NEXT

        }

        _move:
        {
            register_value_alias_t source_value{};
            get_operand_value(r, inst, 1, source_value);

            register_value_alias_t offset_value{};
            get_address_with_offset(r, inst, 1, 2, offset_value);

            set_target_operand_value(r, inst->operands[0], inst->size, offset_value);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::zero, source_value.qw == 0);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(source_value, inst->size));

            NEXT
        }

        _moves:
        {
            register_value_alias_t source_value{};
            get_operand_value(r, inst, 1, source_value);

            register_value_alias_t offset_value{};
            get_address_with_offset(r, inst, 1, 2, offset_value);

            auto previous_size = static_cast<op_sizes>(static_cast<uint8_t>(inst->size) - 1);
            offset_value.qw = common::sign_extend(
                offset_value.qw,
                static_cast<uint32_t>(op_size_in_bytes(previous_size) * 8));

            set_target_operand_value(r, inst->operands[0], inst->size, offset_value);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::zero, source_value.qw == 0);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(source_value, inst->size));

            NEXT
        }

        _movez:
        {
            register_value_alias_t source_value{};
            get_operand_value(r, inst, 1, source_value);

            register_value_alias_t offset_value{};
            get_address_with_offset(r, inst, 1, 2, offset_value);

            switch (inst->size) {
                case op_sizes::none:
                case op_sizes::byte: {
                    break;
                }
                case op_sizes::word: {
                    offset_value.w &= 0b0000000011111111;
                    break;
                }
                case op_sizes::dword: {
                    offset_value.dw &= 0b00000000000000001111111111111111;
                    break;
                }
                case op_sizes::qword: {
                    offset_value.qw &= 0b0000000000000000000000000000000011111111111111111111111111111111;
                    break;
                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, offset_value);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::zero, source_value.qw == 0);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(source_value, inst->size));

            NEXT
        }

        _push:
        {
            register_value_alias_t source_value{};

            get_operand_value(r, inst, 0, source_value);
            push(source_value.qw);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::zero, source_value.qw == 0);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(source_value, inst->size));

            NEXT
        }

        _pushm:
        {
            for (uint8_t i = 0; i < inst->operands_count; i++) {
                const auto& operand = inst->operands[i];
                auto type = operand.is_integer() ?
                            register_type_t::integer :
                            register_type_t::floating_point;

                if (operand.is_range()) {
                    auto range = static_cast<uint16_t>(operand.value.u);
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
                        auto alias = _registers.r[reg_index];
                        push(alias.qw);
                    }
                } else if (operand.is_reg()) {
                    auto reg_index = register_index(
                        static_cast<registers_t>(operand.value.r),
                        type);
                    auto alias = _registers.r[reg_index];
                    push(alias.qw);
                }
            }

            _registers.flags(register_file_t::flags_t::zero, false);
            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::negative, false);

            NEXT
        }

        _pop:
        {
            register_value_alias_t top_of_stack{};
            top_of_stack.qw = pop();

            set_target_operand_value(r, inst->operands[0], inst->size, top_of_stack);

            _registers.flags(register_file_t::flags_t::zero, top_of_stack.qw == 0);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(top_of_stack, inst->size));

            NEXT
        }

        _popm:
        {
            for (uint8_t i = 0; i < inst->operands_count; i++) {
                const auto& operand = inst->operands[i];
                auto type = operand.is_integer() ?
                            register_type_t::integer :
                            register_type_t::floating_point;

                if (operand.is_range()) {
                    auto range = static_cast<uint16_t>(operand.value.u);
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
                        _registers.r[reg_index].qw = pop();
                    }
                } else if (operand.is_reg()) {
                    auto reg_index = register_index(
                        static_cast<registers_t>(operand.value.r),
                        type);
                    _registers.r[reg_index].qw = pop();
                }
            }

            _registers.flags(register_file_t::flags_t::zero, false);
            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::negative, false);

            NEXT
        }

        _dup:
        {
            auto top_of_stack = peek();
            push(top_of_stack);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::zero, top_of_stack == 0);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(register_value_alias_t{.qw = top_of_stack}, inst->size));

            NEXT
        }

        _inc:
        {
            register_value_alias_t reg_value{};
            get_operand_value(r, inst, 0, reg_value);

            register_value_alias_t one{};
            one.qw = 1;

            register_value_alias_t new_value{};
            if (inst->operands[0].is_integer()) {
                switch (inst->size) {
                    case op_sizes::byte: {
                        new_value.b = reg_value.b + one.b;
                        break;
                    }
                    case op_sizes::word: {
                        new_value.w = reg_value.w + one.w;
                        break;
                    }
                    case op_sizes::dword: {
                        new_value.dw = reg_value.dw + one.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        new_value.qw = reg_value.qw + one.qw;
                        break;
                    }
                }
            } else {
                if (inst->size == op_sizes::dword)
                    new_value.dwf = reg_value.dwf + 1.0f;
                else
                    new_value.qwf = reg_value.qwf + 1.0;
            }

//                if (_white_listed_addresses.count(reg_value.qw) > 0) {
//                    _white_listed_addresses.insert(new_value.qw);
//                }

            set_target_operand_value(r, inst->operands[0], inst->size, new_value);

            _registers.flags(
                register_file_t::flags_t::overflow,
                has_overflow(reg_value, one, new_value, inst->size));
            _registers.flags(register_file_t::flags_t::zero, new_value.qw == 0);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(
                register_file_t::flags_t::carry,
                has_carry(reg_value.qw, one.qw, inst->size));
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(new_value, inst->size));

            NEXT
        }

        _dec:
        {
            register_value_alias_t reg_value{};
            get_operand_value(r, inst, 0, reg_value);

            register_value_alias_t one{};
            one.qw = 1;

            register_value_alias_t new_value{};
            if (inst->operands[0].is_integer()) {
                switch (inst->size) {
                    case op_sizes::byte: {
                        new_value.b = reg_value.b - one.b;
                        break;
                    }
                    case op_sizes::word: {
                        new_value.w = reg_value.w - one.w;
                        break;
                    }
                    case op_sizes::dword: {
                        new_value.dw = reg_value.dw - one.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        new_value.qw = reg_value.qw - one.qw;
                        break;
                    }
                }
            } else {
                if (inst->size == op_sizes::dword)
                    new_value.dwf = reg_value.dwf - 1.0f;
                else
                    new_value.qwf = reg_value.qwf - 1.0;
            }

//                if (_white_listed_addresses.count(reg_value.qw) > 0) {
//                    _white_listed_addresses.insert(new_value.qw);
//                }

            set_target_operand_value(r, inst->operands[0], inst->size, new_value);

            _registers.flags(
                register_file_t::flags_t::overflow,
                has_overflow(reg_value, one, new_value, inst->size));
            _registers.flags(register_file_t::flags_t::subtract, true);
            _registers.flags(register_file_t::flags_t::zero, new_value.qw == 0);
            _registers.flags(
                register_file_t::flags_t::carry,
                has_carry(reg_value.qw, one.qw, inst->size));
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(new_value, inst->size));

            NEXT
        }

        _add:
        {
            register_value_alias_t lhs_value{};
            register_value_alias_t rhs_value{};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t sum_result{};

            if (inst->operands[1].is_integer()
	    &&  inst->operands[2].is_integer()) {
                switch (inst->size) {
                    case op_sizes::byte: {
                        sum_result.b = lhs_value.b + rhs_value.b;
                        break;
                    }
                    case op_sizes::word: {
                        sum_result.w = lhs_value.w + rhs_value.w;
                        break;
                    }
                    case op_sizes::dword: {
                        sum_result.dw = lhs_value.dw + rhs_value.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        sum_result.qw = lhs_value.qw + rhs_value.qw;
                        break;
                    }
                }
            } else {
                if (inst->size == op_sizes::dword)
                    sum_result.dwf = lhs_value.dwf + rhs_value.dwf;
                else
                    sum_result.qwf = lhs_value.qwf + rhs_value.qwf;
            }

//                if (_white_listed_addresses.count(lhs_value.qw) > 0
//                ||  _white_listed_addresses.count(rhs_value.qw) > 0) {
//                    _white_listed_addresses.insert(sum_result.qw);
//                }

            set_target_operand_value(r, inst->operands[0], inst->size, sum_result);

            auto zero_flag = is_zero(inst->size, sum_result);
            auto carry_flag = has_carry(lhs_value.qw, rhs_value.qw, inst->size);

            _registers.flags(
                register_file_t::flags_t::overflow,
                has_overflow(lhs_value, rhs_value, sum_result, inst->size));
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(register_file_t::flags_t::carry, carry_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(sum_result, inst->size));

            NEXT
        }

        _sub:
        {
            register_value_alias_t lhs {};
            register_value_alias_t rhs {};

            get_operand_value(r, inst, 1, lhs);
            get_operand_value(r, inst, 2, rhs);

            auto carry_flag = false;
            register_value_alias_t subtraction_result {};

            if (inst->operands[1].is_integer()
            &&  inst->operands[2].is_integer()) {
                switch (inst->size) {
                    case op_sizes::byte:
                        carry_flag = lhs.b < rhs.b;
                        subtraction_result.b = lhs.b - rhs.b;
                        break;
                    case op_sizes::word:
                        carry_flag = lhs.w < rhs.w;
                        subtraction_result.w = lhs.w - rhs.w;
                        break;
                    case op_sizes::dword:
                        carry_flag = lhs.dw < rhs.dw;
                        subtraction_result.dw = lhs.dw - rhs.dw;
                        break;
                    case op_sizes::qword:
                        carry_flag = lhs.qw < rhs.qw;
                        subtraction_result.qw = lhs.qw - rhs.qw;
                        break;
                    default:
                        return false;
                }
            } else {
                if (inst->size == op_sizes::dword)
                    subtraction_result.dwf = lhs.dwf - rhs.dwf;
                else
                    subtraction_result.qwf = lhs.qwf - rhs.qwf;
            }

//                if (_white_listed_addresses.count(lhs.qw) > 0
//                ||  _white_listed_addresses.count(rhs.qw) > 0) {
//                    _white_listed_addresses.insert(subtraction_result.qw);
//                }

            set_target_operand_value(r, inst->operands[0], inst->size, subtraction_result);

            auto zero_flag = is_zero(inst->size, subtraction_result);
            auto overflow_flag = has_overflow(lhs, rhs, subtraction_result, inst->size);

            _registers.flags(register_file_t::flags_t::subtract, true);
            _registers.flags(register_file_t::flags_t::carry, carry_flag);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::overflow,
                !carry_flag && overflow_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(subtraction_result, inst->size));

            NEXT
        }

        _mul:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t product_result {};

            if (inst->operands[1].is_integer()
            &&  inst->operands[2].is_integer()) {
                switch (inst->size) {
                    case op_sizes::byte: {
                        product_result.b = lhs_value.b * rhs_value.b;
                        break;
                    }
                    case op_sizes::word: {
                        product_result.w = lhs_value.w * rhs_value.w;
                        break;
                    }
                    case op_sizes::dword: {
                        product_result.dw = lhs_value.dw * rhs_value.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        product_result.qw = lhs_value.qw * rhs_value.qw;
                        break;
                    }
                }
            } else {
                if (inst->size == op_sizes::dword)
                    product_result.dwf = lhs_value.dwf * rhs_value.dwf;
                else
                    product_result.qwf = lhs_value.qwf * rhs_value.qwf;
            }

            set_target_operand_value(r, inst->operands[0], inst->size, product_result);

            auto zero_flag = is_zero(inst->size, product_result);
            auto carry_flag = has_carry(lhs_value.qw, rhs_value.qw, inst->size);

            _registers.flags(
                register_file_t::flags_t::overflow,
                has_overflow(lhs_value, rhs_value, product_result, inst->size));
            _registers.flags(register_file_t::flags_t::carry, carry_flag);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(product_result, inst->size));

            NEXT
        }

        _div:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t result {};

            if (inst->operands[1].is_integer()
            &&  inst->operands[2].is_integer()) {
                switch (inst->size) {
                    case op_sizes::byte: {
                        if (rhs_value.b != 0)
                            result.b = lhs_value.b / rhs_value.b;
                        break;
                    }
                    case op_sizes::word: {
                        if (rhs_value.w != 0)
                            result.w = lhs_value.w / rhs_value.w;
                        break;
                    }
                    case op_sizes::dword: {
                        if (rhs_value.dw != 0)
                            result.dw = lhs_value.dw / rhs_value.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        if (rhs_value.qw != 0)
                            result.qw = lhs_value.qw / rhs_value.qw;
                        break;
                    }
                }
            } else {
                if (inst->size == op_sizes::dword) {
                    if (rhs_value.dwf != 0) {
                        result.dwf = lhs_value.dwf / rhs_value.dwf;
                    }
                } else {
                    if (rhs_value.qwf != 0) {
                        result.qwf = lhs_value.qwf / rhs_value.qwf;
                    }
                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, result);

            auto zero_flag = is_zero(inst->size, result);
            auto carry_flag = has_carry(lhs_value.qw, rhs_value.qw, inst->size);
            auto overflow_flag = has_overflow(lhs_value, rhs_value, result, inst->size);

            _registers.flags(
                register_file_t::flags_t::overflow,
                overflow_flag);
            _registers.flags(register_file_t::flags_t::carry, carry_flag);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _mod:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    if (lhs_value.b != 0 && rhs_value.b != 0)
                        result.b = lhs_value.b % rhs_value.b;
                    break;
                }
                case op_sizes::word: {
                    if (lhs_value.w != 0 && rhs_value.w != 0)
                        result.w = lhs_value.w % rhs_value.w;
                    break;
                }
                case op_sizes::dword: {
                    if (lhs_value.dw != 0 && rhs_value.dw != 0)
                        result.dw = lhs_value.dw % rhs_value.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    if (lhs_value.qw != 0 && rhs_value.qw != 0)
                        result.qw = lhs_value.qw % rhs_value.qw;
                    break;
                }
            }

            auto zero_flag = is_zero(inst->size, result);
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            _registers.flags(
                register_file_t::flags_t::overflow,
                has_overflow(lhs_value, rhs_value, result, inst->size));
            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _neg:
        {
            register_value_alias_t value {};
            get_operand_value(r, inst, 1, value);

            register_value_alias_t result {};

            if (inst->operands[1].is_integer()) {
                switch (inst->size) {
                    case op_sizes::byte: {
                        int8_t negated_result = -value.b;
                        result.b = static_cast<uint64_t>(negated_result);
                        break;
                    }
                    case op_sizes::word: {
                        int16_t negated_result = -value.w;
                        result.w = static_cast<uint64_t>(negated_result);
                        break;
                    }
                    case op_sizes::dword: {
                        int32_t negated_result = -value.dw;
                        result.dw = static_cast<uint64_t>(negated_result);
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        int64_t negated_result = -value.qw;
                        result.qw = static_cast<uint64_t>(negated_result);
                        break;
                    }
                }
            } else {
                if (inst->size == op_sizes::dword)
                    result.dwf = value.dwf * -1.0f;
                else
                    result.qwf = value.qwf * -1.0;
            }

            auto zero_flag = is_zero(inst->size, result);
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _shr:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    result.b = lhs_value.b >> rhs_value.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs_value.w >> rhs_value.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs_value.dw >> rhs_value.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs_value.qw >> rhs_value.qw;
                    break;
                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, result);

            auto zero_flag = is_zero(inst->size, result);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _shl:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    result.b = lhs_value.b << rhs_value.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs_value.w << rhs_value.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs_value.dw << rhs_value.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs_value.qw << rhs_value.qw;
                    break;
                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, result);

            auto zero_flag = is_zero(inst->size, result);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _ror:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t right_rotated_value {};
            right_rotated_value.qw = common::rotr(lhs_value.qw, rhs_value.b);
            set_target_operand_value(r, inst->operands[0], inst->size, right_rotated_value);

            auto zero_flag = is_zero(inst->size, right_rotated_value);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(right_rotated_value, inst->size));

            NEXT
        }

        _rol:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t left_rotated_value {};
            left_rotated_value.qw = common::rotl(lhs_value.qw, rhs_value.b);
            set_target_operand_value(r, inst->operands[0], inst->size, left_rotated_value);

            auto zero_flag = is_zero(inst->size, left_rotated_value);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(left_rotated_value, inst->size));

            NEXT
        }

        _pow:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t power_value {};

            if (inst->operands[1].is_integer()
            &&  inst->operands[2].is_integer()) {
                switch (inst->size) {
                    case op_sizes::byte: {
                        power_value.b = common::power(lhs_value.b, rhs_value.b);
                        break;
                    }
                    case op_sizes::word: {
                        power_value.w = common::power(lhs_value.w, rhs_value.w);
                        break;
                    }
                    case op_sizes::dword: {
                        power_value.dw = common::power(lhs_value.dw, rhs_value.dw);
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        power_value.qw = common::power(lhs_value.qw, rhs_value.qw);
                        break;
                    }
                }
            } else {
                if (inst->size == op_sizes::dword) {
                    power_value.dwf = std::pow(lhs_value.dwf, rhs_value.dwf);
                } else {
                    power_value.qwf = std::pow(lhs_value.qwf, rhs_value.qwf);
                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, power_value);

            auto zero_flag = is_zero(inst->size, power_value);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(power_value, inst->size));

            NEXT
        }

        _and:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    result.b = lhs_value.b & rhs_value.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs_value.w & rhs_value.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs_value.dw & rhs_value.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs_value.qw & rhs_value.qw;
                    break;
                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, result);

            auto zero_flag = is_zero(inst->size, result);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _or:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    result.b = lhs_value.b | rhs_value.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs_value.w | rhs_value.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs_value.dw | rhs_value.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs_value.qw | rhs_value.qw;
                    break;
                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, result);

            auto zero_flag = is_zero(inst->size, result);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _xor:
        {
            register_value_alias_t lhs_value {};
            register_value_alias_t rhs_value {};

            get_operand_value(r, inst, 1, lhs_value);
            get_operand_value(r, inst, 2, rhs_value);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    result.b = lhs_value.b ^ rhs_value.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = lhs_value.w ^ rhs_value.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = lhs_value.dw ^ rhs_value.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = lhs_value.qw ^ rhs_value.qw;
                    break;
                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, result);

            auto zero_flag = is_zero(inst->size, result);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _not:
        {
            register_value_alias_t value {};

            get_operand_value(r, inst, 1, value);

            switch (inst->size) {
                case op_sizes::byte: {
                    value.b = ~value.b;
                    break;
                }
                case op_sizes::word: {
                    value.w = ~value.w;
                    break;
                }
                case op_sizes::dword: {
                    value.dw = ~value.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    value.qw = ~value.qw;
                    break;
                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, value);

            auto zero_flag = is_zero(inst->size, value);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(value, inst->size));

            NEXT
        }

        _bis:
        {
            register_value_alias_t value {};
            register_value_alias_t bit_number {};

            get_operand_value(r, inst, 1, value);
            get_operand_value(r, inst, 2, bit_number);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    auto masked_value = static_cast<uint8_t>(1) << bit_number.b;
                    result.b = value.b | masked_value;
                    break;
                }
                case op_sizes::word: {
                    auto masked_value = static_cast<uint16_t>(1) << bit_number.b;
                    result.w = value.w | masked_value;
                    break;
                }
                case op_sizes::dword: {
                    auto masked_value = static_cast<uint32_t>(1) << bit_number.b;
                    result.dw = value.dw | masked_value;
                    break;
                }
                default:
                case op_sizes::qword: {
                    auto masked_value = static_cast<uint64_t>(1) << bit_number.b;
                    result.qw = value.qw | masked_value;
                    break;
                }
            }

            set_target_operand_value(r, inst->operands[0], inst->size, result);

            auto zero_flag = is_zero(inst->size, result);

            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _bic:
        {
            register_value_alias_t value {};
            register_value_alias_t bit_number {};

            get_operand_value(r, inst, 1, value);
            get_operand_value(r, inst, 2, bit_number);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    auto masked_value = ~(static_cast<uint8_t>(1) << bit_number.b);
                    result.b = value.b & masked_value;
                    break;
                }
                case op_sizes::word: {
                    auto masked_value = ~(static_cast<uint16_t>(1) << bit_number.b);
                    result.w = value.w & masked_value;
                    break;
                }
                case op_sizes::dword: {
                    auto masked_value = ~(static_cast<uint32_t>(1) << bit_number.b);
                    result.dw = value.dw & masked_value;
                    break;
                }
                default:
                case op_sizes::qword: {
                    auto masked_value = ~(static_cast<uint64_t>(1) << bit_number.b);
                    result.qw = value.qw & masked_value;
                    break;
                }
            }

            auto zero_flag = is_zero(inst->size, result);
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _test:
        {
            register_value_alias_t value {};
            register_value_alias_t mask {};

            get_operand_value(r, inst, 0, value);
            get_operand_value(r, inst, 1, mask);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    result.b = value.b & mask.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = value.w & mask.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = value.dw & mask.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = value.qw & mask.qw;
                    break;
                }
            }

            auto zero_flag = is_zero(inst->size, result);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _cmp:
        {
            register_value_alias_t lhs {};
            register_value_alias_t rhs {};

            get_operand_value(r, inst, 0, lhs);
            get_operand_value(r, inst, 1, rhs);

            register_value_alias_t result {};
            bool carry_flag = false;

            switch (inst->size) {
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

            auto zero_flag = is_zero(inst->size, result);
            auto overflow_flag = has_overflow(lhs, rhs, result, inst->size);

            _registers.flags(register_file_t::flags_t::subtract, true);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(register_file_t::flags_t::carry, carry_flag);
            _registers.flags(
                register_file_t::flags_t::overflow,
                !carry_flag && overflow_flag);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(result, inst->size));

            NEXT
        }

        _bz:
        {
            register_value_alias_t value {};
            get_operand_value(r, inst, 0, value);

            auto zero_flag = is_zero(inst->size, value);
            if (zero_flag) {
                get_operand_value(r, inst, 1, _registers.r[register_pc]);
            }

            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(value, inst->size));

            NEXT
        }

        _bnz:
        {
            register_value_alias_t value {};
            get_operand_value(r, inst, 0, value);

            auto zero_flag = is_zero(inst->size, value);
            if (!zero_flag) {
                get_operand_value(r, inst, 1, _registers.r[register_pc]);
            }

            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(
                register_file_t::flags_t::negative,
                is_negative(value, inst->size));

            NEXT
        }

        _tbz:
        {
            register_value_alias_t value {};
            register_value_alias_t mask {};

            get_operand_value(r, inst, 0, value);
            get_operand_value(r, inst, 1, mask);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    result.b = value.b & mask.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = value.w & mask.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = value.dw & mask.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = value.qw & mask.qw;
                    break;
                }
            }

            auto zero_flag = is_zero(inst->size, result);
            if (zero_flag) {
                get_operand_value(r, inst, 2, _registers.r[register_pc]);
            }

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst->size));

            NEXT
        }

        _tbnz:
        {
            register_value_alias_t value {};
            register_value_alias_t mask {};

            get_operand_value(r, inst, 0, value);
            get_operand_value(r, inst, 1, mask);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    result.b = value.b & mask.b;
                    break;
                }
                case op_sizes::word: {
                    result.w = value.w & mask.w;
                    break;
                }
                case op_sizes::dword: {
                    result.dw = value.dw & mask.dw;
                    break;
                }
                default:
                case op_sizes::qword: {
                    result.qw = value.qw & mask.qw;
                    break;
                }
            }

            auto zero_flag = is_zero(inst->size, result);
            if (!zero_flag) {
                get_operand_value(r, inst, 2, _registers.r[register_pc]);
            }

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst->size));

            NEXT
        }

        _bne:
        {
            // ZF = 0
            if (!_registers.flags(register_file_t::flags_t::zero)) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _beq:
        {
            // ZF = 1
            if (_registers.flags(register_file_t::flags_t::zero)) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _bs:
        {
            // SF = 1
            if (_registers.flags(register_file_t::flags_t::negative)) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _bo:
        {
            // OF = 1
            if (_registers.flags(register_file_t::flags_t::overflow)) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _bcc:
        {
            // CF = 0
            if (!_registers.flags(register_file_t::flags_t::carry)) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _bb:
        _bae:
        _bcs:
        {
            // CF = 1
            if (_registers.flags(register_file_t::flags_t::carry)) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _ba:
        {
            // CF = 0 and ZF = 0
            if (!_registers.flags(register_file_t::flags_t::zero)
            &&  !_registers.flags(register_file_t::flags_t::carry)) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _bbe:
        {
            // CF = 1 or ZF = 1
            if (_registers.flags(register_file_t::flags_t::carry)
            ||  _registers.flags(register_file_t::flags_t::zero)) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _bg:
        {
            // ZF = 0 and SF = OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            auto zf = _registers.flags(register_file_t::flags_t::zero);

            if (!zf && sf == of) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _bl:
        {
            // SF <> OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);

            if (sf != of) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _bge:
        {
            // SF = OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);

            if (sf == of) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _ble:
        {
            // ZF = 1 or SF <> OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            auto zf = _registers.flags(register_file_t::flags_t::zero);

            if (zf || sf != of) {
                get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    cache_entry->size,
                    _registers.r[register_pc]);
            }

            NEXT
        }

        _seta:
        _setnbe:
        {
            // CF = 0 and ZF = 0
            register_value_alias_t result {};
            auto zero_flag = _registers.flags(register_file_t::flags_t::zero);
            auto carry_flag = _registers.flags(register_file_t::flags_t::carry);
            result.b = !zero_flag && !carry_flag ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setna:
        _setbe:
        {
            // CF = 1 or ZF = 1
            auto zero_flag = _registers.flags(register_file_t::flags_t::zero);
            auto carry_flag = _registers.flags(register_file_t::flags_t::carry);
            register_value_alias_t result {};
            result.b = (zero_flag || carry_flag) ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setb:
        _setc:
        _setnae:
        {
            // CF = 1
            register_value_alias_t result {};
            result.b = _registers.flags(register_file_t::flags_t::carry) ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setnb:
        _setae:
        _setnc:
        {
            // CF = 0
            register_value_alias_t result {};
            result.b = !_registers.flags(register_file_t::flags_t::carry) ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setg:
        _setnle:
        {
            // ZF = 0 and SF = OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            auto zf = _registers.flags(register_file_t::flags_t::zero);
            register_value_alias_t result {};
            result.b = !zf && sf == of ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setl:
        _setnge:
        {
            // SF != OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            register_value_alias_t result {};
            result.b = sf != of ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setnl:
        _setge:
        {
            // SF = OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            register_value_alias_t result {};
            result.b = sf == of ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setle:
        _setng:
        {
            // ZF = 1 or SF != OF
            auto sf = _registers.flags(register_file_t::flags_t::negative);
            auto of = _registers.flags(register_file_t::flags_t::overflow);
            auto zf = _registers.flags(register_file_t::flags_t::zero);
            register_value_alias_t result {};
            result.b = zf || sf != of ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _sets:
        {
            // SF = 1
            register_value_alias_t result {};
            result.b = _registers.flags(register_file_t::flags_t::negative) ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setns:
        {
            // SF = 0
            register_value_alias_t result {};
            result.b = !_registers.flags(register_file_t::flags_t::negative) ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _seto:
        {
            // OF = 1
            register_value_alias_t result {};
            result.b = _registers.flags(register_file_t::flags_t::overflow) ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setno:
        {
            // OF = 0
            register_value_alias_t result {};
            result.b = !_registers.flags(register_file_t::flags_t::overflow) ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setz:
        {
            register_value_alias_t result {};
            result.b = _registers.flags(register_file_t::flags_t::zero) ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _setnz:
        {
            register_value_alias_t result {};
            result.b = !_registers.flags(register_file_t::flags_t::zero) ? 1 : 0;
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            NEXT
        }

        _jsr:
        {
            push(_registers.r[register_pc].qw);

            get_constant_address_or_pc_with_offset(
                r,
                inst,
                0,
                cache_entry->size,
                _registers.r[register_pc]);

            NEXT
        }

        _rts:
        {
            _registers.r[register_pc].qw = pop();

            NEXT
        }

        _jmp:
        {
            get_constant_address_or_pc_with_offset(
                r,
                inst,
                0,
                cache_entry->size,
                _registers.r[register_pc]);

            NEXT
        }

        _swi:
        {
            register_value_alias_t index {};

            get_operand_value(r, inst, 0, index);

            size_t swi_offset = sizeof(uint64_t) * index.qw;
            uint64_t swi_address = read(op_sizes::qword, swi_offset);
            if (swi_address != 0) {
                // XXX: what state should we save and restore here?
                push(_registers.r[register_pc].qw);
                _registers.r[register_pc].qw = swi_address;
            }

            NEXT
        }

        _swap:
        {
            register_value_alias_t value {};
            get_operand_value(r, inst, 1, value);

            register_value_alias_t result {};

            switch (inst->size) {
                case op_sizes::byte: {
                    uint8_t upper_nybble = common::get_upper_nybble(value.b);
                    uint8_t lower_nybble = common::get_lower_nybble(value.b);
                    result.b = common::set_upper_nybble(result.b, lower_nybble);
                    result.b = common::set_lower_nybble(result.b, upper_nybble);
                    break;
                }
                case op_sizes::word:
                    result.w = common::endian_swap_word(value.w);
                    break;
                case op_sizes::dword:
                    result.dw = common::endian_swap_dword(value.dw);
                    break;
                case op_sizes::qword:
                default:
                    result.qw = common::endian_swap_qword(value.qw);
                    break;
            }

            auto zero_flag = is_zero(inst->size, result);
            set_target_operand_value(r, inst->operands[0], inst->size, result);

            _registers.flags(register_file_t::flags_t::carry, false);
            _registers.flags(register_file_t::flags_t::subtract, false);
            _registers.flags(register_file_t::flags_t::overflow, false);
            _registers.flags(register_file_t::flags_t::zero, zero_flag);
            _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst->size));

            NEXT
        }

        _trap:
        {
            register_value_alias_t index {};
            get_operand_value(r, inst, 0, index);
            execute_trap(index.b);

            NEXT
        }

        _ffi:
        {
            register_value_alias_t address {};
            get_operand_value(r, inst, 0, address);

            register_value_alias_t signature_id {};
            if (inst->operands_count > 1) {
                get_operand_value(r, inst, 1, signature_id);
            }

            auto func = _ffi->find_function(address.qw);
            if (func == nullptr) {
                execute_trap(trap_invalid_ffi_call);
                return false;
            }

            _ffi->reset();
            _ffi->calling_convention(func->calling_mode);

            vm::function_value_list_t* arguments = nullptr;
            if (func->is_variadic()) {
                auto it = func->call_site_arguments.find(static_cast<common::id_t>(signature_id.qw));
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

            auto result_value = _ffi->call(func);
            if (func->return_value.type != ffi_types_t::void_type) {
                push(result_value);
//                if (func->return_value.type == ffi_types_t::pointer_type)
//                    _white_listed_addresses.insert(result_value);
            }

            NEXT
        }

        _meta:
        {
            register_value_alias_t meta_data_size {};
            get_operand_value(r, inst, 0, meta_data_size);
            NEXT
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

    size_t terp::stack_size() const {
        return _stack_size;
    }

    void terp::push(uint64_t value) {
        _registers.r[register_sp].qw -= sizeof(uint64_t);
        write(op_sizes::qword, _registers.r[register_sp].qw, value);
    }

    bool terp::bounds_check_address(
            common::result& r,
            const register_value_alias_t& address) {
//        if (_white_listed_addresses.count(address.qw))
//            return true;

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

    std::vector<uint64_t> terp::jump_to_subroutine(
            common::result& r,
            uint64_t address) {
        std::vector<uint64_t> return_values;

        auto return_address = _registers.r[register_pc].qw;
        push(return_address);
        _registers.r[register_pc].qw = address;

        while (!has_exited()) {
            // XXX: need to introduce a terp_step_result_t
            //auto result = step(r);
            // XXX: did an RTS just execute?
            //      does _registers.pc == return_address?  if so, we're done
            //if (!result) {
            //    break;
            //}
        }

        // XXX: how do we handle multiple return values?
        // XXX: pull return values from the stack
        return return_values;
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

    // XXX: need to add support for both big and little endian
    uint64_t terp::read(op_sizes size, uint64_t address) const {
        auto heap_ptr = reinterpret_cast<uint8_t*>(address);
        uint64_t result = 0;
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

    void terp::heap_vector(heap_vectors_t vector, uint64_t address) {
        size_t heap_vector_address =
            _heap_address +
            heap_vector_table_start +
            (sizeof(uint64_t) * static_cast<uint8_t>(vector));
        write(op_sizes::qword, heap_vector_address, address);
    }

    std::string terp::disassemble(common::result& r, uint64_t address) {
        std::stringstream stream;
        while (true) {
            auto cache_entry = _icache.fetch_at(r, address);
            if (cache_entry == nullptr)
                break;

            stream << fmt::format("${:016X}: ", address)
                   << cache_entry->inst.disassemble()
                   << fmt::format(" (${:02X} bytes)\n", cache_entry->size);

            if (cache_entry->inst.op == op_codes::exit)
                break;

            address += cache_entry->size;
        }
        return stream.str();
    }

    bool terp::is_zero(
            op_sizes size,
            const register_value_alias_t& value) {
        switch (size) {
            case op_sizes::byte:
                return value.b == 0;
            case op_sizes::word:
                return value.w == 0;
            case op_sizes::dword:
                return value.dw == 0;
            case op_sizes::qword:
                return value.qw == 0;
            default:
                return false;
        }

        return false;
    }

    bool terp::has_overflow(
            const register_value_alias_t& lhs,
            const register_value_alias_t& rhs,
            const register_value_alias_t& result,
            op_sizes size) {
        switch (size) {
            case op_sizes::byte:
                return ((~(lhs.b ^ rhs.b)) & (lhs.b ^ result.b) & mask_byte_negative) != 0;
            case op_sizes::word:
                return ((~(lhs.w ^ rhs.w)) & (lhs.w ^ result.w) & mask_word_negative) != 0;
            case op_sizes::dword:
                return ((~(lhs.dw ^ rhs.dw)) & (lhs.dw ^ result.dw) & mask_dword_negative) != 0;
            case op_sizes::qword:
            default: {
                return ((~(lhs.qw ^ rhs.qw)) & (lhs.qw ^ result.qw) & mask_qword_negative) != 0;
            }
        }
    }

    void terp::get_operand_value(
            common::result& r,
            const instruction_t* inst,
            uint8_t operand_index,
            register_value_alias_t& value) const {
        const auto& operand = inst->operands[operand_index];
        if (operand.is_reg()) {
            auto reg_index = register_index(
                static_cast<registers_t>(operand.value.r),
                operand.is_integer() ? register_type_t::integer : register_type_t::floating_point);
            value = _registers.r[reg_index];
        } else {
            value.qw = operand.value.u;
            switch (operand.size) {
                case op_sizes::byte: {
                    value.b = static_cast<uint8_t>(operand.value.u);
                    break;
                }
                case op_sizes::word: {
                    value.w = static_cast<uint16_t>(operand.value.u);
                    break;
                }
                case op_sizes::dword: {
                    value.dw = static_cast<uint32_t>(operand.value.u);
                    break;
                }
                case op_sizes::qword: {
                    value.qw = operand.value.u;
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    void terp::set_target_operand_value(
            common::result& r,
            const operand_encoding_t& operand,
            op_sizes size,
            const register_value_alias_t& value) {
        if (operand.is_reg()) {
            auto type = operand.is_integer() ?
                        register_type_t::integer :
                        register_type_t::floating_point;
            auto reg_index = register_index(
                static_cast<registers_t>(operand.value.r),
                type);
            register_set_zoned_value(
                _registers.r[reg_index],
                value.qw,
                size);
        } else {
            r.error(
                "B006",
                "constant cannot be a target operand type.");
        }
    }

    void terp::get_address_with_offset(
            common::result& r,
            const instruction_t* inst,
            uint8_t address_index,
            uint8_t offset_index,
            register_value_alias_t& address) {
        get_operand_value(r, inst, address_index, address);

//        auto is_white_listed = _white_listed_addresses.count(address.qw) > 0;

        if (inst->operands_count > 2) {
            register_value_alias_t offset {};

            get_operand_value(r, inst, offset_index, offset);

            if (inst->operands[offset_index].is_negative()) {
                address.qw -= offset.qw;
            } else {
                address.qw += offset.qw;
            }

//            if(inst.op == op_codes::move
//            || inst.op == op_codes::moves
//            || inst.op == op_codes::movez) {
//                if (is_white_listed)
//                    _white_listed_addresses.insert(address.qw);
//            }
        }

//        if (!is_white_listed
//        && (inst.op == op_codes::load || inst.op == op_codes::store)) {
//            if (!bounds_check_address(r, address)) {
//                // XXX: how to handle?
//                //return false;
//            }
//        }
    }

    void terp::get_constant_address_or_pc_with_offset(
            common::result& r,
            const instruction_t* inst,
            uint8_t operand_index,
            uint64_t inst_size,
            register_value_alias_t& address) {
        get_operand_value(r, inst, operand_index, address);

        if (inst->operands_count >= 2) {
            register_value_alias_t offset {};

            auto offset_index = static_cast<uint8_t>(operand_index + 1);
            get_operand_value(r, inst, offset_index, offset);

            if (inst->operands[offset_index].is_negative()) {
                address.qw -= offset.qw + inst_size;
            } else {
                address.qw += offset.qw - inst_size;
            }
        }
   }

    bool terp::has_carry(
            uint64_t lhs,
            uint64_t rhs,
            op_sizes size) {
        switch (size) {
            case op_sizes::byte:
                return lhs == UINT8_MAX && rhs > 0;
            case op_sizes::word:
                return lhs == UINT16_MAX && rhs > 0;
            case op_sizes::dword:
                return lhs == UINT32_MAX && rhs > 0;
            case op_sizes::qword:
            default:
                return lhs == UINT64_MAX && rhs > 0;
        }
    }

    bool terp::is_negative(const register_value_alias_t& value, op_sizes size) {
        switch (size) {
            case op_sizes::byte: {
                return (value.b & mask_byte_negative) != 0;
            }
            case op_sizes::word: {
                return (value.w & mask_word_negative) != 0;
            }
            case op_sizes::dword: {
                return (value.dw & mask_dword_negative) != 0;
            }
            case op_sizes::qword:
            default:
                return (value.qw & mask_qword_negative) != 0;
        }
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

}
