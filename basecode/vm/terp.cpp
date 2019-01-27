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
#include "instruction_block.h"

namespace basecode::vm {

    size_t instruction_t::encoding_size() const {
        size_t encoding_size = base_size;

        for (size_t i = 0; i < operands_count; i++) {
            const auto& operand = operands[i];
            encoding_size += 1;

            if ((operand.is_reg())) {
                // N.B. the negative flag is used for register ranges
                if (operand.is_negative()) {
                    encoding_size += sizeof(uint16_t);
                } else {
                    encoding_size += sizeof(uint8_t);
                }
            } else {
                auto working_size = operand.size;
                if (operand.fixup_ref != nullptr) {
                    working_size = operand.fixup_ref->size;
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

        uint8_t* encoding_ptr = reinterpret_cast<uint8_t*>(address);
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
                // N.B. the negative flag is used for register ranges
                if (operand.is_negative()) {
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
                // N.B. the negative flag is used for storing register ranges
                if (operand.is_negative()) {
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

    std::string instruction_t::disassemble(const id_resolve_callable& id_resolver) const {
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

                std::string prefix, postfix;

                if (operand.is_negative()) {
                    if (operand.is_prefix())
                        prefix = "--";
                    else
                        prefix = "-";

                    if (operand.is_postfix())
                        postfix = "--";
                } else {
                    if (operand.is_prefix())
                        prefix = "++";

                    if (operand.is_postfix())
                        postfix = "++";
                }

                if (operand.is_reg()) {
                    if (operand.is_integer()) {
                        switch (operand.value.r) {
                            case registers_t::sp: {
                                operands_stream << prefix << "SP" << postfix;
                                break;
                            }
                            case registers_t::fp: {
                                operands_stream << prefix << "FP" << postfix;
                                break;
                            }
                            case registers_t::pc: {
                                operands_stream << prefix << "PC" << postfix;
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
                                operands_stream << prefix
                                                << "I"
                                                << std::to_string(operand.value.r)
                                                << postfix;
                                break;
                            }
                        }
                    } else {
                        operands_stream << "F" << std::to_string(operand.value.r);
                    }
                } else {
//                    if (operand.is_unresolved()) {
//                        if (id_resolver == nullptr)
//                            operands_stream << fmt::format("id({})", operand.value.u);
//                        else
//                            operands_stream << id_resolver(operand.value.u);
//                    } else {
                        operands_stream << prefix;

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

                        operands_stream << postfix;
//                    }
                }
            }

            stream << std::left << std::setw(24) << operands_stream.str();
        } else {
            stream << "UNKNOWN";
        }

        return stream.str();
    }

    ///////////////////////////////////////////////////////////////////////////

    instruction_cache::instruction_cache(terp* terp) : _terp(terp) {
    }

    void instruction_cache::reset() {
        _cache.clear();
    }

    size_t instruction_cache::fetch_at(
            common::result& r,
            uint64_t address,
            instruction_t& inst) {
        auto it = _cache.find(address);
        if (it == _cache.end()) {
            auto size = inst.decode(r, address);
            if (size == 0)
                return 0;
            _cache.insert(std::make_pair(
                address,
                icache_entry_t{.size = size, .inst = inst}));
            return size;
        } else {
            inst = it->second.inst;
            return it->second.size;
        }
    }

    size_t instruction_cache::fetch(common::result& r, instruction_t& inst) {
        return fetch_at(r, _terp->register_file().r[register_pc].qw, inst);
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
        while (!has_exited())
            if (!step(r))
                return false;
        return true;
    }

    bool terp::step(common::result& r) {
        instruction_t inst;
        auto inst_size = _icache.fetch(r, inst);
        if (inst_size == 0)
            return false;

        _registers.r[register_pc].qw += inst_size;

        switch (inst.op) {
            case op_codes::nop: {
                break;
            }
            case op_codes::alloc: {
                operand_value_t size;
                if (!get_operand_value(r, inst, 1, size))
                    return false;

                size.alias.u *= op_size_in_bytes(inst.size);
                operand_value_t address;
                address.alias.u = _allocator->alloc(size.alias.u);
                if (address.alias.u == 0) {
                    execute_trap(trap_out_of_memory);
                    return false;
                }

                if (!set_target_operand_value(r, inst.operands[0], op_sizes::qword, address))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, address.alias.u == 0);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(address, inst.size));

                break;
            }
            case op_codes::free: {
                operand_value_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;

                auto freed_size = _allocator->free(address.alias.u);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::negative, false);
                _registers.flags(register_file_t::flags_t::zero, freed_size != 0);

                break;
            }
            case op_codes::size: {
                operand_value_t address;
                if (!get_operand_value(r, inst, 1, address))
                    return false;

                operand_value_t block_size;
                block_size.alias.u = _allocator->size(address.alias.u);
                if (!set_target_operand_value(r, inst.operands[0], inst.size, block_size))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, block_size.alias.u == 0);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(block_size, inst.size));

                break;
            }
            case op_codes::load: {
                operand_value_t address;

                if (!get_address_with_offset(r, inst, 1, 2, address))
                    return false;

                operand_value_t loaded_data;
                loaded_data.alias.u = read(inst.size, address.alias.u);
                auto zero_flag = is_zero(inst.size, loaded_data);
                if (!set_target_operand_value(r, inst.operands[0], inst.size, loaded_data))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(loaded_data, inst.size));
                break;
            }
            case op_codes::store: {
                operand_value_t address;
                if (!get_address_with_offset(r, inst, 0, 2, address))
                    return false;

                operand_value_t data;
                if (!get_operand_value(r, inst, 1, data))
                    return false;

                auto zero_flag = is_zero(inst.size, data);
                write(inst.size, address.alias.u, data.alias.u);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(data, inst.size));
                break;
            }
            case op_codes::copy: {
                operand_value_t source_address, target_address;

                if (!get_operand_value(r, inst, 0, target_address))
                    return false;

                if (!bounds_check_address(r, target_address))
                    return false;

                if (!get_operand_value(r, inst, 1, source_address))
                    return false;

                if (!bounds_check_address(r, source_address))
                    return false;

                operand_value_t length;
                if (!get_operand_value(r, inst, 2, length))
                    return false;

                memcpy(
                    reinterpret_cast<void*>(target_address.alias.u),
                    reinterpret_cast<void*>(source_address.alias.u),
                    length.alias.u * op_size_in_bytes(inst.size));

                _registers.flags(register_file_t::flags_t::zero, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::negative, false);

                break;
            }
            case op_codes::convert: {
                operand_value_t target_value;
                if (!get_operand_value(r, inst, 0, target_value))
                    return false;

                operand_value_t value;
                if (!get_operand_value(r, inst, 1, value))
                    return false;

                auto casted_value = value;

                // XXX: how to handle NaN & Inf for integers
                switch (target_value.type) {
                    case register_type_t::integer: {
                        casted_value.type = register_type_t::integer;
                        switch (value.type) {
                            case register_type_t::floating_point: {
                                switch (inst.size) {
                                    case op_sizes::dword:
                                        casted_value.alias.u = static_cast<uint64_t>(value.alias.f);
                                        break;
                                    case op_sizes::qword:
                                        casted_value.alias.u = static_cast<uint64_t>(value.alias.d);
                                        break;
                                    default:
                                        // XXX: this is an error
                                        break;
                                }
                                break;
                            }
                            default:
                                break;
                        }
                        break;
                    }
                    case register_type_t::floating_point: {
                        casted_value.type = register_type_t::floating_point;
                        switch (value.type) {
                            case register_type_t::integer: {
                                switch (inst.size) {
                                    case op_sizes::dword:
                                        casted_value.alias.f = static_cast<float>(value.alias.u);
                                        break;
                                    case op_sizes::qword:
                                        casted_value.alias.d = static_cast<double>(value.alias.u);
                                        break;
                                    default:
                                        // XXX: this is an error
                                        break;
                                }
                                break;
                            }
                            case register_type_t::floating_point: {
                                switch (inst.size) {
                                    case op_sizes::dword:
                                        casted_value.alias.f = static_cast<float>(value.alias.d);
                                        break;
                                    case op_sizes::qword:
                                        casted_value.alias.d = value.alias.f;
                                        break;
                                    default:
                                        // XXX: this is an error
                                        break;
                                }
                                break;
                            }
                            default:
                                break;
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, casted_value))
                    return false;

                break;
            }
            case op_codes::fill: {
                operand_value_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;

                if (!bounds_check_address(r, address))
                    return false;

                operand_value_t value;
                if (!get_operand_value(r, inst, 1, value))
                    return false;

                operand_value_t length;
                if (!get_operand_value(r, inst, 2, length))
                    return false;
                length.alias.u *= op_size_in_bytes(inst.size);

                switch (inst.size) {
                    case op_sizes::byte:
                        memset(
                            reinterpret_cast<void*>(address.alias.u),
                            static_cast<uint8_t>(value.alias.u),
                            length.alias.u);
                        break;
                    case op_sizes::word:
                        memset(
                            reinterpret_cast<void*>(address.alias.u),
                            static_cast<uint16_t>(value.alias.u),
                            length.alias.u);
                        break;
                    case op_sizes::dword:
                        memset(
                            reinterpret_cast<void*>(address.alias.u),
                            static_cast<uint32_t>(value.alias.u),
                            length.alias.u);
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

                break;
            }
            case op_codes::clr: {
                operand_value_t value;
                value.type = register_type_t::integer;
                value.alias.u = 0;

                if (!set_target_operand_value(r, inst.operands[0], inst.size, value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, true);
                _registers.flags(register_file_t::flags_t::negative, false);

                break;
            }
            case op_codes::move: {
                operand_value_t source_value;
                if (!get_operand_value(r, inst, 1, source_value))
                    return false;

                operand_value_t offset_value;
                if (!get_address_with_offset(r, inst, 1, 2, offset_value))
                    return false;

                if (!set_target_operand_value(r, inst.operands[0], inst.size, offset_value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, source_value.alias.u == 0);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(source_value, inst.size));

                break;
            }
            case op_codes::moves: {
                operand_value_t source_value;
                if (!get_operand_value(r, inst, 1, source_value))
                    return false;

                operand_value_t offset_value;
                if (!get_address_with_offset(r, inst, 1, 2, offset_value))
                    return false;

                auto previous_size = static_cast<op_sizes>(static_cast<uint8_t>(inst.size) - 1);
                offset_value.alias.u = common::sign_extend(
                    offset_value.alias.u,
                    static_cast<uint32_t>(op_size_in_bytes(previous_size) * 8));

                if (!set_target_operand_value(r, inst.operands[0], inst.size, offset_value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, source_value.alias.u == 0);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(source_value, inst.size));

                break;
            }
            case op_codes::movez: {
                operand_value_t source_value;
                if (!get_operand_value(r, inst, 1, source_value))
                    return false;

                operand_value_t offset_value;
                if (!get_address_with_offset(r, inst, 1, 2, offset_value))
                    return false;

                switch (inst.size) {
                    case op_sizes::none:
                    case op_sizes::byte: {
                        break;
                    }
                    case op_sizes::word: {
                        offset_value.alias.u &= 0b0000000000000000000000000000000000000000000000000000000011111111;
                        break;
                    }
                    case op_sizes::dword: {
                        offset_value.alias.u &= 0b0000000000000000000000000000000000000000000000001111111111111111;
                        break;
                    }
                    case op_sizes::qword: {
                        offset_value.alias.u &= 0b0000000000000000000000000000000011111111111111111111111111111111;
                        break;
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, offset_value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, source_value.alias.u == 0);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(source_value, inst.size));

                break;
            }
            case op_codes::push: {
                operand_value_t source_value;

                if (!get_operand_value(r, inst, 0, source_value))
                    return false;

                push(source_value.alias.u);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, source_value.alias.u == 0);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(source_value, inst.size));

                break;
            }
            case op_codes::pushm: {
                for (uint8_t i = 0; i < inst.operands_count; i++) {
                    const auto& operand = inst.operands[i];
                    auto type = operand.is_integer() ?
                                register_type_t::integer :
                                register_type_t::floating_point;

                    auto range = static_cast<uint16_t>(operand.value.u);
                    auto start = static_cast<uint8_t>(range & 0xff00);
                    auto end = static_cast<uint8_t>(range & 0x00ff);

                    for (uint8_t reg = start; reg <= end; reg++) {
                        auto reg_index = register_index(static_cast<registers_t>(reg), type);
                        auto alias = _registers.r[reg_index];
                        push(alias.qw);
                    }
                }

                _registers.flags(register_file_t::flags_t::zero, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::negative, false);

                break;
            }
            case op_codes::pop: {
                operand_value_t top_of_stack;
                top_of_stack.alias.u = pop();

                if (!set_target_operand_value(r, inst.operands[0], inst.size, top_of_stack))
                    return false;

                _registers.flags(register_file_t::flags_t::zero, top_of_stack.alias.u == 0);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(top_of_stack, inst.size));

                break;
            }
            case op_codes::popm: {
                for (uint8_t i = 0; i < inst.operands_count; i++) {
                    const auto& operand = inst.operands[i];
                    auto type = operand.is_integer() ?
                                register_type_t::integer :
                                register_type_t::floating_point;

                    auto range = static_cast<uint16_t>(operand.value.u);
                    auto start = static_cast<uint8_t>(range & 0xff00);
                    auto end = static_cast<uint8_t>(range & 0x00ff);

                    for (uint8_t reg = start; reg <= end; reg++) {
                        auto reg_index = register_index(static_cast<registers_t>(reg), type);
                        _registers.r[reg_index].qw = pop();
                    }
                }

                _registers.flags(register_file_t::flags_t::zero, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::negative, false);

                break;
            }
            case op_codes::dup: {
                operand_value_t top_of_stack;
                top_of_stack.alias.u = peek();
                push(top_of_stack.alias.u);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, top_of_stack.alias.u == 0);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(top_of_stack, inst.size));

                break;
            }
            case op_codes::inc: {
                operand_value_t reg_value;
                if (get_operand_value(r, inst, 0, reg_value))
                    return false;

                register_value_alias_t one {};
                one.qw = 1;

                operand_value_t new_value;
                if (reg_value.type == register_type_t::floating_point) {
                    if (inst.size == op_sizes::dword)
                        new_value.alias.f = static_cast<float>(reg_value.alias.f + 1.0);
                    else
                        new_value.alias.d = reg_value.alias.d + 1.0;
                    new_value.type = register_type_t::floating_point;
                }
                else {
                    register_value_alias_t alias {};
                    alias.qw = reg_value.alias.u;

                    new_value.type = register_type_t::integer;
                    switch (inst.size) {
                        case op_sizes::byte: {
                            new_value.alias.u = alias.b + one.b;
                            break;
                        }
                        case op_sizes::word: {
                            new_value.alias.u = alias.w + one.w;
                            break;
                        }
                        case op_sizes::dword: {
                            new_value.alias.u = alias.dw + one.dw;
                            break;
                        }
                        default:
                        case op_sizes::qword: {
                            new_value.alias.u = alias.qw + one.qw;
                            break;
                        }
                    }
                }

                if (_white_listed_addresses.count(reg_value.alias.u) > 0) {
                    _white_listed_addresses.insert(new_value.alias.u);
                }

                if (set_target_operand_value(r, inst.operands[0], inst.size, new_value))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(
                        reg_value.as_register_alias(),
                        one,
                        new_value.as_register_alias(),
                        inst.size));
                _registers.flags(register_file_t::flags_t::zero, new_value.alias.u == 0);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(
                    register_file_t::flags_t::carry,
                    has_carry(reg_value.alias.u, one.qw, inst.size));
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(new_value, inst.size));

                break;
            }
            case op_codes::dec: {
                operand_value_t reg_value;
                if (get_operand_value(r, inst, 0, reg_value))
                    return false;

                register_value_alias_t one {};
                one.qw = 1;

                operand_value_t new_value;
                if (reg_value.type == register_type_t::floating_point) {
                    if (inst.size == op_sizes::dword)
                        new_value.alias.f = static_cast<float>(reg_value.alias.f - 1.0);
                    else
                        new_value.alias.d = reg_value.alias.d - 1.0;
                    new_value.type = register_type_t::floating_point;
                } else {
                    register_value_alias_t alias {};
                    alias.qw = reg_value.alias.u;

                    new_value.type = register_type_t::integer;

                    switch (inst.size) {
                        case op_sizes::byte: {
                            new_value.alias.u = alias.b - one.b;
                            break;
                        }
                        case op_sizes::word: {
                            new_value.alias.u = alias.w - one.w;
                            break;
                        }
                        case op_sizes::dword: {
                            new_value.alias.u = alias.dw - one.dw;
                            break;
                        }
                        default:
                        case op_sizes::qword: {
                            new_value.alias.u = alias.qw - one.qw;
                            break;
                        }
                    }
                }

                if (_white_listed_addresses.count(reg_value.alias.u) > 0) {
                    _white_listed_addresses.insert(new_value.alias.u);
                }

                if (set_target_operand_value(r, inst.operands[0], inst.size, new_value))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(
                        reg_value.as_register_alias(),
                        one,
                        new_value.as_register_alias(),
                        inst.size));
                _registers.flags(register_file_t::flags_t::subtract, true);
                _registers.flags(register_file_t::flags_t::zero, new_value.alias.u == 0);
                _registers.flags(
                    register_file_t::flags_t::carry,
                    has_carry(reg_value.alias.u, one.qw, inst.size));
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(new_value, inst.size));
                break;
            }
            case op_codes::add: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t sum_result;
                if (lhs_value.type == register_type_t::floating_point
                &&  rhs_value.type == register_type_t::floating_point) {
                    sum_result.type = register_type_t::floating_point;
                    if (inst.size == op_sizes::dword)
                        sum_result.alias.f = lhs_value.alias.f + rhs_value.alias.f;
                    else
                        sum_result.alias.d = lhs_value.alias.d + rhs_value.alias.d;
                } else {
                    register_value_alias_t lhs_alias {};
                    lhs_alias.qw = lhs_value.alias.u;

                    register_value_alias_t rhs_alias {};
                    rhs_alias.qw = rhs_value.alias.u;

                    sum_result.type = register_type_t::integer;
                    switch (inst.size) {
                        case op_sizes::byte: {
                            sum_result.alias.u = lhs_alias.b + rhs_alias.b;
                            break;
                        }
                        case op_sizes::word: {
                            sum_result.alias.u = lhs_alias.w + rhs_alias.w;
                            break;
                        }
                        case op_sizes::dword: {
                            sum_result.alias.u = lhs_alias.dw + rhs_alias.dw;
                            break;
                        }
                        default:
                        case op_sizes::qword: {
                            sum_result.alias.u = lhs_alias.qw + rhs_alias.qw;
                            break;
                        }
                    }
                }

                if (_white_listed_addresses.count(lhs_value.alias.u) > 0
                ||  _white_listed_addresses.count(rhs_value.alias.u) > 0) {
                    _white_listed_addresses.insert(sum_result.alias.u);
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, sum_result))
                    return false;

                auto zero_flag = is_zero(inst.size, sum_result);
                auto carry_flag = has_carry(
                    lhs_value.alias.u,
                    rhs_value.alias.u,
                    inst.size);

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(
                        lhs_value.as_register_alias(),
                        rhs_value.as_register_alias(),
                        sum_result.as_register_alias(),
                        inst.size));
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(register_file_t::flags_t::carry, carry_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(sum_result, inst.size));

                break;
            }
            case op_codes::sub: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                auto carry_flag = false;

                operand_value_t subtraction_result;
                if (lhs_value.type == register_type_t::floating_point
                &&  rhs_value.type == register_type_t::floating_point) {
                    if (inst.size == op_sizes::dword)
                        subtraction_result.alias.f = lhs_value.alias.f - rhs_value.alias.f;
                    else
                        subtraction_result.alias.d = lhs_value.alias.d - rhs_value.alias.d;
                    subtraction_result.type = register_type_t::floating_point;
                } else {
                    subtraction_result.type = register_type_t::integer;

                    register_value_alias_t lhs {};
                    lhs.qw = lhs_value.alias.u;

                    register_value_alias_t rhs {};
                    rhs.qw = rhs_value.alias.u;

                    switch (inst.size) {
                        case op_sizes::byte:
                            carry_flag = lhs.b < rhs.b;
                            subtraction_result.alias.u = lhs.b - rhs.b;
                            break;
                        case op_sizes::word:
                            carry_flag = lhs.w < rhs.w;
                            subtraction_result.alias.u = lhs.w - rhs.w;
                            break;
                        case op_sizes::dword:
                            carry_flag = lhs.dw < rhs.dw;
                            subtraction_result.alias.u = lhs.dw - rhs.dw;
                            break;
                        case op_sizes::qword:
                            carry_flag = lhs.qw < rhs.qw;
                            subtraction_result.alias.u = lhs.qw - rhs.qw;
                            break;
                        default:
                            return false;
                    }
                }

                if (_white_listed_addresses.count(lhs_value.alias.u) > 0
                ||  _white_listed_addresses.count(rhs_value.alias.u) > 0) {
                    _white_listed_addresses.insert(subtraction_result.alias.u);
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, subtraction_result))
                    return false;

                auto zero_flag = is_zero(inst.size, subtraction_result);
                auto overflow_flag = has_overflow(
                    lhs_value.as_register_alias(),
                    rhs_value.as_register_alias(),
                    subtraction_result.as_register_alias(),
                    inst.size);

                _registers.flags(register_file_t::flags_t::subtract, true);
                _registers.flags(register_file_t::flags_t::carry, carry_flag);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::overflow,
                    !carry_flag && overflow_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(subtraction_result, inst.size));
                break;
            }
            case op_codes::mul: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t product_result;
                if (lhs_value.type == register_type_t::floating_point
                &&  rhs_value.type == register_type_t::floating_point) {
                    product_result.type = register_type_t::floating_point;
                    if (inst.size == op_sizes::dword)
                        product_result.alias.f = lhs_value.alias.f * rhs_value.alias.f;
                    else
                        product_result.alias.d = lhs_value.alias.d * rhs_value.alias.d;
                }
                else {
                    register_value_alias_t lhs_alias {};
                    lhs_alias.qw = lhs_value.alias.u;

                    register_value_alias_t rhs_alias {};
                    rhs_alias.qw = rhs_value.alias.u;

                    product_result.type = register_type_t::integer;
                    switch (inst.size) {
                        case op_sizes::byte: {
                            product_result.alias.u = lhs_alias.b * rhs_alias.b;
                            break;
                        }
                        case op_sizes::word: {
                            product_result.alias.u = lhs_alias.w * rhs_alias.w;
                            break;
                        }
                        case op_sizes::dword: {
                            product_result.alias.u = lhs_alias.dw * rhs_alias.dw;
                            break;
                        }
                        default:
                        case op_sizes::qword: {
                            product_result.alias.u = lhs_alias.qw * rhs_alias.qw;
                            break;
                        }
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, product_result))
                    return false;

                auto zero_flag = is_zero(inst.size, product_result);
                auto carry_flag = has_carry(
                    lhs_value.alias.u,
                    rhs_value.alias.u,
                    inst.size);

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(
                        lhs_value.as_register_alias(),
                        rhs_value.as_register_alias(),
                        product_result.as_register_alias(),
                        inst.size));
                _registers.flags(register_file_t::flags_t::carry, carry_flag);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(product_result, inst.size));
                break;
            }
            case op_codes::div: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t result;
                result.alias.u = 0;

                if (lhs_value.type == register_type_t::floating_point
                &&  rhs_value.type == register_type_t::floating_point) {
                    result.type = register_type_t::floating_point;
                    if (inst.size == op_sizes::dword) {
                        if (rhs_value.alias.f != 0) {
                            result.alias.f = lhs_value.alias.f / rhs_value.alias.f;
                        }
                    } else {
                        if (rhs_value.alias.d != 0) {
                            result.alias.d = lhs_value.alias.d / rhs_value.alias.d;
                        }
                    }
                } else {
                    result.type = register_type_t::integer;
                    register_value_alias_t lhs_alias{};
                    lhs_alias.qw = lhs_value.alias.u;

                    register_value_alias_t rhs_alias{};
                    rhs_alias.qw = rhs_value.alias.u;

                    switch (inst.size) {
                        case op_sizes::byte: {
                            if (rhs_alias.b != 0)
                                result.alias.u = lhs_alias.b / rhs_alias.b;
                            break;
                        }
                        case op_sizes::word: {
                            if (rhs_alias.w != 0)
                            result.alias.u = lhs_alias.w / rhs_alias.w;
                            break;
                        }
                        case op_sizes::dword: {
                            if (rhs_alias.dw != 0)
                                result.alias.u = lhs_alias.dw / rhs_alias.dw;
                            break;
                        }
                        default:
                        case op_sizes::qword: {
                            if (rhs_alias.qw != 0)
                                result.alias.u = lhs_alias.qw / rhs_alias.qw;
                            break;
                        }
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                auto zero_flag = is_zero(inst.size, result);
                auto carry_flag = has_carry(
                    lhs_value.alias.u,
                    rhs_value.alias.u,
                    inst.size);
                auto overflow_flag = has_overflow(
                    lhs_value.as_register_alias(),
                    rhs_value.as_register_alias(),
                    result.as_register_alias(),
                    inst.size);

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    overflow_flag);
                _registers.flags(register_file_t::flags_t::carry, carry_flag);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::mod: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t result;
                result.alias.u = 0;

                register_value_alias_t lhs_alias {};
                lhs_alias.qw = lhs_value.alias.u;

                register_value_alias_t rhs_alias {};
                rhs_alias.qw = rhs_value.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        if (lhs_alias.b != 0 && rhs_alias.b != 0)
                            result.alias.u = lhs_alias.b % rhs_alias.b;
                        break;
                    }
                    case op_sizes::word: {
                        if (lhs_alias.w != 0 && rhs_alias.w != 0)
                            result.alias.u = lhs_alias.w % rhs_alias.w;
                        break;
                    }
                    case op_sizes::dword: {
                        if (lhs_alias.dw != 0 && rhs_alias.dw != 0)
                            result.alias.u = lhs_alias.dw % rhs_alias.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        if (lhs_alias.qw != 0 && rhs_alias.qw != 0)
                            result.alias.u = lhs_alias.qw % rhs_alias.qw;
                        break;
                    }
                }

                auto zero_flag = is_zero(inst.size, result);
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(
                        lhs_value.as_register_alias(),
                        rhs_value.as_register_alias(),
                        result.as_register_alias(),
                        inst.size));
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::neg: {
                operand_value_t value;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                operand_value_t result;
                if (value.type == register_type_t::floating_point) {
                    if (inst.size == op_sizes::dword)
                        result.alias.f = static_cast<float>(value.alias.f * -1.0);
                    else
                        result.alias.d = value.alias.d * -1.0;
                    result.type = register_type_t::floating_point;
                } else {
                    result.type = register_type_t::integer;

                    register_value_alias_t alias {};
                    alias.qw = value.alias.u;

                    switch (inst.size) {
                        case op_sizes::byte: {
                            int8_t negated_result = -static_cast<int8_t>(alias.b);
                            result.alias.u = static_cast<uint64_t>(negated_result);
                            break;
                        }
                        case op_sizes::word: {
                            int16_t negated_result = -static_cast<int16_t>(alias.w);
                            result.alias.u = static_cast<uint64_t>(negated_result);
                            break;
                        }
                        case op_sizes::dword: {
                            int32_t negated_result = -static_cast<int32_t>(alias.dw);
                            result.alias.u = static_cast<uint64_t>(negated_result);
                            break;
                        }
                        default:
                        case op_sizes::qword: {
                            int64_t negated_result = -static_cast<int64_t>(alias.qw);
                            result.alias.u = static_cast<uint64_t>(negated_result);
                            break;
                        }
                    }
                }

                auto zero_flag = is_zero(inst.size, result);
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::shr: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t result;
                register_value_alias_t lhs_alias {};
                lhs_alias.qw = lhs_value.alias.u;

                register_value_alias_t rhs_alias {};
                rhs_alias.qw = rhs_value.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        result.alias.u = lhs_alias.b >> rhs_alias.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.alias.u = lhs_alias.w >> rhs_alias.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.alias.u = lhs_alias.dw >> rhs_alias.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.alias.u = lhs_alias.qw >> rhs_alias.qw;
                        break;
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                auto zero_flag = is_zero(inst.size, result);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));
                break;
            }
            case op_codes::shl: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t result;
                register_value_alias_t lhs_alias {};
                lhs_alias.qw = lhs_value.alias.u;

                register_value_alias_t rhs_alias {};
                rhs_alias.qw = rhs_value.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        result.alias.u = lhs_alias.b << rhs_alias.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.alias.u = lhs_alias.w << rhs_alias.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.alias.u = lhs_alias.dw << rhs_alias.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.alias.u = lhs_alias.qw << rhs_alias.qw;
                        break;
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                auto zero_flag = is_zero(inst.size, result);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::ror: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t right_rotated_value;
                right_rotated_value.alias.u = common::rotr(
                    lhs_value.alias.u,
                    static_cast<uint8_t>(rhs_value.alias.u));
                if (!set_target_operand_value(r, inst.operands[0], inst.size, right_rotated_value))
                    return false;

                auto zero_flag = is_zero(inst.size, right_rotated_value);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(right_rotated_value, inst.size));

                break;
            }
            case op_codes::rol: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t left_rotated_value;
                left_rotated_value.alias.u = common::rotl(
                    lhs_value.alias.u,
                    static_cast<uint8_t>(rhs_value.alias.u));
                if (!set_target_operand_value(r, inst.operands[0], inst.size, left_rotated_value))
                    return false;

                auto zero_flag = is_zero(inst.size, left_rotated_value);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(left_rotated_value, inst.size));

                break;
            }
            case op_codes::pow: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t power_value;
                power_value.alias.u = 0;

                if (lhs_value.type == register_type_t::floating_point
                &&  rhs_value.type == register_type_t::floating_point) {
                    power_value.type = register_type_t::floating_point;
                    if (inst.size == op_sizes::dword) {
                        power_value.alias.f = std::pow(
                            lhs_value.alias.f,
                            rhs_value.alias.f);
                    } else {
                        power_value.alias.d = std::pow(
                            lhs_value.alias.d,
                            rhs_value.alias.d);
                    }
                } else {
                    power_value.type = register_type_t::integer;

                    register_value_alias_t lhs_alias {};
                    lhs_alias.qw = lhs_value.alias.u;

                    register_value_alias_t rhs_alias {};
                    rhs_alias.qw = rhs_value.alias.u;

                    switch (inst.size) {
                        case op_sizes::byte: {
                            power_value.alias.u = common::power(
                                lhs_alias.b,
                                rhs_alias.b);
                            break;
                        }
                        case op_sizes::word: {
                            power_value.alias.u = common::power(
                                lhs_alias.w,
                                rhs_alias.w);
                            break;
                        }
                        case op_sizes::dword: {
                            power_value.alias.u = common::power(
                                lhs_alias.dw,
                                rhs_alias.dw);
                            break;
                        }
                        default:
                        case op_sizes::qword: {
                            power_value.alias.u = common::power(
                                lhs_alias.qw,
                                rhs_alias.qw);
                            break;
                        }
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, power_value))
                    return false;

                auto zero_flag = is_zero(inst.size, power_value);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(power_value, inst.size));

                break;
            }
            case op_codes::and_op: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t result;
                result.type = register_type_t::integer;

                register_value_alias_t lhs_alias {};
                lhs_alias.qw = lhs_value.alias.u;

                register_value_alias_t rhs_alias {};
                rhs_alias.qw = rhs_value.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        result.alias.u = lhs_alias.b & rhs_alias.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.alias.u = lhs_alias.w & rhs_alias.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.alias.u = lhs_alias.dw & rhs_alias.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.alias.u = lhs_alias.qw & rhs_alias.qw;
                        break;
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                auto zero_flag = is_zero(inst.size, result);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::or_op: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t result;
                result.type = register_type_t::integer;

                register_value_alias_t lhs_alias {};
                lhs_alias.qw = lhs_value.alias.u;

                register_value_alias_t rhs_alias {};
                rhs_alias.qw = rhs_value.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        result.alias.u = lhs_alias.b | rhs_alias.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.alias.u = lhs_alias.w | rhs_alias.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.alias.u = lhs_alias.dw | rhs_alias.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.alias.u = lhs_alias.qw | rhs_alias.qw;
                        break;
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                auto zero_flag = is_zero(inst.size, result);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::xor_op: {
                operand_value_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                operand_value_t result;
                result.type = register_type_t::integer;

                register_value_alias_t lhs_alias {};
                lhs_alias.qw = lhs_value.alias.u;

                register_value_alias_t rhs_alias {};
                rhs_alias.qw = rhs_value.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        result.alias.u = lhs_alias.b ^ rhs_alias.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.alias.u = lhs_alias.w ^ rhs_alias.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.alias.u = lhs_alias.dw ^ rhs_alias.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.alias.u = lhs_alias.qw ^ rhs_alias.qw;
                        break;
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                auto zero_flag = is_zero(inst.size, result);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::not_op: {
                operand_value_t value;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                operand_value_t result;
                result.type = register_type_t::integer;

                register_value_alias_t alias {};
                alias.qw = value.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        result.alias.u = ~alias.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.alias.u = ~alias.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.alias.u = ~alias.dw;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.alias.u = ~alias.qw;
                        break;
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                auto zero_flag = is_zero(inst.size, result);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::bis: {
                operand_value_t value, bit_number;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                if (!get_operand_value(r, inst, 2, bit_number))
                    return false;

                operand_value_t result;
                result.type = register_type_t::integer;

                register_value_alias_t value_alias {};
                value_alias.qw = value.alias.u;

                register_value_alias_t bit_alias {};
                bit_alias.qw = bit_number.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        auto masked_value = static_cast<uint8_t>(1) << bit_alias.b;
                        result.alias.u = value_alias.b | masked_value;
                        break;
                    }
                    case op_sizes::word: {
                        auto masked_value = static_cast<uint16_t>(1) << bit_alias.w;
                        result.alias.u = value_alias.w | masked_value;
                        break;
                    }
                    case op_sizes::dword: {
                        auto masked_value = static_cast<uint32_t>(1) << bit_alias.dw;
                        result.alias.u = value_alias.dw | masked_value;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        auto masked_value = static_cast<uint64_t>(1) << bit_alias.qw;
                        result.alias.u = value_alias.qw | masked_value;
                        break;
                    }
                }

                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                auto zero_flag = is_zero(inst.size, result);

                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::bic: {
                operand_value_t value, bit_number;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                if (!get_operand_value(r, inst, 2, bit_number))
                    return false;

                operand_value_t result;
                result.type = register_type_t::integer;

                register_value_alias_t value_alias {};
                value_alias.qw = value.alias.u;

                register_value_alias_t bit_alias {};
                bit_alias.qw = bit_number.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        auto masked_value = ~(static_cast<uint8_t>(1) << bit_alias.b);
                        result.alias.u = value_alias.b & masked_value;
                        break;
                    }
                    case op_sizes::word: {
                        auto masked_value = ~(static_cast<uint16_t>(1) << bit_alias.w);
                        result.alias.u = value_alias.w & masked_value;
                        break;
                    }
                    case op_sizes::dword: {
                        auto masked_value = ~(static_cast<uint32_t>(1) << bit_alias.dw);
                        result.alias.u = value_alias.dw & masked_value;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        auto masked_value = ~(static_cast<uint64_t>(1) << bit_alias.qw);
                        result.alias.u = value_alias.qw & masked_value;
                        break;
                    }
                }

                auto zero_flag = is_zero(inst.size, result);
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::test: {
                operand_value_t value, mask;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, mask))
                    return false;

                operand_value_t result;

                register_value_alias_t value_alias {};
                value_alias.qw = value.alias.u;

                register_value_alias_t mask_alias {};
                mask_alias.qw = mask.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        result.alias.u = value_alias.b & mask_alias.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.alias.u = value_alias.w & mask_alias.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.alias.u = value_alias.w & mask_alias.w;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.alias.u = value_alias.w & mask_alias.w;
                        break;
                    }
                }

                auto zero_flag = is_zero(inst.size, result);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::cmp: {
                operand_value_t lhs_value, rhs_value;
                lhs_value.alias.u = 0;
                rhs_value.alias.u = 0;

                if (!get_operand_value(r, inst, 0, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 1, rhs_value))
                    return false;

                register_value_alias_t lhs {};
                lhs.qw = lhs_value.alias.u;

                register_value_alias_t rhs {};
                rhs.qw = rhs_value.alias.u;

                operand_value_t result;
                bool carry_flag;

                switch (inst.size) {
                    case op_sizes::byte:
                        carry_flag = lhs.b < rhs.b;
                        result.alias.u = lhs.b - rhs.b;
                        break;
                    case op_sizes::word:
                        carry_flag = lhs.w < rhs.w;
                        result.alias.u = lhs.w - rhs.w;
                        break;
                    case op_sizes::dword:
                        carry_flag = lhs.dw < rhs.dw;
                        result.alias.u = lhs.dw - rhs.dw;
                        break;
                    case op_sizes::qword:
                        carry_flag = lhs.qw < rhs.qw;
                        result.alias.u = lhs.qw - rhs.qw;
                        break;
                    default:
                        return false;
                }

                auto zero_flag = is_zero(inst.size, result);
                auto overflow_flag = has_overflow(
                    lhs_value.as_register_alias(),
                    rhs_value.as_register_alias(),
                    result.as_register_alias(),
                    inst.size);

                _registers.flags(register_file_t::flags_t::subtract, true);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(register_file_t::flags_t::carry, carry_flag);
                _registers.flags(
                    register_file_t::flags_t::overflow,
                    !carry_flag && overflow_flag);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(result, inst.size));

                break;
            }
            case op_codes::bz: {
                operand_value_t value, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, address))
                    return false;

                auto zero_flag = is_zero(inst.size, value);
                if (zero_flag)
                    _registers.r[register_pc].qw = address.alias.u;

                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(value, inst.size));

                break;
            }
            case op_codes::bnz: {
                operand_value_t value, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, address))
                    return false;

                auto zero_flag = is_zero(inst.size, value);
                if (!zero_flag)
                    _registers.r[register_pc].qw = address.alias.u;

                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(
                    register_file_t::flags_t::negative,
                    is_negative(value, inst.size));

                break;
            }
            case op_codes::tbz: {
                operand_value_t value, mask, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, mask))
                    return false;

                if (!get_operand_value(r, inst, 2, address))
                    return false;

                operand_value_t result;
                result.type = register_type_t::integer;

                register_value_alias_t value_alias {};
                value_alias.qw = value.alias.u;

                register_value_alias_t mask_alias {};
                mask_alias.qw = mask.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        result.alias.u = value_alias.b & mask_alias.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.alias.u = value_alias.w & mask_alias.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.alias.u = value_alias.w & mask_alias.w;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.alias.u = value_alias.w & mask_alias.w;
                        break;
                    }
                }

                auto zero_flag = is_zero(inst.size, result);
                if (zero_flag)
                    _registers.r[register_pc].qw = address.alias.u;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::tbnz: {
                operand_value_t value, mask, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, mask))
                    return false;

                if (!get_operand_value(r, inst, 2, address))
                    return false;

                operand_value_t result;
                result.type = register_type_t::integer;

                register_value_alias_t value_alias {};
                value_alias.qw = value.alias.u;

                register_value_alias_t mask_alias {};
                mask_alias.qw = mask.alias.u;

                switch (inst.size) {
                    case op_sizes::byte: {
                        result.alias.u = value_alias.b & mask_alias.b;
                        break;
                    }
                    case op_sizes::word: {
                        result.alias.u = value_alias.w & mask_alias.w;
                        break;
                    }
                    case op_sizes::dword: {
                        result.alias.u = value_alias.w & mask_alias.w;
                        break;
                    }
                    default:
                    case op_sizes::qword: {
                        result.alias.u = value_alias.w & mask_alias.w;
                        break;
                    }
                }

                auto zero_flag = is_zero(inst.size, result);
                if (!zero_flag)
                    _registers.r[register_pc].qw = address.alias.u;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::bne: {
                // ZF = 0
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (!_registers.flags(register_file_t::flags_t::zero)) {
                    _registers.r[register_pc].qw = address.alias.u;
                }

                break;
            }
            case op_codes::beq: {
                // ZF = 1
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (_registers.flags(register_file_t::flags_t::zero)) {
                    _registers.r[register_pc].qw = address.alias.u;
                }

                break;
            }
            case op_codes::bs: {
                // SF = 1
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (_registers.flags(register_file_t::flags_t::negative)) {
                    _registers.r[register_pc].qw = address.alias.u;
                }

                break;
            }
            case op_codes::bo: {
                // OF = 1
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (_registers.flags(register_file_t::flags_t::overflow)) {
                    _registers.r[register_pc].qw = address.alias.u;
                }

                break;
            }
            case op_codes::ba: {
                // CF = 0 and ZF = 0
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (!_registers.flags(register_file_t::flags_t::zero)
                &&  !_registers.flags(register_file_t::flags_t::carry)) {
                    _registers.r[register_pc].qw = address.alias.u;
                }
                break;
            }
            case op_codes::bg: {
                // ZF = 0 and SF = OF
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                auto sf = _registers.flags(register_file_t::flags_t::negative);
                auto of = _registers.flags(register_file_t::flags_t::overflow);
                auto zf = _registers.flags(register_file_t::flags_t::zero);

                if (!zf && sf == of) {
                    _registers.r[register_pc].qw = address.alias.u;
                }

                break;
            }
            case op_codes::bge: {
                // SF = OF
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                auto sf = _registers.flags(register_file_t::flags_t::negative);
                auto of = _registers.flags(register_file_t::flags_t::overflow);

                if (sf == of) {
                    _registers.r[register_pc].qw = address.alias.u;
                }

                break;
            }
            case op_codes::bcc: {
                // CF = 0
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (!_registers.flags(register_file_t::flags_t::carry)) {
                    _registers.r[register_pc].qw = address.alias.u;
                }

                break;
            }
            case op_codes::bae:
            case op_codes::bcs:
            case op_codes::bb: {
                // CF = 1
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (_registers.flags(register_file_t::flags_t::carry)) {
                    _registers.r[register_pc].qw = address.alias.u;
                }

                break;
            }
            case op_codes::bbe: {
                // CF = 1 or ZF = 1
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                if (_registers.flags(register_file_t::flags_t::carry)
                ||  _registers.flags(register_file_t::flags_t::zero)) {
                    _registers.r[register_pc].qw = address.alias.u;
                }

                break;
            }
            case op_codes::bl: {
                // SF <> OF
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                auto sf = _registers.flags(register_file_t::flags_t::negative);
                auto of = _registers.flags(register_file_t::flags_t::overflow);

                if (sf != of) {
                    _registers.r[register_pc].qw = address.alias.u;
                }

                break;
            }
            case op_codes::ble: {
                // ZF = 1 or SF <> OF
                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                auto sf = _registers.flags(register_file_t::flags_t::negative);
                auto of = _registers.flags(register_file_t::flags_t::overflow);
                auto zf = _registers.flags(register_file_t::flags_t::zero);

                if (zf || sf != of) {
                    _registers.r[register_pc].qw = address.alias.u;
                }
                break;
            }
            case op_codes::setb:
            case op_codes::setnae:
            case op_codes::setc: {
                // CF = 1
                operand_value_t result;
                result.alias.u = _registers.flags(register_file_t::flags_t::carry) ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::setnb:
            case op_codes::setae:
            case op_codes::setnc: {
                // CF = 0
                operand_value_t result;
                result.alias.u = !_registers.flags(register_file_t::flags_t::carry) ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::seta:
            case op_codes::setnbe: {
                // CF = 0 and ZF = 0
                operand_value_t result;
                auto zero_flag = _registers.flags(register_file_t::flags_t::zero);
                auto carry_flag = _registers.flags(register_file_t::flags_t::carry);
                result.alias.u = !zero_flag && !carry_flag ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::setna:
            case op_codes::setbe: {
                // CF = 1 or ZF = 1
                operand_value_t result;
                auto zero_flag = _registers.flags(register_file_t::flags_t::zero);
                auto carry_flag = _registers.flags(register_file_t::flags_t::carry);
                result.alias.u = (zero_flag || carry_flag) ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::setg:
            case op_codes::setnle: {
                // ZF = 0 and SF = OF
                auto sf = _registers.flags(register_file_t::flags_t::negative);
                auto of = _registers.flags(register_file_t::flags_t::overflow);
                auto zf = _registers.flags(register_file_t::flags_t::zero);
                operand_value_t result;
                result.alias.u = !zf && sf == of ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::setnl:
            case op_codes::setge: {
                // SF = OF
                auto sf = _registers.flags(register_file_t::flags_t::negative);
                auto of = _registers.flags(register_file_t::flags_t::overflow);
                operand_value_t result;
                result.alias.u = sf == of ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::setle:
            case op_codes::setng: {
                // ZF = 1 or SF != OF
                auto sf = _registers.flags(register_file_t::flags_t::negative);
                auto of = _registers.flags(register_file_t::flags_t::overflow);
                auto zf = _registers.flags(register_file_t::flags_t::zero);
                operand_value_t result;
                result.alias.u = zf || sf != of ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::setl:
            case op_codes::setnge: {
                // SF != OF
                auto sf = _registers.flags(register_file_t::flags_t::negative);
                auto of = _registers.flags(register_file_t::flags_t::overflow);
                operand_value_t result;
                result.alias.u = sf != of ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::seto: {
                // OF = 1
                operand_value_t result;
                result.alias.u = _registers.flags(register_file_t::flags_t::overflow) ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::setno: {
                // OF = 0
                operand_value_t result;
                result.alias.u = !_registers.flags(register_file_t::flags_t::overflow) ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::sets: {
                // SF = 1
                operand_value_t result;
                result.alias.u = _registers.flags(register_file_t::flags_t::negative) ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::setns: {
                // SF = 0
                operand_value_t result;
                result.alias.u = !_registers.flags(register_file_t::flags_t::negative) ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::setz: {
                operand_value_t result;
                result.alias.u = _registers.flags(register_file_t::flags_t::zero) ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::setnz: {
                operand_value_t result;
                result.alias.u = !_registers.flags(register_file_t::flags_t::zero) ? 1 : 0;
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;
                break;
            }
            case op_codes::jsr: {
                auto pc = _registers.r[register_pc].qw;
                push(pc);

                operand_value_t address;
                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                _registers.r[register_pc].qw = address.alias.u;
                break;
            }
            case op_codes::rts: {
                auto address = pop();
                _registers.r[register_pc].qw = address;
                break;
            }
            case op_codes::jmp: {
                operand_value_t address;

                auto result = get_constant_address_or_pc_with_offset(
                    r,
                    inst,
                    0,
                    inst_size,
                    address);
                if (!result)
                    return false;

                _registers.r[register_pc].qw = address.alias.u;
                break;
            }
            case op_codes::swi: {
                operand_value_t index;

                if (!get_operand_value(r, inst, 0, index))
                    return false;

                size_t swi_offset = sizeof(uint64_t) * index.alias.u;
                uint64_t swi_address = read(op_sizes::qword, swi_offset);
                if (swi_address != 0) {
                    // XXX: what state should we save and restore here?
                    push(_registers.r[register_pc].qw);
                    _registers.r[register_pc].qw = swi_address;
                }

                break;
            }
            case op_codes::swap: {
                operand_value_t value;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                operand_value_t result;
                result.alias.u = 0;

                switch (inst.size) {
                    case op_sizes::byte: {
                        auto byte_value = static_cast<uint8_t>(value.alias.u);
                        uint8_t upper_nybble = common::get_upper_nybble(byte_value);
                        uint8_t lower_nybble = common::get_lower_nybble(byte_value);
                        byte_value = common::set_upper_nybble(byte_value, lower_nybble);
                        result.alias.u = common::set_lower_nybble(byte_value, upper_nybble);
                        break;
                    }
                    case op_sizes::word:
                        result.alias.u = common::endian_swap_word(static_cast<uint16_t>(value.alias.u));
                        break;
                    case op_sizes::dword:
                        result.alias.u = common::endian_swap_dword(static_cast<uint32_t>(value.alias.u));
                        break;
                    case op_sizes::qword:
                    default:
                        result.alias.u = common::endian_swap_qword(value.alias.u);
                        break;
                }

                auto zero_flag = is_zero(inst.size, result);
                if (!set_target_operand_value(r, inst.operands[0], inst.size, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, zero_flag);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::trap: {
                operand_value_t index;

                if (!get_operand_value(r, inst, 0, index))
                    return false;

                execute_trap(static_cast<uint8_t>(index.alias.u));

                break;
            }
            case op_codes::ffi: {
                operand_value_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;

                operand_value_t signature_id {};
                if (inst.operands_count > 1) {
                    if (!get_operand_value(r, inst, 1, signature_id))
                        return false;
                }

                auto func = _ffi->find_function(address.alias.u);
                if (func == nullptr) {
                    execute_trap(trap_invalid_ffi_call);
                    return false;
                }

                _ffi->reset();
                _ffi->calling_convention(func->calling_mode);

                vm::function_value_list_t* arguments = nullptr;
                if (func->is_variadic()) {
                    auto it = func->call_site_arguments.find(
                        static_cast<common::id_t>(signature_id.alias.u));
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
                    if (func->return_value.type == ffi_types_t::pointer_type)
                        _white_listed_addresses.insert(result_value);
                }

                break;
            }
            case op_codes::meta: {
                operand_value_t meta_data_size;

                if (!get_operand_value(r, inst, 0, meta_data_size))
                    return false;

                break;
            }
            case op_codes::exit: {
                _exited = true;
                break;
            }
        }

        return !r.is_failed();
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
            const operand_value_t& address) {
        if (_white_listed_addresses.count(address.alias.u))
            return true;

        auto heap_bottom = _heap_address;
        auto heap_top = _heap_address + _heap_size;

        if (address.alias.u < heap_bottom
        ||  address.alias.u > heap_top) {
            execute_trap(trap_invalid_address);
            r.error(
                "B004",
                fmt::format(
                    "invalid address: ${:016X}; bottom: ${:016X}; top: ${:016X}",
                    address.alias.u,
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
            auto result = step(r);
            // XXX: did an RTS just execute?
            //      does _registers.pc == return_address?  if so, we're done
            if (!result) {
                break;
            }
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
        uint8_t* heap_ptr = reinterpret_cast<uint8_t*>(address);
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
            instruction_t inst;
            auto inst_size = _icache.fetch_at(r, address, inst);
            if (inst_size == 0)
                break;

            stream << fmt::format("${:016X}: ", address)
                   << inst.disassemble()
                   << fmt::format(" (${:02X} bytes)\n", inst_size);

            if (inst.op == op_codes::exit)
                break;

            address += inst_size;
        }
        return stream.str();
    }

    bool terp::is_zero(
            op_sizes size,
            const operand_value_t& value) {
        auto alias = value.as_register_alias();

        switch (size) {
            case op_sizes::byte:
                return alias.b == 0;
            case op_sizes::word:
                return alias.w == 0;
            case op_sizes::dword:
                return alias.dw == 0;
            case op_sizes::qword:
                return alias.qw == 0;
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

    bool terp::get_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            operand_value_t& value) const {
        register_value_alias_t alias {};
        auto size = op_sizes::qword;

        auto& operand = inst.operands[operand_index];
        value.type = operand.is_integer() ?
            register_type_t::integer :
            register_type_t::floating_point;
        if (operand.is_reg()) {
            auto reg_index = register_index(
                static_cast<registers_t>(operand.value.r),
                value.type);
            alias = _registers.r[reg_index];
        } else {
            alias.qw = operand.value.u;
            size = operand.size;
        }

        switch (size) {
            case op_sizes::byte: {
                value.alias.u = alias.b;
                break;
            }
            case op_sizes::word: {
                value.alias.u = alias.w;
                break;
            }
            case op_sizes::dword: {
                value.alias.u = alias.dw;
                break;
            }
            case op_sizes::qword: {
                value.alias.u = alias.qw;
                break;
            }
            default:
                return false;
        }

        return true;
    }

    bool terp::set_target_operand_value(
            common::result& r,
            const operand_encoding_t& operand,
            op_sizes size,
            const operand_value_t& value) {
        auto type = operand.is_integer() ?
            register_type_t::integer :
            register_type_t::floating_point;
        if (operand.is_reg()) {
            auto reg_index = register_index(
                static_cast<registers_t>(operand.value.r),
                type);
            set_zoned_value(
                type,
                _registers.r[reg_index],
                value.alias.u,
                size);
        } else {
            r.error(
                "B006",
                "constant cannot be a target operand type.");
            return false;
        }

        return true;
    }

    bool terp::get_address_with_offset(
            common::result& r,
            const instruction_t& inst,
            uint8_t address_index,
            uint8_t offset_index,
            operand_value_t& address) {
        if (!get_operand_value(r, inst, address_index, address))
            return false;

        auto is_white_listed = _white_listed_addresses.count(address.alias.u) > 0;

        if (inst.operands_count > 2) {
            operand_value_t offset;

            if (!get_operand_value(r, inst, offset_index, offset))
                return false;

            if (inst.operands[offset_index].is_negative()) {
                address.alias.u -= offset.alias.u;
            } else {
                address.alias.u += offset.alias.u;
            }

            if(inst.op == op_codes::move
            || inst.op == op_codes::moves
            || inst.op == op_codes::movez) {
                if (is_white_listed)
                    _white_listed_addresses.insert(address.alias.u);
            }
        }

        if (!is_white_listed
        && (inst.op == op_codes::load || inst.op == op_codes::store)) {
            if (!bounds_check_address(r, address))
                return false;
        }

        return true;
    }

    bool terp::get_constant_address_or_pc_with_offset(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t inst_size,
            operand_value_t& address) {
        if (!get_operand_value(r, inst, operand_index, address))
            return false;

        if (inst.operands_count >= 2) {
            operand_value_t offset;

            auto offset_index = static_cast<uint8_t>(operand_index + 1);
            if (!get_operand_value(r, inst, offset_index, offset))
                return false;

            if (inst.operands[offset_index].is_negative()) {
                address.alias.u -= offset.alias.u + inst_size;
            } else {
                address.alias.u += offset.alias.u - inst_size;
            }
        }

        return true;
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

    bool terp::is_negative(const operand_value_t& value, op_sizes size) {
        switch (size) {
            case op_sizes::byte: {
                return (value.alias.u & mask_byte_negative) != 0;
            }
            case op_sizes::word: {
                return (value.alias.u & mask_word_negative) != 0;
            }
            case op_sizes::dword: {
                return (value.alias.u & mask_dword_negative) != 0;
            }
            case op_sizes::qword:
            default:
                return (value.alias.u & mask_qword_negative) != 0;
        }
    }

    // XXX: need to add support for both big and little endian
    void terp::write(op_sizes size, uint64_t address, uint64_t value) {
        uint8_t* heap_ptr = reinterpret_cast<uint8_t*>(address);
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

    void terp::set_zoned_value(
            register_type_t type,
            register_value_alias_t& reg,
            uint64_t value,
            op_sizes size) {
        switch (size) {
            case op_sizes::byte: {
                reg.b = static_cast<uint8_t>(value);
                break;
            }
            case op_sizes::word: {
                reg.w = static_cast<uint16_t>(value);
                break;
            }
            case op_sizes::dword: {
                reg.dw = static_cast<uint32_t>(value);
                break;
            }
            default:
            case op_sizes::qword: {
                reg.qw = value;
                break;
            }
        }
    }

};