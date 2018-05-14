#include <fmt/format.h>
#include "terp.h"
#include "hex_formatter.h"

namespace basecode {

    terp::terp(uint32_t heap_size) : _heap_size(heap_size) {
    }

    terp::~terp() {
        delete _heap;
        _heap = nullptr;
    }

    void terp::reset() {
        _registers.pc = 0;
        _registers.fr = 0;
        _registers.sr = 0;
        _registers.sp = heap_size_in_qwords();

        for (size_t i = 0; i < 64; i++) {
            _registers.i[i] = 0;
            _registers.f[i] = 0.0;
        }

        _exited = false;
    }

    void terp::dump_state() {
        fmt::print("Basecode Interpreter State ----------------------------\n");

        fmt::print(
            "I0={:08x} | I1={:08x} | I2={:08x} | I3={:08x}\n",
            _registers.i[0],
            _registers.i[1],
            _registers.i[2],
            _registers.i[3]);

        fmt::print(
            "I4={:08x} | I5={:08x} | I6={:08x} | I7={:08x}\n",
            _registers.i[4],
            _registers.i[5],
            _registers.i[6],
            _registers.i[7]);

        fmt::print("-------------------------------------------------------\n");

        fmt::print(
            "PC={:08x} | SP={:08x} | FR={:08x} | SR={:08x}\n\n",
            _registers.pc,
            _registers.sp,
            _registers.fr,
            _registers.sr);
    }

