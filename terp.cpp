#include <sstream>
#include <iomanip>
#include <fmt/format.h>
#include "terp.h"
#include "hex_formatter.h"
#include "bytes.h"

namespace basecode {

    static inline uint64_t rotl(uint64_t n, uint8_t c) {
        const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);
        c &= mask;
        return (n << c) | (n >> ((-c) & mask));
    }

    static inline uint64_t rotr(uint64_t n, uint8_t c) {
        const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);
        c &= mask;
        return (n >> c) | (n << ((-c) & mask));
    }

    ///////////////////////////////////////////////////////////////////////////

    size_t instruction_t::encoding_size() const {
        size_t encoding_size = base_size;

        for (size_t i = 0; i < operands_count; i++) {
            encoding_size += 1;

            if ((operands[i].is_reg())) {
                encoding_size += sizeof(uint8_t);
            } else {
                switch (size) {
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

    size_t instruction_t::decode(result& r, uint8_t* heap, uint64_t address) {
        if (address % alignment != 0) {
            r.add_message(
                "B003",
                fmt::format("instruction alignment violation: alignment = {} bytes, address = ${:016X}",
                            alignment,
                            address),
                true);
            return 0;
        }

        uint8_t* encoding_ptr = heap + address;
        uint8_t encoding_size = *encoding_ptr;
        op = static_cast<op_codes>(*(encoding_ptr + 1));
        uint8_t op_size_and_operands_count = static_cast<uint8_t>(*(encoding_ptr + 2));
        size = static_cast<op_sizes>(get_upper_nybble(op_size_and_operands_count));
        operands_count = get_lower_nybble(op_size_and_operands_count);

        size_t offset = base_size;
        for (size_t i = 0; i < operands_count; i++) {
            operands[i].type = static_cast<operand_encoding_t::flags_t>(*(encoding_ptr + offset));
            ++offset;

            if ((operands[i].is_reg())) {
                operands[i].value.r8 = *(encoding_ptr + offset);
                ++offset;
            } else {
                switch (size) {
                    case op_sizes::byte: {
                        uint8_t* constant_value_ptr = encoding_ptr + offset;
                        operands[i].value.u64 = *constant_value_ptr;
                        offset += sizeof(uint8_t);
                        break;
                    }
                    case op_sizes::word: {
                        uint16_t* constant_value_ptr = reinterpret_cast<uint16_t*>(encoding_ptr + offset);
                        operands[i].value.u64 = *constant_value_ptr;
                        offset += sizeof(uint16_t);
                        break;
                    }
                    case op_sizes::dword: {
                        if (operands[i].is_integer()) {
                            uint32_t* constant_value_ptr = reinterpret_cast<uint32_t*>(encoding_ptr + offset);
                            operands[i].value.u64 = *constant_value_ptr;
                            offset += sizeof(uint32_t);
                        } else {
                            float* constant_value_ptr = reinterpret_cast<float*>(encoding_ptr + offset);
                            operands[i].value.d64 = *constant_value_ptr;
                            offset += sizeof(float);
                        }
                        break;
                    }
                    case op_sizes::qword: {
                        if (operands[i].is_integer()) {
                            uint64_t* constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                            operands[i].value.u64 = *constant_value_ptr;
                            offset += sizeof(uint64_t);
                        } else {
                            double* constant_value_ptr = reinterpret_cast<double*>(encoding_ptr + offset);
                            operands[i].value.d64 = *constant_value_ptr;
                            offset += sizeof(double);
                        }
                        break;
                    }
                    case op_sizes::none: {
                        if (operands[i].is_integer()) {
                            r.add_message(
                                "B010",
                                "constant integers cannot have a size of 'none'.",
                                true);
                        } else {
                            r.add_message(
                                "B010",
                                "constant floats cannot have a size of 'none', 'byte', or 'word'.",
                                true);
                        }
                        break;
                    }
                }
            }
        }

        return encoding_size;
    }

    size_t instruction_t::encode(result& r, uint8_t* heap, uint64_t address) {
        if (address % alignment != 0) {
            r.add_message(
                "B003",
                fmt::format("instruction alignment violation: alignment = {} bytes, address = ${:016X}",
                            alignment,
                            address),
                true);
            return 0;
        }

        uint8_t encoding_size = base_size;
        size_t offset = base_size;

        auto encoding_ptr = heap + address;
        *(encoding_ptr + 1) = static_cast<uint8_t>(op);

        uint8_t size_type_and_operand_count = 0;
        size_type_and_operand_count = set_upper_nybble(
            size_type_and_operand_count,
            static_cast<uint8_t>(size));
        size_type_and_operand_count = set_lower_nybble(
            size_type_and_operand_count,
            operands_count);
        *(encoding_ptr + 2) = size_type_and_operand_count;

        for (size_t i = 0; i < operands_count; i++) {
            *(encoding_ptr + offset) = static_cast<uint8_t>(operands[i].type);
            ++offset;
            ++encoding_size;

            if (operands[i].is_reg()) {
                *(encoding_ptr + offset) = operands[i].value.r8;
                ++offset;
                ++encoding_size;
            } else {
                switch (size) {
                    case op_sizes::byte: {
                        uint8_t* constant_value_ptr = encoding_ptr + offset;
                        *constant_value_ptr = static_cast<uint8_t>(operands[i].value.u64);
                        offset += sizeof(uint8_t);
                        encoding_size += sizeof(uint8_t);
                        break;
                    }
                    case op_sizes::word: {
                        uint16_t* constant_value_ptr = reinterpret_cast<uint16_t*>(encoding_ptr + offset);
                        *constant_value_ptr = static_cast<uint16_t>(operands[i].value.u64);
                        offset += sizeof(uint16_t);
                        encoding_size += sizeof(uint16_t);
                        break;
                    }
                    case op_sizes::dword: {
                        if (operands[i].is_integer()) {
                            uint32_t* constant_value_ptr = reinterpret_cast<uint32_t*>(encoding_ptr + offset);
                            *constant_value_ptr = static_cast<uint32_t>(operands[i].value.u64);
                            offset += sizeof(uint32_t);
                            encoding_size += sizeof(uint32_t);
                        } else {
                            float* constant_value_ptr = reinterpret_cast<float*>(encoding_ptr + offset);
                            *constant_value_ptr = static_cast<float>(operands[i].value.d64);
                            offset += sizeof(float);
                            encoding_size += sizeof(float);
                        }
                        break;
                    }
                    case op_sizes::qword: {
                        if (operands[i].is_integer()) {
                            uint64_t* constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                            *constant_value_ptr = operands[i].value.u64;
                            offset += sizeof(uint64_t);
                            encoding_size += sizeof(uint64_t);
                        } else {
                            double* constant_value_ptr = reinterpret_cast<double*>(encoding_ptr + offset);
                            *constant_value_ptr = operands[i].value.d64;
                            offset += sizeof(double);
                            encoding_size += sizeof(double);
                        }
                        break;
                    }
                    case op_sizes::none:
                        if (operands[i].is_integer()) {
                            r.add_message(
                                "B009",
                                "constant integers cannot have a size of 'none'.",
                                true);
                        } else {
                            r.add_message(
                                "B009",
                                "constant floats cannot have a size of 'none', 'byte', or 'word'.",
                                true);
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
        operands[index].value.u64 = align(address, alignment);
    }

    ///////////////////////////////////////////////////////////////////////////

    instruction_cache::instruction_cache(terp* terp) : _terp(terp) {
    }

    void instruction_cache::reset() {
        _cache.clear();
    }

    size_t instruction_cache::fetch_at(
            result& r,
            uint64_t address,
            instruction_t& inst) {
        auto it = _cache.find(address);
        if (it == _cache.end()) {
            auto size = inst.decode(r, _terp->heap(), address);
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

    size_t instruction_cache::fetch(result& r, instruction_t& inst) {
        return fetch_at(r, _terp->register_file().pc, inst);
    }

    ///////////////////////////////////////////////////////////////////////////

    terp::terp(size_t heap_size, size_t stack_size) : _heap_size(heap_size),
                                                      _stack_size(stack_size),
                                                      _icache(this) {
    }

    terp::~terp() {
        delete _heap;
        _heap = nullptr;
    }

    void terp::reset() {
        _registers.pc = program_start;
        _registers.fr = 0;
        _registers.sr = 0;
        _registers.sp = _heap_size;

        for (size_t i = 0; i < 64; i++) {
            _registers.i[i] = 0;
            _registers.f[i] = 0.0;
        }

        _icache.reset();

        _exited = false;
    }

    uint64_t terp::pop() {
        uint64_t value = *qword_ptr(_registers.sp);
        _registers.sp += sizeof(uint64_t);
        return value;
    }

    uint64_t terp::peek() const {
        uint64_t value = *qword_ptr(_registers.sp);
        return value;
    }

    void terp::dump_state(uint8_t count) {
        fmt::print("\n-------------------------------------------------------------\n");
        fmt::print(
            "PC =${:08x} | SP =${:08x} | FR =${:08x} | SR =${:08x}\n",
            _registers.pc,
            _registers.sp,
            _registers.fr,
            _registers.sr);

        fmt::print("-------------------------------------------------------------\n");

        uint8_t index = 0;
        for (size_t y = 0; y < count; y++) {
            fmt::print(
                "I{:02}=${:08x} | I{:02}=${:08x} | I{:02}=${:08x} | I{:02}=${:08x}\n",
                index,
                _registers.i[index],
                index + 1,
                _registers.i[index + 1],
                index + 2,
                _registers.i[index + 2],
                index + 3,
                _registers.i[index + 3]);
            index += 4;
        }

        fmt::print("-------------------------------------------------------------\n");

        index = 0;
        for (size_t y = 0; y < count; y++) {
            fmt::print(
                "F{:02}=${:08x} | F{:02}=${:08x} | F{:02}=${:08x} | F{:02}=${:08x}\n",
                index,
                static_cast<uint64_t>(_registers.f[index]),
                index + 1,
                static_cast<uint64_t>(_registers.f[index + 1]),
                index + 2,
                static_cast<uint64_t>(_registers.f[index + 2]),
                index + 3,
                static_cast<uint64_t>(_registers.f[index + 3]));
            index += 4;
        }

        fmt::print("\n");
    }

    bool terp::step(result& r) {
        instruction_t inst;
        auto inst_size = _icache.fetch(r, inst);
        if (inst_size == 0)
            return false;

        _registers.pc += inst_size;

        switch (inst.op) {
            case op_codes::nop: {
                break;
            }
            case op_codes::load: {
                uint64_t address;

                if (!get_operand_value(r, inst, 1, address))
                    return false;

                if (inst.operands_count > 2) {
                    uint64_t offset;
                    if (!get_operand_value(r, inst, 2, offset))
                        return false;
                    address += offset;
                }

                uint64_t value = *qword_ptr(address);
                if (!set_target_operand_value(r, inst, 0, value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));
                break;
            }
            case op_codes::store: {
                uint64_t value;
                if (!get_operand_value(r, inst, 0, value))
                    return false;

                uint64_t address;
                if (!get_operand_value(r, inst, 1, address))
                    return false;

                if (inst.operands_count > 2) {
                    uint64_t offset;
                    if (!get_operand_value(r, inst, 2, offset))
                        return false;
                    address += offset;
                }

                *qword_ptr(address) = value;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));
                break;
            }
            case op_codes::copy: {
                uint64_t source_address, target_address;

                if (!get_operand_value(r, inst, 0, source_address))
                    return false;

                if (!get_operand_value(r, inst, 1, target_address))
                    return false;

                uint64_t length;
                if (!get_operand_value(r, inst, 2, length))
                    return false;

                memcpy(
                    _heap + target_address,
                    _heap + source_address,
                    length * op_size_in_bytes(inst.size));

                _registers.flags(register_file_t::flags_t::zero, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::negative, false);

                break;
            }
            case op_codes::fill: {
                uint64_t value;
                if (!get_operand_value(r, inst, 0, value))
                    return false;

                uint64_t address;
                if (!get_operand_value(r, inst, 1, address))
                    return false;

                uint64_t length;
                if (!get_operand_value(r, inst, 2, length))
                    return false;
                length *= op_size_in_bytes(inst.size);

                switch (inst.size) {
                    case op_sizes::byte:
                        memset(_heap + address, static_cast<uint8_t>(value), length);
                        break;
                    case op_sizes::word:
                        memset(_heap + address, static_cast<uint16_t>(value), length);
                        break;
                    case op_sizes::dword:
                        memset(_heap + address, static_cast<uint32_t>(value), length);
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
            case op_codes::move: {
                uint64_t source_value;

                if (!get_operand_value(r, inst, 0, source_value))
                    return false;

                if (!set_target_operand_value(r, inst, 1, source_value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, source_value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(source_value, inst.size));

                break;
            }
            case op_codes::push: {
                uint64_t source_value;

                if (!get_operand_value(r, inst, 0, source_value))
                    return false;

                push(source_value);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, source_value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(source_value, inst.size));

                break;
            }
            case op_codes::pop: {
                uint64_t value = pop();

                if (!set_target_operand_value(r, inst, 0, value))
                    return false;

                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));

                break;
            }
            case op_codes::dup: {
                uint64_t value = peek();
                push(value);

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));

                break;
            }
            case op_codes::inc: {
                uint8_t reg = inst.operands[0].value.r8;

                uint64_t lhs_value = _registers.i[reg];
                uint64_t rhs_value = 1;
                uint64_t value = lhs_value + rhs_value;
                if (set_target_operand_value(r, inst, reg, value))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, value, inst.size));
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::carry, has_carry(value, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));

                break;
            }
            case op_codes::dec: {
                uint8_t reg = inst.operands[0].value.r8;

                uint64_t lhs_value = _registers.i[reg];
                uint64_t rhs_value = 1;
                uint64_t value = lhs_value - rhs_value;
                if (set_target_operand_value(r, inst, reg, value))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, value, inst.size));
                _registers.flags(register_file_t::flags_t::subtract, true);
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::carry, has_carry(value, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));
                break;
            }
            case op_codes::add: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t sum_result = lhs_value + rhs_value;
                if (!set_target_operand_value(r, inst, 0, sum_result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, sum_result, inst.size));
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, sum_result == 0);
                _registers.flags(register_file_t::flags_t::carry, has_carry(sum_result, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(sum_result, inst.size));

                break;
            }
            case op_codes::sub: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t subtraction_result = lhs_value - rhs_value;
                if (!set_target_operand_value(r, inst, 0, subtraction_result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, subtraction_result, inst.size));
                _registers.flags(register_file_t::flags_t::subtract, true);
                _registers.flags(register_file_t::flags_t::carry, rhs_value > lhs_value);
                _registers.flags(register_file_t::flags_t::zero, subtraction_result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(subtraction_result, inst.size));
                break;
            }
            case op_codes::mul: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t product_result = lhs_value * rhs_value;
                if (!set_target_operand_value(r, inst, 0, product_result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, product_result, inst.size));
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, product_result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(product_result, inst.size));
                break;
            }
            case op_codes::div: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = 0;
                if (rhs_value != 0)
                    result = lhs_value / rhs_value;

                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, result, inst.size));
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::mod: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value % rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, result, inst.size));
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::neg: {
                uint64_t value;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                int64_t negated_result = -static_cast<int64_t>(value);
                uint64_t result = static_cast<uint64_t>(negated_result);
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::shr: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value >> rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));
                break;
            }
            case op_codes::shl: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value << rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::ror: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t right_rotated_value = rotr(lhs_value, static_cast<uint8_t>(rhs_value));
                if (!set_target_operand_value(r, inst, 0, right_rotated_value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, right_rotated_value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(right_rotated_value, inst.size));

                break;
            }
            case op_codes::rol: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t left_rotated_value = rotl(lhs_value, static_cast<uint8_t>(rhs_value));
                if (!set_target_operand_value(r, inst, 0, left_rotated_value))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, left_rotated_value == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(left_rotated_value, inst.size));

                break;
            }
            case op_codes::and_op: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value & rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::or_op: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value | rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::xor_op: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;

                uint64_t result = lhs_value ^ rhs_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::not_op: {
                uint64_t value;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                uint64_t not_result = ~value;
                if (!set_target_operand_value(r, inst, 0, not_result))
                    return false;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, not_result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(not_result, inst.size));

                break;
            }
            case op_codes::bis: {
                uint64_t value, bit_number;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                if (!get_operand_value(r, inst, 2, bit_number))
                    return false;

                uint64_t masked_value = static_cast<uint64_t>(1 << bit_number);
                uint64_t result = value | masked_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::zero, false);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::bic: {
                uint64_t value, bit_number;

                if (!get_operand_value(r, inst, 1, value))
                    return false;

                if (!get_operand_value(r, inst, 2, bit_number))
                    return false;

                uint64_t masked_value = static_cast<uint64_t>(~(1 << bit_number));
                uint64_t result = value & masked_value;
                if (!set_target_operand_value(r, inst, 0, result))
                    return false;

                _registers.flags(register_file_t::flags_t::zero, true);
                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::test: {
                uint64_t value, mask;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, mask))
                    return false;

                uint64_t result = value & mask;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::cmp: {
                uint64_t lhs_value, rhs_value;

                if (!get_operand_value(r, inst, 0, lhs_value))
                    return false;

                if (!get_operand_value(r, inst, 1, rhs_value))
                    return false;

                uint64_t result = lhs_value - rhs_value;

                _registers.flags(
                    register_file_t::flags_t::overflow,
                    has_overflow(lhs_value, rhs_value, result, inst.size));
                _registers.flags(register_file_t::flags_t::subtract, true);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::carry, has_carry(result, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::bz: {
                uint64_t value, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, address))
                    return false;

                if (value == 0)
                    _registers.pc = address;

                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::carry, has_carry(value, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));

                break;
            }
            case op_codes::bnz: {
                uint64_t value, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, address))
                    return false;

                if (value != 0)
                    _registers.pc = address;

                _registers.flags(register_file_t::flags_t::zero, value == 0);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::carry, has_carry(value, inst.size));
                _registers.flags(register_file_t::flags_t::negative, is_negative(value, inst.size));

                break;
            }
            case op_codes::tbz: {
                uint64_t value, mask, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, mask))
                    return false;

                if (!get_operand_value(r, inst, 2, address))
                    return false;

                uint64_t result = value & mask;
                if (result == 0)
                    _registers.pc = address;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::tbnz: {
                uint64_t value, mask, address;

                if (!get_operand_value(r, inst, 0, value))
                    return false;

                if (!get_operand_value(r, inst, 1, mask))
                    return false;

                if (!get_operand_value(r, inst, 2, address))
                    return false;

                uint64_t result = value & mask;
                if (result != 0)
                    _registers.pc = address;

                _registers.flags(register_file_t::flags_t::carry, false);
                _registers.flags(register_file_t::flags_t::subtract, false);
                _registers.flags(register_file_t::flags_t::overflow, false);
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::negative, is_negative(result, inst.size));

                break;
            }
            case op_codes::bne: {
                uint64_t address;

                if (!get_operand_value(r, inst, 0, address))
                    return false;

                if (!_registers.flags(register_file_t::flags_t::zero)) {
                    _registers.pc = address;
                }

                break;
            }
            case op_codes::beq: {
                uint64_t address;

                if (!get_operand_value(r, inst, 0, address))
                    return false;

                if (_registers.flags(register_file_t::flags_t::zero)) {
                    _registers.pc = address;
                }

                break;
            }
            case op_codes::bg: {
                uint64_t address;

                if (!get_operand_value(r, inst, 0, address))
                    return false;

                if (!_registers.flags(register_file_t::flags_t::carry)
                &&  !_registers.flags(register_file_t::flags_t::zero)) {
                    _registers.pc = address;
                }

                break;
            }
            case op_codes::bge: {
                uint64_t address;

                if (!get_operand_value(r, inst, 0, address))
                    return false;

                if (!_registers.flags(register_file_t::flags_t::carry)) {
                    _registers.pc = address;
                }
                break;
            }
            case op_codes::bl: {
                uint64_t address;

                if (!get_operand_value(r, inst, 0, address))
                    return false;

                if (_registers.flags(register_file_t::flags_t::carry)
                ||  _registers.flags(register_file_t::flags_t::zero)) {
                    _registers.pc = address;
                }
                break;
            }
            case op_codes::ble: {
                uint64_t address;

                if (!get_operand_value(r, inst, 0, address))
                    return false;

                if (_registers.flags(register_file_t::flags_t::carry)) {
                    _registers.pc = address;
                }
                break;
            }
            case op_codes::jsr: {
                push(_registers.pc);

                uint64_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;

                if (inst.operands_count == 2) {
                    uint64_t offset;

                    if (!get_operand_value(r, inst, 1, offset))
                        return false;

                    if (inst.operands[1].is_negative()) {
                        address -= offset + inst_size;
                    } else {
                        address += offset + inst_size;
                    }
                }

                _registers.pc = address;
                break;
            }
            case op_codes::rts: {
                uint64_t address = pop();
                _registers.pc = address;
                break;
            }
            case op_codes::jmp: {
                uint64_t address;

                if (!get_operand_value(r, inst, 0, address))
                    return false;

                _registers.pc = address;
                break;
            }
            case op_codes::swi: {
                uint64_t index;

                if (!get_operand_value(r, inst, 0, index))
                    return false;

                size_t swi_offset = sizeof(uint64_t) * index;
                uint64_t swi_address = *qword_ptr(swi_offset);
                if (swi_address != 0) {
                    // XXX: what state should we save and restore here?
                    push(_registers.pc);
                    _registers.pc = swi_address;
                }

                break;
            }
            case op_codes::trap: {
                uint64_t index;

                if (!get_operand_value(r, inst, 0, index))
                    return false;

                auto it = _traps.find(static_cast<uint8_t>(index));
                if (it == _traps.end())
                    break;

                it->second(this);

                break;
            }
            case op_codes::meta: {
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
        _registers.sp -= sizeof(uint64_t);
        *qword_ptr(_registers.sp) = value;
        return;
    }

    bool terp::initialize(result& r) {
        _heap = new uint8_t[_heap_size];
        reset();
        return !r.is_failed();
    }

    void terp::remove_trap(uint8_t index) {
        _traps.erase(index);
    }

    std::vector<uint64_t> terp::jump_to_subroutine(
            result& r,
            uint64_t address) {
        std::vector<uint64_t> return_values;

        auto return_address = _registers.pc;
        push(return_address);
        _registers.pc = address;

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
        *qword_ptr(swi_address) = address;
    }

    uint64_t terp::heap_vector(uint8_t index) const {
        size_t heap_vector_address = heap_vector_table_start + (sizeof(uint64_t) * index);
        return *qword_ptr(heap_vector_address);
    }

    void terp::heap_vector(uint8_t index, uint64_t address) {
        size_t heap_vector_address = heap_vector_table_start + (sizeof(uint64_t) * index);
        *qword_ptr(heap_vector_address) = address;
    }

    std::string terp::disassemble(const instruction_t& inst) const {
        std::stringstream stream;

        auto it = s_op_code_names.find(inst.op);
        if (it != s_op_code_names.end()) {
            std::stringstream mnemonic;
            std::string format_spec;

            mnemonic <<  it->second;
            switch (inst.size) {
                case op_sizes::byte:
                    mnemonic << ".B";
                    format_spec = "#${:02X}";
                    break;
                case op_sizes::word:
                    mnemonic << ".W";
                    format_spec = "#${:04X}";
                    break;
                case op_sizes::dword:
                    mnemonic << ".DW";
                    format_spec = "#${:08X}";
                    break;
                case op_sizes::qword:
                    mnemonic << ".QW";
                    format_spec = "#${:016X}";
                    break;
                default: {
                    break;
                }
            }

            stream << std::left << std::setw(10) << mnemonic.str();

            std::stringstream operands_stream;
            for (size_t i = 0; i < inst.operands_count; i++) {
                if (i > 0 && i < inst.operands_count) {
                    operands_stream << ", ";
                }

                const auto& operand = inst.operands[i];
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
                        switch (operand.value.r8) {
                            case i_registers_t::sp: {
                                operands_stream << prefix << "SP" << postfix;
                                break;
                            }
                            case i_registers_t::pc: {
                                operands_stream << prefix << "PC" << postfix;
                                break;
                            }
                            case i_registers_t::fr: {
                                operands_stream << "FR";
                                break;
                            }
                            case i_registers_t::sr: {
                                operands_stream << "SR";
                                break;
                            }
                            default: {
                                operands_stream << prefix
                                                << "I"
                                                << std::to_string(operand.value.r8)
                                                << postfix;
                                break;
                            }
                        }
                    } else {
                        operands_stream << "F" << std::to_string(operand.value.r8);
                    }
                } else {
                    if (operand.is_integer()) {
                        operands_stream << prefix
                                        << fmt::format(format_spec, operand.value.u64)
                                        << postfix;
                    } else {
                        operands_stream << prefix
                                        << fmt::format(format_spec, operand.value.d64)
                                        << postfix;
                    }
                }
            }

            stream << std::left << std::setw(24) << operands_stream.str();
        } else {
            stream << "UNKNOWN";
        }
        return stream.str();
    }

    const register_file_t& terp::register_file() const {
        return _registers;
    }

    void terp::dump_heap(uint64_t offset, size_t size) {
        auto program_memory = basecode::hex_formatter::dump_to_string(
            reinterpret_cast<const void*>(_heap + offset),
            size);
        fmt::print("{}\n", program_memory);
    }

    std::string terp::disassemble(result& r, uint64_t address) {
        std::stringstream stream;
        while (true) {
            instruction_t inst;
            auto inst_size = _icache.fetch_at(r, address, inst);
            if (inst_size == 0)
                break;

            stream << fmt::format("${:016X}: ", address)
                   << disassemble(inst)
                   << fmt::format(" (${:02X} bytes)\n", inst_size);

            if (inst.op == op_codes::exit)
                break;

            address += inst_size;
        }
        return stream.str();
    }

    bool terp::get_operand_value(
            result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            double& value) const {
        auto& operand = inst.operands[operand_index];

        if (operand.is_reg()) {
            if (operand.is_integer()) {
                value = _registers.i[operand.value.r8];
            } else {
                value = _registers.f[operand.value.r8];
            }
        } else {
            if (operand.is_integer()) {
                value = operand.value.u64;
            } else {
                value = operand.value.d64;
            }
        }

        return true;
    }

    bool terp::get_operand_value(
            result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t& value) const {
        auto& operand = inst.operands[operand_index];

        if (operand.is_reg()) {
            if (operand.is_integer()) {
                auto reg = static_cast<i_registers_t>(operand.value.r8);
                switch (reg) {
                    case i_registers_t::pc: {
                        value = _registers.pc;
                        break;
                    }
                    case i_registers_t::sp: {
                        value = _registers.sp;
                        break;
                    }
                    case i_registers_t::fr: {
                        value = _registers.fr;
                        break;
                    }
                    case i_registers_t::sr: {
                        value = _registers.sr;
                        break;
                    }
                    default: {
                        value = _registers.i[reg];
                        break;
                    }
                }
            } else {
                value = static_cast<uint64_t>(_registers.f[operand.value.r8]);
            }
        } else {
            if (operand.is_integer()) {
                value = operand.value.u64;
            } else {
                value = static_cast<uint64_t>(operand.value.d64);
            }
        }

        return true;
    }

    bool terp::set_target_operand_value(
            result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t value) {
        auto& operand = inst.operands[operand_index];

        if (operand.is_reg()) {
            if (operand.is_integer()) {
                auto reg = static_cast<i_registers_t>(operand.value.r8);
                switch (reg) {
                    case i_registers_t::pc: {
                        _registers.pc = set_zoned_value(_registers.pc, value, inst.size);
                        break;
                    }
                    case i_registers_t::sp: {
                        _registers.sp = set_zoned_value(_registers.sp, value, inst.size);
                        break;
                    }
                    case i_registers_t::fr: {
                        _registers.fr = set_zoned_value(_registers.fr, value, inst.size);
                        break;
                    }
                    case i_registers_t::sr: {
                        _registers.sr = set_zoned_value(_registers.sr, value, inst.size);
                        break;
                    }
                    default: {
                        _registers.i[reg] = set_zoned_value(_registers.i[reg], value, inst.size);
                        break;
                    }
                }
            } else {
                _registers.f[operand.value.r8] = value;
            }

        } else {
            r.add_message(
                "B006",
                "constant cannot be a target operand type.",
                true);
            return false;
        }

        return true;
    }

    bool terp::set_target_operand_value(
            result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            double value) {
        auto& operand = inst.operands[operand_index];

        if (operand.is_reg()) {
            if (operand.is_integer()) {
                auto integer_value = static_cast<uint64_t>(value);
                auto reg = static_cast<i_registers_t>(operand.value.r8);
                switch (reg) {
                    case i_registers_t::pc: {
                        _registers.pc = set_zoned_value(_registers.pc, integer_value, inst.size);
                        break;
                    }
                    case i_registers_t::sp: {
                        _registers.sp = set_zoned_value(_registers.sp, integer_value, inst.size);
                        break;
                    }
                    case i_registers_t::fr: {
                        _registers.fr = set_zoned_value(_registers.fr, integer_value, inst.size);
                        break;
                    }
                    case i_registers_t::sr: {
                        _registers.sr = set_zoned_value(_registers.sr, integer_value, inst.size);
                        break;
                    }
                    default: {
                        _registers.i[reg] = set_zoned_value(_registers.i[reg], integer_value, inst.size);;
                        break;
                    }
                }
            } else {
                _registers.f[operand.value.r8] = value;
            }
        } else {
            r.add_message(
                "B006",
                "constant cannot be a target operand type.",
                true);
            return false;
        }

        return true;
    }

    bool terp::has_carry(uint64_t value, op_sizes size) {
        switch (size) {
            case op_sizes::byte:
                return value > UINT8_MAX;
            case op_sizes::word:
                return value > UINT16_MAX;
            case op_sizes::dword:
                return value > UINT32_MAX;
            case op_sizes::qword:
            default:
                return value > UINT64_MAX;
        }
    }

    bool terp::is_negative(uint64_t value, op_sizes size) {
        switch (size) {
            case op_sizes::byte: {
                return (value & mask_byte_negative) != 0;
            }
            case op_sizes::word: {
                return (value & mask_word_negative) != 0;
            }
            case op_sizes::dword: {
                return (value & mask_dword_negative) != 0;
            }
            case op_sizes::qword:
            default:
                return (value & mask_qword_negative) != 0;
        }
        return false;
    }

    void terp::register_trap(uint8_t index, const terp::trap_callable& callable) {
        _traps.insert(std::make_pair(index, callable));
    }

    uint64_t terp::set_zoned_value(uint64_t source, uint64_t value, op_sizes size) {
        uint64_t result = source;
        switch (size) {
            case op_sizes::byte: {
                result &= mask_byte_clear;
                result |= (value & mask_byte);
                break;
            }
            case op_sizes::word: {
                result &= mask_word_clear;
                result |= (value & mask_word);
                break;
            }
            case op_sizes::dword: {
                result &= mask_dword_clear;
                result |= (value & mask_dword);
                break;
            }
            case op_sizes::qword:
            default:
                result = value;
                break;
        }
        return result;
    }

    bool terp::has_overflow(uint64_t lhs, uint64_t rhs, uint64_t result, op_sizes size) {
        switch (size) {
            case op_sizes::byte:
                return ((~(lhs ^ rhs)) & (lhs ^ result) & mask_byte_negative) != 0;
            case op_sizes::word:
                return ((~(lhs ^ rhs)) & (lhs ^ result) & mask_word_negative) != 0;
            case op_sizes::dword:
                return ((~(lhs ^ rhs)) & (lhs ^ result) & mask_dword_negative) != 0;
            case op_sizes::qword:
            default: {
                return ((~(lhs ^ rhs)) & (lhs ^ result) & mask_qword_negative) != 0;
            }
        }
        return false;
    }

};