#pragma once

#include <map>
#include <string>
#include <cstdint>
#include "result.h"

namespace basecode {

    struct register_file_t {
        enum flags_t : uint64_t {
            zero     = 0b0000000000000000000000000000000000000000000000000000000000000001,
            carry    = 0b0000000000000000000000000000000000000000000000000000000000000010,
            overflow = 0b0000000000000000000000000000000000000000000000000000000000000100,
            negative = 0b0000000000000000000000000000000000000000000000000000000000001000,
            extended = 0b0000000000000000000000000000000000000000000000000000000000010000,
        };

        bool flags(flags_t f) const {
            return (fr & f) != 0;
        }

        void flags(flags_t f, bool value) {
            if (value)
                fr |= f;
            else
                fr &= ~f;
        }

        uint64_t i[64];
        double f[64];
        uint64_t pc;
        uint64_t sp;
        uint64_t fr;
        uint64_t sr;
    };

    enum class op_codes : uint8_t {
        nop = 1,
        load,
        store,
        copy,
        fill,
        move,
        push,
        pop,
        dup,
        inc,
        dec,
        add,
        sub,
        mul,
        div,
        mod,
        neg,
        shr,
        shl,
        ror,
        rol,
        and_op,
        or_op,
        xor_op,
        not_op,
        bis,
        bic,
        test,
        cmp,
        bz,
        bnz,
        tbz,
        tbnz,
        bne,
        beq,
        bg,
        bl,
        bge,
        ble,
        jsr,
        rts,
        jmp,
        swi,
        trap,
        meta,
        exit
    };

    // consider refactoring op_sizes and operand_types
    //  into flags.  it probably still needs to be a 16-bit field
    //  to fit everything.
    //
    // byte     = 0b0000_0001
    // word     = 0b0000_0010
    // dword    = 0b0000_0100
    // qword    = 0b0000_1000
    // register = 0b0001_0000
    // negative = 0b0010_0000
    // inc      = 0b0100_0000
    // dec      = 0b1000_0000

    enum class op_sizes : uint8_t {
        none,
        byte,
        word,
        dword,
        qword
    };

    enum class operand_types : uint8_t {
        register_integer = 1,
        register_floating_point,
        register_sp,
        register_pc,
        register_flags,
        register_status,
        constant_integer,
        constant_float,
        constant_offset_positive,
        constant_offset_negative,
        increment_constant_pre,
        increment_constant_post,
        increment_register_pre,
        increment_register_post,
        decrement_constant_pre,
        decrement_constant_post,
        decrement_register_pre,
        decrement_register_post
    };

    struct operand_encoding_t {
        operand_types type = operand_types::register_integer;
        uint8_t index = 0;
        union {
            uint64_t u64;
            double d64;
        } value;
    };

    struct instruction_t {
        static const size_t base_size = 4;

        size_t decode(
            result& r,
            uint8_t* heap,
            uint64_t address);

        size_t encode(
            result& r,
            uint8_t* heap,
            uint64_t address);

        size_t encoding_size() const;

        size_t align(uint64_t value, size_t size) const;

        void patch_branch_address(uint64_t address, uint8_t index = 0);

        op_codes op = op_codes::nop;
        op_sizes size = op_sizes::none;
        uint8_t operands_count = 0;
        operand_encoding_t operands[4];
    };

    struct debug_information_t {
        uint32_t line_number;
        uint16_t column_number;
        std::string symbol;
        std::string source_file;
    };

    struct icache_entry_t {
        size_t size;
        instruction_t inst;
    };

    class terp;

    class instruction_cache {
    public:
        explicit instruction_cache(terp* terp);

        void reset();

        size_t fetch_at(
            result& r,
            uint64_t address,
            instruction_t& inst);

        size_t fetch(result& r, instruction_t& inst);

    private:
        terp* _terp = nullptr;
        std::map<uint64_t, icache_entry_t> _cache {};
    };

    class terp {
    public:
        using trap_callable = std::function<void (terp*)>;

        static constexpr size_t interrupt_vector_table_start = 0;
        static constexpr size_t interrupt_vector_table_size = 16;
        static constexpr size_t interrupt_vector_table_end = sizeof(uint64_t) * interrupt_vector_table_size;
        static constexpr size_t program_start = interrupt_vector_table_end;

        explicit terp(size_t heap_size);

        virtual ~terp();

        void reset();

        uint64_t pop();