    bool terp::step(result& r) {
        instruction_t inst;
        auto inst_size = decode_instruction(r, inst);
        if (inst_size == 0)
            return false;

        switch (inst.op) {
            case op_codes::nop: {
                fmt::print("nop\n");
                break;
            }
            case op_codes::load:
                break;
            case op_codes::store:
                break;
            case op_codes::move: {
                fmt::print("move\n");
                uint64_t source_value;
                if (!get_operand_value(r, inst, 0, source_value))
                    return false;
                if (!set_target_operand_value(r, inst, 1, source_value))
                    return false;
                break;
            }
            case op_codes::push: {
                fmt::print("push\n");
                uint64_t source_value;
                if (!get_operand_value(r, inst, 0, source_value))
                    return false;
                push(source_value);
                break;
            }
            case op_codes::pop: {
                fmt::print("pop\n");
                uint64_t value = pop();
                if (!set_target_operand_value(r, inst, 0, value))
                    return false;
                break;
            }
            case op_codes::add: {
                fmt::print("add\n");
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
                fmt::print("sub\n");
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
                fmt::print("mul\n");
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
                fmt::print("div\n");
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
                fmt::print("mod\n");
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
                break;
            }
            case op_codes::ret: {
                break;
            }
            case op_codes::jmp: {
                break;
            }
            case op_codes::meta: {
                break;
            }
            case op_codes::debug: {
                break;
            }
            case op_codes::exit: {
                fmt::print("exit\n");
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

        auto qword_address = address / sizeof(uint64_t);
        uint8_t size = 4;

        auto encoding_ptr = reinterpret_cast<uint8_t*>(_heap + qword_address);
        auto op_ptr = reinterpret_cast<uint16_t*>(encoding_ptr + 1);
        *op_ptr = static_cast<uint16_t>(instruction.op);

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

            if (instruction.operands[i].type == operand_types::constant) {
                uint64_t* constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                *constant_value_ptr = instruction.operands[i].value;
                offset += sizeof(uint64_t);
                size += sizeof(uint64_t);
            }
        }

        if (instruction.operands_count > 0)
            ++size;

        if (size < 8)
            size = 8;

        size = static_cast<uint8_t>(align(size, sizeof(uint64_t)));
        *encoding_ptr = size;

        return size;
    }

    size_t terp::decode_instruction(
            result& r,
            instruction_t& instruction) {
        if (_registers.pc % 8 != 0) {
            r.add_message("B003", "Instructions must be decoded on 8-byte boundaries.", true);
            return 0;
        }

        uint8_t* encoding_ptr = reinterpret_cast<uint8_t*>(&_heap[_registers.pc / sizeof(uint64_t)]);
        uint8_t size = *encoding_ptr;

        uint16_t* op_ptr = reinterpret_cast<uint16_t*>(encoding_ptr + 1);
        instruction.op = static_cast<op_codes>(*op_ptr);
        instruction.size = static_cast<op_sizes>(static_cast<uint8_t>(*(encoding_ptr + 3)));
        instruction.operands_count = static_cast<uint8_t>(*(encoding_ptr + 4));

        size_t offset = 5;
        for (size_t i = 0; i < instruction.operands_count; i++) {
            instruction.operands[i].type = static_cast<operand_types>(*(encoding_ptr + offset));
            ++offset;

            instruction.operands[i].index = *(encoding_ptr + offset);
            ++offset;

            instruction.operands[i].value = 0;
            if (instruction.operands[i].type == operand_types::constant) {
                uint64_t* constant_value_ptr = reinterpret_cast<uint64_t*>(encoding_ptr + offset);
                instruction.operands[i].value = *constant_value_ptr;
                offset += sizeof(uint64_t);
            }
        }

        _registers.pc += size;

        return size;
    }

    bool terp::has_exited() const {
        return _exited;
    }

    size_t terp::heap_size() const {
        return _heap_size;
    }

    bool terp::initialize(result& r) {
        _heap = new uint64_t[heap_size_in_qwords()];
        reset();
        return !r.is_failed();
    }

    uint64_t terp::pop() {
        uint64_t value = _heap[_registers.sp];
        _registers.sp += sizeof(uint64_t);
        return value;
    }

    void terp::push(uint64_t value) {
        _registers.sp -= sizeof(uint64_t);
        _heap[_registers.sp] = value;
        return;
    }

    size_t terp::heap_size_in_qwords() const {
        return _heap_size / sizeof(uint64_t);
    }

    const register_file_t& terp::register_file() const {
        return _registers;
    }

    void terp::dump_heap(uint64_t address, size_t size) {
        auto program_memory = basecode::hex_formatter::dump_to_string(
            reinterpret_cast<const void*>(_heap),
            size);
        fmt::print("{}\n", program_memory);
    }

    size_t terp::align(uint64_t value, size_t size) const {
        auto offset = value % size;
        return offset ? value + (size - offset) : value;
    }

    bool terp::get_operand_value(
            result& r,
            const instruction_t& instruction,
            uint8_t operand_index,
            double& value) const {
        switch (instruction.operands[operand_index].type) {
            case operand_types::register_floating_point:
                value = _registers.f[instruction.operands[operand_index].index];
                break;
            case operand_types::register_integer:
            case operand_types::register_sp:
            case operand_types::register_pc:
            case operand_types::register_flags:
            case operand_types::register_status:
                r.add_message(
                    "B005",
                    "integer registers cannot be used for floating point operands.",
                    true);
                break;
            case operand_types::constant:
                value = instruction.operands[operand_index].value;
                break;
        }

        return true;
    }

    bool terp::get_operand_value(
            result& r,
            const instruction_t& instruction,
            uint8_t operand_index,
            uint64_t& value) const {
        switch (instruction.operands[operand_index].type) {
            case operand_types::register_integer:
                value = _registers.i[instruction.operands[operand_index].index];
                break;
            case operand_types::register_floating_point:
                r.add_message(
                    "B005",
                    "integer registers don't support floating point values.",
                    true);
                break;
            case operand_types::register_sp:
                value = _registers.sp;
                break;
            case operand_types::register_pc:
                value = _registers.pc;
                break;
            case operand_types::register_flags:
                value = _registers.fr;
                break;
            case operand_types::register_status:
                value = _registers.sr;
                break;
            case operand_types::constant:
                value = instruction.operands[operand_index].value;
                break;
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
            case operand_types::register_integer:
                _registers.i[instruction.operands[operand_index].index] = value;
                return true;
            case operand_types::register_floating_point:
                r.add_message(
                    "B009",
                    "floating point registers cannot be the target for integer values.",
                    true);
                break;
            case operand_types::register_sp:
                _registers.sp = value;
                break;
            case operand_types::register_pc:
                _registers.pc = value;
                break;
            case operand_types::register_flags:
                _registers.fr = value;
                break;
            case operand_types::register_status:
                _registers.sr = value;
                break;
            case operand_types::constant:
                r.add_message(
                    "B006",
                    "constant cannot be a target operand type.",
                    true);
                break;
        }

        return false;
    }

    bool terp::set_target_operand_value(
            result& r,
            const instruction_t& instruction,
            uint8_t operand_index,
            double value) {
        switch (instruction.operands[operand_index].type) {
            case operand_types::register_sp:
            case operand_types::register_pc:
            case operand_types::register_flags:
            case operand_types::register_status:
            case operand_types::register_integer:
                r.add_message(
                    "B009",
                    "integer registers cannot be the target for floating point values.",
                    true);
                break;
            case operand_types::register_floating_point:
                _registers.f[instruction.operands[operand_index].index] = value;
                break;
            case operand_types::constant:
                r.add_message(
                    "B006",
                    "constant cannot be a target operand type.",
                    true);
                break;
        }

        return false;
    }

};