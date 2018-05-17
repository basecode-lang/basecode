#include <sstream>
#include <iomanip>
#include <fmt/format.h>
#include "terp.h"
#include "hex_formatter.h"

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
            encoding_size += 2;
            switch (operands[i].type) {
                case operand_types::constant_integer:
                case operand_types::increment_constant_pre:
                case operand_types::increment_constant_post:
                case operand_types::decrement_constant_pre:
                case operand_types::decrement_constant_post:
                case operand_types::constant_offset_positive:
                case operand_types::constant_offset_negative: {
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
                            encoding_size += sizeof(uint32_t);
                            break;
                        case op_sizes::qword:
                            encoding_size += sizeof(uint64_t);
                            break;
                    }
                    break;
                }
                case operand_types::constant_float: {
                    switch (size) {
                        case op_sizes::none:
                        case op_sizes::byte:
                        case op_sizes::word:
                            break;
                        case op_sizes::dword:
                            encoding_size += sizeof(float);
                            break;
                        case op_sizes::qword:
                            encoding_size += sizeof(double);
                            break;
                    }
                    break;
                }
                default:
                    break;
            }
        }

        encoding_size = align(encoding_size, sizeof(uint64_t));

        return encoding_size;
    }

    size_t instruction_t::align(uint64_t value, size_t size) const {
        auto offset = value % size;
        return offset ? value + (size - offset) : value;
    }

    size_t instruction_t::decode(result& r, uint8_t* heap, uint64_t address) {
        if (address % 8 != 0) {
            r.add_message(
                "B003",
                fmt::format("instructions must be decoded on 8-byte boundaries: address = ${:016X}", address),
                true);
            return 0;
        }

        uint8_t* encoding_ptr = heap + address;
        uint8_t encoding_size = *encoding_ptr;
        op = static_cast<op_codes>(*(encoding_ptr + 1));
        size = static_cast<op_sizes>(static_cast<uint8_t>(*(encoding_ptr + 2)));
        operands_count = static_cast<uint8_t>(*(encoding_ptr + 3));

        size_t offset = base_size;
        for (size_t i = 0; i < operands_count; i++) {
            operands[i].type = static_cast<operand_types>(*(encoding_ptr + offset));
            ++offset;

            operands[i].index = *(encoding_ptr + offset);
            ++offset;

            switch (operands[i].type) {
                case operand_types::constant_integer:
                case operand_types::constant_offset_negative:
                case operand_types::constant_offset_positive:
                case operand_types::increment_constant_pre:
                case operand_types::decrement_constant_pre:
                case operand_types::increment_constant_post:
                case operand_types::decrement_constant_post: {
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
                            uint32_t* constant_value_ptr = reinterpret_cast<uint32_t*>(encoding_ptr + offset);
                            operands[i].value.u64 = *constant_value_ptr;
                            offset += sizeof(uint32_t);
                            break;
                        }
                        case op_sizes::qword: {
                            uint64_t* constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                            operands[i].value.u64 = *constant_value_ptr;
                            offset += sizeof(uint64_t);
                            break;
                        }
                        case op_sizes::none: {
                            r.add_message(
                                "B010",
                                "constant integers cannot have a size of 'none'.",
                                true);
                            break;
                        }
                    }
                    break;
                }
                case operand_types::constant_float: {
                    switch (size) {
                        case op_sizes::dword: {
                            float* constant_value_ptr = reinterpret_cast<float*>(encoding_ptr + offset);
                            operands[i].value.d64 = *constant_value_ptr;
                            offset += sizeof(float);
                            break;
                        }
                        case op_sizes::qword: {
                            double* constant_value_ptr = reinterpret_cast<double*>(encoding_ptr + offset);
                            operands[i].value.d64 = *constant_value_ptr;
                            offset += sizeof(double);
                            break;
                        }
                        case op_sizes::none:
                        case op_sizes::byte:
                        case op_sizes::word:
                            r.add_message(
                                "B010",
                                "constant floats cannot have a size of 'none', 'byte', or 'word'.",
                                true);
                            break;
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        return encoding_size;
    }

    size_t instruction_t::encode(result& r, uint8_t* heap, uint64_t address) {
        if (address % 8 != 0) {
            r.add_message(
                "B003",
                fmt::format("instructions must be encoded on 8-byte boundaries: address = ${:016X}", address),
                true);
            return 0;
        }

        uint8_t encoding_size = base_size;
        size_t offset = base_size;

        auto encoding_ptr = heap + address;
        *(encoding_ptr + 1) = static_cast<uint8_t>(op);
        *(encoding_ptr + 2) = static_cast<uint8_t>(size);
        *(encoding_ptr + 3) = operands_count;

        for (size_t i = 0; i < operands_count; i++) {
            *(encoding_ptr + offset) = static_cast<uint8_t>(operands[i].type);
            ++offset;

            *(encoding_ptr + offset) = operands[i].index;
            ++offset;

            encoding_size += 2;

            switch (operands[i].type) {
                case operand_types::constant_integer:
                case operand_types::constant_offset_negative:
                case operand_types::constant_offset_positive:
                case operand_types::increment_constant_pre:
                case operand_types::increment_constant_post:
                case operand_types::decrement_constant_pre:
                case operand_types::decrement_constant_post: {
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
                            uint32_t* constant_value_ptr = reinterpret_cast<uint32_t*>(encoding_ptr + offset);
                            *constant_value_ptr = static_cast<uint32_t>(operands[i].value.u64);
                            offset += sizeof(uint32_t);
                            encoding_size += sizeof(uint32_t);
                            break;
                        }
                        case op_sizes::qword: {
                            uint64_t* constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                            *constant_value_ptr = operands[i].value.u64;
                            offset += sizeof(uint64_t);
                            encoding_size += sizeof(uint64_t);
                            break;
                        }
                        case op_sizes::none:
                            r.add_message(
                                "B009",
                                "constant integers cannot have a size of 'none'.",
                                true);
                            break;
                    }
                    break;
                }
                case operand_types::constant_float: {
                    switch (size) {
                        case op_sizes::dword: {
                            float* constant_value_ptr = reinterpret_cast<float*>(encoding_ptr + offset);
                            *constant_value_ptr = static_cast<float>(operands[i].value.d64);
                            offset += sizeof(float);
                            encoding_size += sizeof(float);
                            break;
                        }
                        case op_sizes::qword: {
                            double* constant_value_ptr = reinterpret_cast<double*>(encoding_ptr + offset);
                            *constant_value_ptr = operands[i].value.d64;
                            offset += sizeof(double);
                            encoding_size += sizeof(double);
                            break;
                        }
                        case op_sizes::byte:
                        case op_sizes::word:
                        case op_sizes::none:
                            r.add_message(
                                "B009",
                                "constant floats cannot have a size of 'none', 'byte', or 'word'.",
                                true);
                            break;
                    }
                }
                default: {
                    break;
                }
            }
        }

        encoding_size = static_cast<uint8_t>(align(encoding_size, sizeof(uint64_t)));
        *encoding_ptr = encoding_size;

        return encoding_size;
    }

    void instruction_t::patch_branch_address(uint64_t address, uint8_t index) {
        operands[index].value.u64 = align(address, sizeof(uint64_t));
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

    terp::terp(size_t heap_size) : _heap_size(heap_size),
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
                break;
            }
            case op_codes::move: {
                uint64_t source_value;
                if (!get_operand_value(r, inst, 0, source_value))
                    return false;
                if (!set_target_operand_value(r, inst, 1, source_value))
                    return false;
                break;
            }
            case op_codes::push: {
                uint64_t source_value;
                if (!get_operand_value(r, inst, 0, source_value))
                    return false;
                push(source_value);
                break;
            }
            case op_codes::pop: {
                uint64_t value = pop();
                if (!set_target_operand_value(r, inst, 0, value))
                    return false;
                break;
            }
            case op_codes::dup: {
                push(peek());
                break;
            }
            case op_codes::inc: {
                _registers.i[inst.operands[0].index]++;
                break;
            }
            case op_codes::dec: {
                _registers.i[inst.operands[0].index]--;
                break;
            }
            case op_codes::add: {
                uint64_t lhs_value, rhs_value;
                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;
                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;
                if (!set_target_operand_value(r, inst, 0, lhs_value + rhs_value))
                    return false;
                break;
            }
            case op_codes::sub: {
                uint64_t lhs_value, rhs_value;
                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;
                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;
                if (!set_target_operand_value(r, inst, 0, lhs_value - rhs_value))
                    return false;
                break;
            }
            case op_codes::mul: {
                uint64_t lhs_value, rhs_value;
                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;
                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;
                if (!set_target_operand_value(r, inst, 0, lhs_value * rhs_value))
                    return false;
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
                break;
            }
            case op_codes::mod: {
                uint64_t lhs_value, rhs_value;
                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;
                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;
                if (!set_target_operand_value(r, inst, 0, lhs_value % rhs_value))
                    return false;
                break;
            }
            case op_codes::neg: {
                uint64_t value;
                if (!get_operand_value(r, inst, 1, value))
                    return false;
                int64_t negated_result = -static_cast<int64_t>(value);
                if (!set_target_operand_value(r, inst, 0, static_cast<uint64_t>(negated_result)))
                    return false;
                break;
            }
            case op_codes::shr: {
                uint64_t lhs_value, rhs_value;
                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;
                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;
                if (!set_target_operand_value(r, inst, 0, lhs_value >> rhs_value))
                    return false;
                break;
            }
            case op_codes::shl: {
                uint64_t lhs_value, rhs_value;
                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;
                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;
                if (!set_target_operand_value(r, inst, 0, lhs_value << rhs_value))
                    return false;
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
                break;
            }
            case op_codes::and_op: {
                uint64_t lhs_value, rhs_value;
                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;
                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;
                if (!set_target_operand_value(r, inst, 0, lhs_value & rhs_value))
                    return false;
                break;
            }
            case op_codes::or_op: {
                uint64_t lhs_value, rhs_value;
                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;
                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;
                if (!set_target_operand_value(r, inst, 0, lhs_value | rhs_value))
                    return false;
                break;
            }
            case op_codes::xor_op: {
                uint64_t lhs_value, rhs_value;
                if (!get_operand_value(r, inst, 1, lhs_value))
                    return false;
                if (!get_operand_value(r, inst, 2, rhs_value))
                    return false;
                if (!set_target_operand_value(r, inst, 0, lhs_value ^ rhs_value))
                    return false;
                break;
            }
            case op_codes::not_op: {
                uint64_t value;
                if (!get_operand_value(r, inst, 1, value))
                    return false;
                uint64_t not_result = ~value;
                if (!set_target_operand_value(r, inst, 0, not_result))
                    return false;
                break;
            }
            case op_codes::bis: {
                uint64_t value, bit_number;
                if (!get_operand_value(r, inst, 1, value))
                    return false;
                if (!get_operand_value(r, inst, 2, bit_number))
                    return false;
                uint64_t masked_value = static_cast<uint64_t>(1 << bit_number);
                if (!set_target_operand_value(r, inst, 0, value | masked_value))
                    return false;
                break;
            }
            case op_codes::bic: {
                uint64_t value, bit_number;
                if (!get_operand_value(r, inst, 1, value))
                    return false;
                if (!get_operand_value(r, inst, 2, bit_number))
                    return false;
                uint64_t masked_value = static_cast<uint64_t>(~(1 << bit_number));
                if (!set_target_operand_value(r, inst, 0, value & masked_value))
                    return false;
                break;
            }
            case op_codes::test: {
                uint64_t value, mask;
                if (!get_operand_value(r, inst, 0, value))
                    return false;
                if (!get_operand_value(r, inst, 1, mask))
                    return false;
                _registers.flags(register_file_t::flags_t::zero, (value & mask) != 0);
                break;
            }
            case op_codes::cmp: {
                uint64_t lhs_value, rhs_value;
                if (!get_operand_value(r, inst, 0, lhs_value))
                    return false;
                if (!get_operand_value(r, inst, 1, rhs_value))
                    return false;
                uint64_t result = lhs_value - rhs_value;
                _registers.flags(register_file_t::flags_t::zero, result == 0);
                _registers.flags(register_file_t::flags_t::overflow, rhs_value > lhs_value);
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
                if ((value & mask) == 0)
                    _registers.pc = address;
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
                if ((value & mask) != 0)
                    _registers.pc = address;
                break;
            }
            case op_codes::bne: {
                uint64_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;
                if (_registers.flags(register_file_t::flags_t::zero) == 0) {
                    _registers.pc = address;
                }
                break;
            }
            case op_codes::beq: {
                uint64_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;
                if (_registers.flags(register_file_t::flags_t::zero) != 0) {
                    _registers.pc = address;
                }
                break;
            }
            case op_codes::bg: {
                break;
            }
            case op_codes::bge: {
                break;
            }
            case op_codes::bl: {
                break;
            }
            case op_codes::ble: {
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

                    switch (inst.operands[1].type) {
                        case operand_types::constant_offset_positive:
                            address += offset + inst_size;
                            break;
                        case operand_types::constant_offset_negative:
                            address -= offset + inst_size;
                            break;
                        default:
                            break;
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

    void terp::swi(uint8_t index, uint64_t address) {
        size_t swi_address = sizeof(uint64_t) * index;
        *qword_ptr(swi_address) = address;
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
                switch (inst.operands[i].type) {
                    case operand_types::register_integer:
                        operands_stream << "I" << std::to_string(inst.operands[i].index);
                        break;
                    case operand_types::register_floating_point:
                        operands_stream << "F" << std::to_string(inst.operands[i].index);
                        break;
                    case operand_types::register_sp:
                        operands_stream << "SP";
                        break;
                    case operand_types::register_pc:
                        operands_stream << "PC";
                        break;
                    case operand_types::register_flags:
                        operands_stream << "FR";
                        break;
                    case operand_types::register_status:
                        operands_stream << "SR";
                        break;
                    case operand_types::constant_integer:
                    case operand_types::constant_offset_positive:
                        operands_stream << fmt::format(format_spec, inst.operands[i].value.u64);
                        break;
                    case operand_types::constant_float:
                        operands_stream << fmt::format(format_spec, inst.operands[i].value.d64);
                        break;
                    case operand_types::constant_offset_negative:
                        format_spec = "-" + format_spec;
                        operands_stream << fmt::format(format_spec, inst.operands[i].value.u64);
                        break;
                    case operand_types::increment_constant_pre:
                        operands_stream << "++" << std::to_string(inst.operands[i].value.u64);
                        break;
                    case operand_types::increment_constant_post:
                        operands_stream << std::to_string(inst.operands[i].value.u64) << "++";
                        break;
                    case operand_types::increment_register_pre:
                        operands_stream << "++" << "I" << std::to_string(inst.operands[i].index);
                        break;
                    case operand_types::increment_register_post:
                        operands_stream << "I" << std::to_string(inst.operands[i].index) << "++";
                        break;
                    case operand_types::decrement_constant_pre:
                        operands_stream << "--" << std::to_string(inst.operands[i].value.u64);
                        break;
                    case operand_types::decrement_constant_post:
                        operands_stream << std::to_string(inst.operands[i].value.u64) << "--";
                        break;
                    case operand_types::decrement_register_pre:
                        operands_stream << "--" << "I" << std::to_string(inst.operands[i].index);
                        break;
                    case operand_types::decrement_register_post:
                        operands_stream << "I" << std::to_string(inst.operands[i].index) << "--";
                        break;
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
            const instruction_t& instruction,
            uint8_t operand_index,
            double& value) const {
        switch (instruction.operands[operand_index].type) {
            case operand_types::increment_register_pre:
            case operand_types::decrement_register_pre:
            case operand_types::increment_register_post:
            case operand_types::decrement_register_post:
            case operand_types::register_floating_point: {
                value = _registers.f[instruction.operands[operand_index].index];
                break;
            }
            case operand_types::register_sp:
            case operand_types::register_pc:
            case operand_types::register_flags:
            case operand_types::register_status:
            case operand_types::register_integer: {
                r.add_message(
                    "B005",
                    "integer registers cannot be used for floating point operands.",
                    true);
                break;
            }
            case operand_types::constant_integer:
            case operand_types::increment_constant_pre:
            case operand_types::decrement_constant_pre:
            case operand_types::increment_constant_post:
            case operand_types::decrement_constant_post:
            case operand_types::constant_offset_negative:
            case operand_types::constant_offset_positive: {
                value = instruction.operands[operand_index].value.u64;
                break;
            }
            case operand_types::constant_float: {
                value = instruction.operands[operand_index].value.d64;
                break;
            }
        }

        return true;
    }

    bool terp::get_operand_value(
            result& r,
            const instruction_t& instruction,
            uint8_t operand_index,
            uint64_t& value) const {
        switch (instruction.operands[operand_index].type) {
            case operand_types::increment_register_pre:
            case operand_types::decrement_register_pre:
            case operand_types::increment_register_post:
            case operand_types::decrement_register_post:
            case operand_types::register_integer: {
                value = _registers.i[instruction.operands[operand_index].index];
                break;
            }
            case operand_types::register_floating_point: {
                value = static_cast<uint64_t>(_registers.f[instruction.operands[operand_index].index]);
                break;
            }
            case operand_types::register_sp: {
                value = _registers.sp;
                break;
            }
            case operand_types::register_pc: {
                value = _registers.pc;
                break;
            }
            case operand_types::register_flags: {
                value = _registers.fr;
                break;
            }
            case operand_types::register_status: {
                value = _registers.sr;
                break;
            }
            case operand_types::constant_integer:
            case operand_types::increment_constant_pre:
            case operand_types::decrement_constant_pre:
            case operand_types::increment_constant_post:
            case operand_types::decrement_constant_post:
            case operand_types::constant_offset_positive:
            case operand_types::constant_offset_negative: {
                value = instruction.operands[operand_index].value.u64;
                break;
            }
            case operand_types::constant_float: {
                value = static_cast<uint64_t>(instruction.operands[operand_index].value.d64);
                break;
            }
        }

        return true;
    }

    bool terp::set_target_operand_value(
            result& r,
            const instruction_t& instruction,
            uint8_t operand_index,
            uint64_t value) {
        switch (instruction.operands[operand_index].type) {
            case operand_types::increment_register_pre:
            case operand_types::decrement_register_pre:
            case operand_types::increment_register_post:
            case operand_types::decrement_register_post:
            case operand_types::register_integer: {
                _registers.i[instruction.operands[operand_index].index] = value;
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                return true;
            }
            case operand_types::register_floating_point: {
                _registers.f[instruction.operands[operand_index].index] = value;
                _registers.flags(register_file_t::flags_t::zero, value == 0);
                break;
            }
            case operand_types::register_sp: {
                _registers.sp = value;
                break;
            }
            case operand_types::register_pc: {
                _registers.pc = value;
                break;
            }
            case operand_types::register_flags: {
                _registers.fr = value;
                break;
            }
            case operand_types::register_status: {
                _registers.sr = value;
                break;
            }
            case operand_types::constant_float:
            case operand_types::constant_integer:
            case operand_types::constant_offset_negative:
            case operand_types::constant_offset_positive:
            case operand_types::increment_constant_pre:
            case operand_types::decrement_constant_pre:
            case operand_types::increment_constant_post:
            case operand_types::decrement_constant_post: {
                r.add_message(
                    "B006",
                    "constant cannot be a target operand type.",
                    true);
                break;
            }
        }

        return false;
    }

    bool terp::set_target_operand_value(
            result& r,
            const instruction_t& instruction,
            uint8_t operand_index,
            double value) {
        switch (instruction.operands[operand_index].type) {
            case operand_types::increment_register_pre:
            case operand_types::decrement_register_pre:
            case operand_types::increment_register_post:
            case operand_types::decrement_register_post:
            case operand_types::register_integer: {
                uint64_t integer_value = static_cast<uint64_t>(value);
                _registers.i[instruction.operands[operand_index].index] = integer_value;
                _registers.flags(register_file_t::flags_t::zero, integer_value == 0);
                break;
            }
            case operand_types::register_floating_point: {
                _registers.f[instruction.operands[operand_index].index] = value;
                break;
            }
            case operand_types::register_sp: {
                _registers.sp = static_cast<uint64_t>(value);
                break;
            }
            case operand_types::register_pc: {
                _registers.pc = static_cast<uint64_t>(value);
                break;
            }
            case operand_types::register_flags: {
                _registers.fr = static_cast<uint64_t>(value);
                break;
            }
            case operand_types::register_status: {
                _registers.sr = static_cast<uint64_t>(value);
                break;
            }
            case operand_types::constant_float:
            case operand_types::constant_integer:
            case operand_types::constant_offset_positive:
            case operand_types::constant_offset_negative:
            case operand_types::increment_constant_pre:
            case operand_types::increment_constant_post:
            case operand_types::decrement_constant_pre:
            case operand_types::decrement_constant_post: {
                r.add_message(
                    "B006",
                    "constant cannot be a target operand type.",
                    true);
                break;
            }
        }

        return false;
    }

    void terp::register_trap(uint8_t index, const terp::trap_callable& callable) {
        _traps.insert(std::make_pair(index, callable));
    }

};