        bool step(result& r);

        uint64_t peek() const;

        bool has_exited() const;

        inline uint8_t* heap() {
            return _heap;
        }

        size_t heap_size() const;

        void push(uint64_t value);

        bool initialize(result& r);

        void remove_trap(uint8_t index);

        void dump_state(uint8_t count = 16);

        void swi(uint8_t index, uint64_t address);

        const register_file_t& register_file() const;

        void dump_heap(uint64_t offset, size_t size = 256);

        std::string disassemble(result& r, uint64_t address);

        std::string disassemble(const instruction_t& inst) const;

        void register_trap(uint8_t index, const trap_callable& callable);

    protected:
        bool set_target_operand_value(
            result& r,
            const instruction_t& instruction,
            uint8_t operand_index,
            uint64_t value);

        bool set_target_operand_value(
            result& r,
            const instruction_t& instruction,
            uint8_t operand_index,
            double value);

        bool get_operand_value(
            result& r,
            const instruction_t& instruction,
            uint8_t operand_index,
            uint64_t& value) const;

        bool get_operand_value(
            result& r,
            const instruction_t& instruction,
            uint8_t operand_index,
            double& value) const;

        inline uint8_t op_size_in_bytes(op_sizes size) const {
            switch (size) {
                case op_sizes::none:  return 0;
                case op_sizes::byte:  return 1;
                case op_sizes::word:  return 2;
                case op_sizes::dword: return 4;
                case op_sizes::qword: return 8;
            }
        }

    private:
        inline uint8_t* byte_ptr(uint64_t address) const {
            return _heap + address;
        }

        inline uint16_t* word_ptr(uint64_t address) const {
            return reinterpret_cast<uint16_t*>(_heap + address);
        }

        inline uint64_t* qword_ptr(uint64_t address) const {
            return reinterpret_cast<uint64_t*>(_heap + address);
        }

        inline uint32_t* dword_ptr(uint64_t address) const {
            return reinterpret_cast<uint32_t*>(_heap + address);
        }

    private:
        inline static std::map<op_codes, std::string> s_op_code_names = {
            {op_codes::nop,    "NOP"},
            {op_codes::load,   "LOAD"},
            {op_codes::store,  "STORE"},
            {op_codes::copy,   "COPY"},
            {op_codes::fill,   "FILL"},
            {op_codes::move,   "MOVE"},
            {op_codes::push,   "PUSH"},
            {op_codes::pop,    "POP"},
            {op_codes::dup,    "DUP"},
            {op_codes::inc,    "INC"},
            {op_codes::dec,    "DEC"},
            {op_codes::add,    "ADD"},
            {op_codes::sub,    "SUB"},
            {op_codes::mul,    "MUL"},
            {op_codes::div,    "DIV"},
            {op_codes::mod,    "MOD"},
            {op_codes::neg,    "NEG"},
            {op_codes::shr,    "SHR"},
            {op_codes::shl,    "SHL"},
            {op_codes::ror,    "ROR"},
            {op_codes::rol,    "ROL"},
            {op_codes::and_op, "AND"},
            {op_codes::or_op,  "OR"},
            {op_codes::xor_op, "XOR"},
            {op_codes::not_op, "NOT"},
            {op_codes::bis,    "BIS"},
            {op_codes::bic,    "BIC"},
            {op_codes::test,   "TEST"},
            {op_codes::cmp,    "CMP"},
            {op_codes::bz,     "BZ"},
            {op_codes::bnz,    "BNZ"},
            {op_codes::tbz,    "TBZ"},
            {op_codes::tbnz,   "TBNZ"},
            {op_codes::bne,    "BNE"},
            {op_codes::beq,    "BEQ"},
            {op_codes::bg,     "BG"},
            {op_codes::bge,    "BGE"},
            {op_codes::bl,     "BL"},
            {op_codes::ble,    "BLE"},
            {op_codes::jsr,    "JSR"},
            {op_codes::rts,    "RTS"},
            {op_codes::jmp,    "JMP"},
            {op_codes::swi,    "SWI"},
            {op_codes::trap,   "TRAP"},
            {op_codes::meta,   "META"},
            {op_codes::exit,   "EXIT"},
        };

        bool _exited = false;
        size_t _heap_size = 0;
        uint8_t* _heap = nullptr;
        instruction_cache _icache;
        register_file_t _registers {};
        std::map<uint8_t, trap_callable> _traps {};
    };

};