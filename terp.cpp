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

    terp::terp(size_t heap_size) : _heap_size(heap_size) {
    }

    terp::~terp() {
        delete _heap;
        _heap = nullptr;
    }

    void terp::reset() {
        _registers.pc = 0;
        _registers.fr = 0;
        _registers.sr = 0;
        _registers.sp = _heap_size;

        for (size_t i = 0; i < 64; i++) {
            _registers.i[i] = 0;
            _registers.f[i] = 0.0;
        }

        _exited = false;
    }

    uint64_t terp::pop() {
        uint64_t value = *qword_ptr(_registers.sp);
        _registers.sp += sizeof(uint64_t);
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
        auto inst_size = inst.decode(r, _heap, _registers.pc);
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
                _registers.flags(register_file_t::flags_t::zero, false);

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
                _registers.flags(register_file_t::flags_t::zero, false);

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
                _registers.flags(register_file_t::flags_t::zero, false);

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
                _registers.flags(register_file_t::flags_t::zero, false);

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
                    _registers.flags(register_file_t::flags_t::zero, false);
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
                _registers.flags(register_file_t::flags_t::zero, false);

                push(_registers.pc);
                uint64_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;
                _registers.pc = address;
                break;
            }
            case op_codes::rts: {
                uint64_t address = pop();
                _registers.pc = address;
                break;
            }
            case op_codes::jmp: {
                _registers.flags(register_file_t::flags_t::zero, false);

                uint64_t address;
                if (!get_operand_value(r, inst, 0, address))
                    return false;
                _registers.pc = address;
                break;
            }
            case op_codes::meta: {
                break;
            }
            case op_codes::debug: {
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

    std::string terp::disassemble(const instruction_t& inst) const {
        std::stringstream stream;

        auto it = s_op_code_names.find(inst.op);
        if (it != s_op_code_names.end()) {
            std::stringstream mnemonic;

            mnemonic <<  it->second;
            switch (inst.size) {
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

            for (size_t i = 0; i < inst.operands_count; i++) {
                if (i > 0 && i < inst.operands_count) {
                    stream << ", ";
                }
                switch (inst.operands[i].type) {
                    case operand_types::register_integer:
                        stream << "I" << std::to_string(inst.operands[i].index);
                        break;
                    case operand_types::register_floating_point:
                        stream << "F" << std::to_string(inst.operands[i].index);
                        break;
                    case operand_types::register_sp:
                        stream << "SP";
                        break;
                    case operand_types::register_pc:
                        stream << "PC";
                        break;
                    case operand_types::register_flags:
                        stream << "FR";
                        break;
                    case operand_types::register_status:
                        stream << "SR";
                        break;
                    case operand_types::constant_integer:
                        stream << fmt::format("#${:08X}", inst.operands[i].value.u64);
                        break;
                    case operand_types::constant_float:
                        stream << fmt::format("#${:08X}", inst.operands[i].value.d64);
                        break;
                    case operand_types::increment_constant_pre:
                        stream << "++" << std::to_string(inst.operands[i].value.u64);
                        break;
                    case operand_types::increment_constant_post:
                        stream << std::to_string(inst.operands[i].value.u64) << "++";
                        break;
                    case operand_types::increment_register_pre:
                        stream << "++" << "I" << std::to_string(inst.operands[i].index);
                        break;
                    case operand_types::increment_register_post:
                        stream << "I" << std::to_string(inst.operands[i].index) << "++";
                        break;
                    case operand_types::decrement_constant_pre:
                        stream << "--" << std::to_string(inst.operands[i].value.u64);
                        break;
                    case operand_types::decrement_constant_post:
                        stream << std::to_string(inst.operands[i].value.u64) << "--";
                        break;
                    case operand_types::decrement_register_pre:
                        stream << "--" << "I" << std::to_string(inst.operands[i].index);
                        break;
                    case operand_types::decrement_register_post:
                        stream << "I" << std::to_string(inst.operands[i].index) << "--";
                        break;
                }
            }
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
            auto inst_size = inst.decode(r, _heap, address);
            if (inst_size == 0)
                break;

            stream << fmt::format("${:08X}: ", address)
                   << disassemble(inst) << "\n";

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
            case operand_types::increment_constant_pre:
            case operand_types::decrement_constant_pre:
            case operand_types::increment_constant_post:
            case operand_types::decrement_constant_post:
            case operand_types::constant_integer: {
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
            case operand_types::increment_constant_pre:
            case operand_types::decrement_constant_pre:
            case operand_types::increment_constant_post:
            case operand_types::decrement_constant_post:
            case operand_types::constant_integer: {
                value = instruction.operands[operand_index].value.u64;
                break;
            }
            case operand_types::constant_float: {
                value = static_cast<uint64_t>(instruction.operands[operand_index].value.d64);
                break;
            }
        }

        // XXX: need to implement zero extend
        switch (instruction.size) {
            case op_sizes::byte:
                break;
            case op_sizes::word:
                break;
            case op_sizes::dword:
                break;
            case op_sizes::qword:
                break;
            default: {
                r.add_message(
                    "B005",
                    "unsupported size of 'none' for operand.",
                    true);
                return false;
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
                return true;
            }
            case operand_types::register_floating_point: {
                _registers.f[instruction.operands[operand_index].index] = value;
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
                _registers.i[instruction.operands[operand_index].index] = static_cast<uint64_t>(value);
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

};