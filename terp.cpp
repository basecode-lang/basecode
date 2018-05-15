#include <sstream>
#include <iomanip>
#include <fmt/format.h>
#include "terp.h"
#include "hex_formatter.h"

namespace basecode {

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

    void terp::dump_state() {
        fmt::print("Basecode Interpreter State ----------------------------------\n");

        uint8_t index = 0;
        for (size_t y = 0; y < 16; y++) {
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
        for (size_t y = 0; y < 16; y++) {
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

        fmt::print("-------------------------------------------------------------\n");

        fmt::print(
            "PC =${:08x} | SP =${:08x} | FR =${:08x} | SR =${:08x}\n\n",
            _registers.pc,
            _registers.sp,
            _registers.fr,
            _registers.sr);
    }

    bool terp::step(result& r) {
        instruction_t inst;
        auto inst_size = decode_instruction(r, _registers.pc, inst);
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
                break;
            }
            case op_codes::fill: {
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
                break;
            }
            case op_codes::shr: {
                break;
            }
            case op_codes::shl: {
                break;
            }
            case op_codes::ror: {
                break;
            }
            case op_codes::rol: {
                break;
            }
            case op_codes::and_op: {
                break;
            }
            case op_codes::or_op: {
                break;
            }
            case op_codes::xor_op: {
                break;
            }
            case op_codes::not_op: {
                break;
            }
            case op_codes::bis: {
                break;
            }
            case op_codes::bic: {
                break;
            }
            case op_codes::test: {
                break;
            }
            case op_codes::cmp: {
                break;
            }
            case op_codes::bz: {
                break;
            }
            case op_codes::bnz: {
                break;
            }
            case op_codes::tbz: {
                break;
            }
            case op_codes::tbnz: {
                break;
            }
            case op_codes::bne: {
                break;
            }
            case op_codes::beq: {
                break;
            }
            case op_codes::bae: {
                break;
            }
            case op_codes::ba: {
                break;
            }
            case op_codes::ble: {
                break;
            }
            case op_codes::bl: {
                break;
            }
            case op_codes::bo: {
                break;
            }
            case op_codes::bcc: {
                break;
            }
            case op_codes::bcs: {
                break;
            }
            case op_codes::jsr: {
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

    size_t terp::encode_instruction(
            result& r,
            uint64_t address,
            instruction_t instruction) {
        if (address % 8 != 0) {
            r.add_message("B003", "Instructions must be encoded on 8-byte boundaries.", true);
            return 0;
        }

        uint8_t size = 5;

        auto op_ptr = word_ptr(address + 1);
        *op_ptr = static_cast<uint16_t>(instruction.op);

        auto encoding_ptr = byte_ptr(address);
        *(encoding_ptr + 3) = static_cast<uint8_t>(instruction.size);
        *(encoding_ptr + 4) = instruction.operands_count;

        size_t offset = 5;
        for (size_t i = 0; i < instruction.operands_count; i++) {
            *(encoding_ptr + offset) = static_cast<uint8_t>(instruction.operands[i].type);
            ++offset;
            ++size;

            *(encoding_ptr + offset) = instruction.operands[i].index;
            ++offset;
            ++size;

            switch (instruction.operands[i].type) {
                case operand_types::register_sp:
                case operand_types::register_pc:
                case operand_types::register_flags:
                case operand_types::register_status:
                case operand_types::register_integer:
                case operand_types::increment_register_pre:
                case operand_types::decrement_register_pre:
                case operand_types::register_floating_point:
                case operand_types::increment_register_post:
                case operand_types::decrement_register_post:
                    break;
                case operand_types::increment_constant_pre:
                case operand_types::increment_constant_post:
                case operand_types::decrement_constant_pre:
                case operand_types::decrement_constant_post:
                case operand_types::constant_integer: {
                    uint64_t* constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                    *constant_value_ptr = instruction.operands[i].value.u64;
                    offset += sizeof(uint64_t);
                    size += sizeof(uint64_t);
                    break;
                }
                case operand_types::constant_float: {
                    double* constant_value_ptr = reinterpret_cast<double*>(encoding_ptr + offset);
                    *constant_value_ptr = instruction.operands[i].value.d64;
                    offset += sizeof(double);
                    size += sizeof(double);
                    break;
                }
            }
        }

        if (size < 8)
            size = 8;

        size = static_cast<uint8_t>(align(size, sizeof(uint64_t)));
        *encoding_ptr = size;
        return size;
    }

    size_t terp::decode_instruction(
            result& r,
            uint64_t address,
            instruction_t& instruction) {
        if (address % 8 != 0) {
            r.add_message("B003", "Instructions must be decoded on 8-byte boundaries.", true);
            return 0;
        }

        uint16_t* op_ptr = word_ptr(address + 1);
        instruction.op = static_cast<op_codes>(*op_ptr);

        uint8_t* encoding_ptr = byte_ptr(address);
        uint8_t size = *encoding_ptr;
        instruction.size = static_cast<op_sizes>(static_cast<uint8_t>(*(encoding_ptr + 3)));
        instruction.operands_count = static_cast<uint8_t>(*(encoding_ptr + 4));

        size_t offset = 5;
        for (size_t i = 0; i < instruction.operands_count; i++) {
            instruction.operands[i].type = static_cast<operand_types>(*(encoding_ptr + offset));
            ++offset;

            instruction.operands[i].index = *(encoding_ptr + offset);
            ++offset;

            switch (instruction.operands[i].type) {
                case operand_types::register_sp:
                case operand_types::register_pc:
                case operand_types::register_flags:
                case operand_types::register_status:
                case operand_types::register_integer:
                case operand_types::increment_register_pre:
                case operand_types::decrement_register_pre:
                case operand_types::increment_register_post:
                case operand_types::decrement_register_post:
                case operand_types::register_floating_point:
                    break;
                case operand_types::increment_constant_pre:
                case operand_types::decrement_constant_pre:
                case operand_types::increment_constant_post:
                case operand_types::decrement_constant_post:
                case operand_types::constant_integer: {
                    uint64_t* constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                    instruction.operands[i].value.u64 = *constant_value_ptr;
                    offset += sizeof(uint64_t);
                    break;
                }
                case operand_types::constant_float: {
                    double* constant_value_ptr = reinterpret_cast<double*>(encoding_ptr + offset);
                    instruction.operands[i].value.d64 = *constant_value_ptr;
                    offset += sizeof(double);
                    break;
                }
            }
        }

        return size;
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

    size_t terp::align(uint64_t value, size_t size) const {
        auto offset = value % size;
        return offset ? value + (size - offset) : value;
    }

    std::string terp::disassemble(result& r, uint64_t address) {
        std::stringstream stream;
        while (true) {
            instruction_t inst;
            auto inst_size = decode_instruction(r, address, inst);
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