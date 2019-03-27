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

#include <map>
#include <stack>
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <boost/any.hpp>
#include <common/result.h>
#include <common/id_pool.h>
#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

typedef struct DLLib_ DLLib;
typedef struct DCstruct_ DCstruct;
typedef struct DCCallVM_ DCCallVM;

namespace basecode::vm {

    class ffi;
    class terp;
    class label;
    class symbol;
    class label_map;
    class assembler;
    class basic_block;
    class basic_block_map;
    class assembly_parser;
    class assembly_listing;
    class register_allocator;

    using symbol_list_t = std::vector<symbol*>;
    using basic_block_list_t = std::vector<basic_block*>;
    using basic_block_stack_t = std::stack<basic_block*>;
    using symbol_address_map = std::unordered_map<std::string, void*>;
    using id_resolve_callable = std::function<std::string (uint64_t)>;

    ///////////////////////////////////////////////////////////////////////////

    enum class op_sizes : uint8_t {
        none,
        byte,
        word,
        dword,
        qword
    };

    static inline std::string op_size_format_spec(op_sizes size) {
        switch (size) {
            case op_sizes::byte:
                return "#${:02X}";
            case op_sizes::word:
                return "#${:04X}";
            case op_sizes::dword:
                return "#${:08X}";
            default:
            case op_sizes::qword:
                return "#${:016X}";
        }
    }

    static inline uint8_t op_size_in_bytes(op_sizes size) {
        switch (size) {
            case op_sizes::byte:  return 1;
            case op_sizes::word:  return 2;
            case op_sizes::dword: return 4;
            case op_sizes::qword: return 8;
            default:              return 0;
        }
    }

    static inline op_sizes op_size_for_byte_size(size_t size) {
        switch (size) {
            case 1:     return op_sizes::byte;
            case 2:     return op_sizes::word;
            case 4:     return op_sizes::dword;
            case 8:     return op_sizes::qword;
            default:    return op_sizes::none;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    enum class register_type_t : uint8_t {
        none,
        pc,
        sp,
        fp,
        fr,
        sr,
        integer,
        floating_point,
    };

    enum registers_t : uint8_t {
        r0,
        r1,
        r2,
        r3,
        r4,
        r5,
        r6,
        r7,
        r8,
        r9,

        r10,
        r11,
        r12,
        r13,
        r14,
        r15,
        r16,
        r17,
        r18,
        r19,

        r20,
        r21,
        r22,
        r23,
        r24,
        r25,
        r26,
        r27,
        r28,
        r29,

        r30,
        r31,
        r32,
        r33,
        r34,
        r35,
        r36,
        r37,
        r38,
        r39,

        r40,
        r41,
        r42,
        r43,
        r44,
        r45,
        r46,
        r47,
        r48,
        r49,

        r50,
        r51,
        r52,
        r53,
        r54,
        r55,
        r56,
        r57,
        r58,
        r59,

        r60,
        r61,
        r62,
        r63,

        pc,
        sp,
        fr,
        sr,
        fp
    };

    union register_value_alias_t {
        uint8_t  b;
        uint16_t w;
        uint32_t dw;
        float    dwf;
        uint64_t qw;
        double   qwf;
    };

    struct register_t {
        static register_t pc() {
            return register_t {
                .size = op_sizes::qword,
                .number = registers_t::pc,
                .type = register_type_t::integer,
            };
        }

        static register_t sp() {
            return register_t {
                .size = op_sizes::qword,
                .number = registers_t::sp,
                .type = register_type_t::integer,
            };
        }

        static register_t fp() {
            return register_t {
                .size = op_sizes::qword,
                .number = registers_t::fp,
                .type = register_type_t::integer,
            };
        }

        static register_t empty() {
            return register_t {
                .size = op_sizes::qword,
                .number = registers_t::r0,
                .type = register_type_t::none,
            };
        }

        op_sizes size = op_sizes::qword;
        registers_t number = registers_t::r0;
        register_type_t type = register_type_t::none;
        register_value_alias_t value {
            .qw = 0
        };
    };

    static constexpr const uint32_t register_integer_start   = 0;
    static constexpr const uint32_t number_integer_registers = 64;
    static constexpr const uint32_t register_float_start     = number_integer_registers;
    static constexpr const uint32_t number_float_registers   = 64;
    static constexpr const uint32_t number_special_registers = 5;
    static constexpr const uint32_t number_general_purpose_registers = number_integer_registers + number_float_registers;
    static constexpr const uint32_t number_total_registers   = number_integer_registers
                                                               + number_float_registers
                                                               + number_special_registers;
    static constexpr const uint32_t register_special_start   = number_integer_registers
                                                               + number_float_registers;
    static constexpr const uint32_t register_pc = register_special_start;
    static constexpr const uint32_t register_sp = register_special_start + 1;
    static constexpr const uint32_t register_fr = register_special_start + 2;
    static constexpr const uint32_t register_sr = register_special_start + 3;
    static constexpr const uint32_t register_fp = register_special_start + 4;

    static inline size_t register_index(registers_t r, register_type_t type) {
        switch (r) {
            case pc: return register_pc;
            case sp: return register_sp;
            case fr: return register_fr;
            case sr: return register_sr;
            case fp: return register_fp;
            default:
                if (type == register_type_t::floating_point)
                    return register_float_start + static_cast<uint8_t>(r);
                else
                    return static_cast<uint8_t>(r);
        }
    }

    struct register_file_t {
        enum flags_t : uint64_t {
            zero     = 0b0000000000000000000000000000000000000000000000000000000000000001,
            carry    = 0b0000000000000000000000000000000000000000000000000000000000000010,
            overflow = 0b0000000000000000000000000000000000000000000000000000000000000100,
            negative = 0b0000000000000000000000000000000000000000000000000000000000001000,
            extended = 0b0000000000000000000000000000000000000000000000000000000000010000,
            subtract = 0b0000000000000000000000000000000000000000000000000000000000100000,
        };

        inline void set_flags(
                bool zero,
                bool carry,
                bool overflow,
                bool negative,
                bool subtract = false) {
            auto& flags = r[register_fr];
            if (zero)
                flags.qw |= flags_t::zero;
            else
                flags.qw &= ~flags_t::zero;

            if (carry)
                flags.qw |= flags_t::carry;
            else
                flags.qw &= ~flags_t::carry;

            if (overflow)
                flags.qw |= flags_t::overflow;
            else
                flags.qw &= ~flags_t::overflow;

            if (negative)
                flags.qw |= flags_t::negative;
            else
                flags.qw &= ~flags_t::negative;

            if (subtract)
                flags.qw |= flags_t::subtract;
            else
                flags.qw &= ~flags_t::subtract;
        }

        inline bool flags(flags_t f) const {
            return (r[register_fr].qw & f) != 0;
        }

        inline void flags(flags_t f, bool value) {
            if (value)
                r[register_fr].qw |= f;
            else
                r[register_fr].qw &= ~f;
        }

        register_value_alias_t r[number_total_registers];
    };

    enum class op_codes : uint8_t {
        nop = 1,
        alloc,
        free,
        size,
        load,
        store,
        copy,
        convert,
        fill,
        clr,
        move,
        moves,
        movez,
        push,
        pushm,
        pop,
        popm,
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
        pow,
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
        bs,
        bo,
        bcc,
        bcs,
        ba,
        bae,
        bb,
        bbe,
        bg,
        bl,
        bge,
        ble,
        seta,
        setna,
        setae,
        setnae,
        setb,
        setnb,
        setbe,
        setnbe,
        setc,
        setnc,
        setg,
        setng,
        setge,
        setnge,
        setl,
        setnl,
        setle,
        setnle,
        sets,
        setns,
        seto,
        setno,
        setz,
        setnz,
        jsr,
        rts,
        jmp,
        swi,
        swap,
        trap,
        ffi,
        meta,
        exit,
    };

    inline static std::map<op_codes, std::string> s_op_code_names = {
        {op_codes::nop,    "NOP"},
        {op_codes::alloc,  "ALLOC"},
        {op_codes::free,   "FREE"},
        {op_codes::size,   "SIZE"},
        {op_codes::load,   "LOAD"},
        {op_codes::store,  "STORE"},
        {op_codes::copy,   "COPY"},
        {op_codes::convert,"CVRT"},
        {op_codes::fill,   "FILL"},
        {op_codes::clr,    "CLR"},
        {op_codes::move,   "MOVE"},
        {op_codes::moves,  "MOVES"},
        {op_codes::movez,  "MOVEZ"},
        {op_codes::push,   "PUSH"},
        {op_codes::pushm,  "PUSHM"},
        {op_codes::pop,    "POP"},
        {op_codes::popm,   "POPM"},
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
        {op_codes::pow,    "POW"},
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
        {op_codes::bcc,    "BCC"},
        {op_codes::bcs,    "BCS"},
        {op_codes::bs,     "BS"},
        {op_codes::bo,     "BO"},
        {op_codes::ba,     "BA"},
        {op_codes::bae,    "BAE"},
        {op_codes::bb,     "BB"},
        {op_codes::bbe,    "BBE"},
        {op_codes::bg,     "BG"},
        {op_codes::bge,    "BGE"},
        {op_codes::bl,     "BL"},
        {op_codes::ble,    "BLE"},
        {op_codes::seta,   "SETA"},
        {op_codes::setna,  "SETNA"},
        {op_codes::setae,  "SETAE"},
        {op_codes::setnae, "SETNAE"},
        {op_codes::setb,   "SETB"},
        {op_codes::setnb,  "SETNB"},
        {op_codes::setbe,  "SETBE"},
        {op_codes::setnbe, "SETNBE"},
        {op_codes::setc,   "SETC"},
        {op_codes::setnc,  "SETNC"},
        {op_codes::setg,   "SETG"},
        {op_codes::setng,  "SETNG"},
        {op_codes::setge,  "SETGE"},
        {op_codes::setnge, "SETNGE"},
        {op_codes::setl,   "SETL"},
        {op_codes::setnl,  "SETNL"},
        {op_codes::setle,  "SETLE"},
        {op_codes::setnle, "SETNLE"},
        {op_codes::sets,   "SETS"},
        {op_codes::setns,  "SETNS"},
        {op_codes::seto,   "SETO"},
        {op_codes::setno,  "SETNO"},
        {op_codes::setz,   "SETZ"},
        {op_codes::setnz,  "SETNZ"},
        {op_codes::jsr,    "JSR"},
        {op_codes::rts,    "RTS"},
        {op_codes::jmp,    "JMP"},
        {op_codes::swi,    "SWI"},
        {op_codes::swap,   "SWAP"},
        {op_codes::trap,   "TRAP"},
        {op_codes::ffi,    "FFI"},
        {op_codes::meta,   "META"},
        {op_codes::exit,   "EXIT"},
    };

    inline static std::string op_code_name(op_codes type) {
        const auto it = s_op_code_names.find(type);
        if (it != s_op_code_names.end()) {
            return it->second;
        }
        return "";
    }

    ///////////////////////////////////////////////////////////////////////////

    struct assembler_local_t {
        register_t reg;
        std::string name;
        int64_t offset = 0;
    };

    enum class assembler_named_ref_type_t : uint8_t {
        none,
        label,
        local,
        offset,
    };

    struct assembler_named_ref_t {
        std::string name;
        vm::op_sizes size = vm::op_sizes::qword;
        assembler_named_ref_type_t type = assembler_named_ref_type_t::none;
    };

    struct fixup_t {
        int64_t offset = 0;
        assembler_named_ref_t* named_ref = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    union operand_value_alias_t {
        uint8_t  r;
        uint64_t u;
        int64_t  s;
        float    f;
        double   d;
    };

    struct operand_encoding_t {
        using flags_t = uint8_t;

        enum flags : uint8_t {
            none        = 0b00000000,
            constant    = 0b00000000,
            reg         = 0b00000001,
            integer     = 0b00000010,
            negative    = 0b00000100,
            range       = 0b00001000,
            dword       = 0b00010000,
            word        = 0b00100000,
            byte        = 0b01000000,
        };

        void size_to_flags() {
            switch (size) {
                case op_sizes::byte:
                    type |= operand_encoding_t::flags::byte;
                    break;
                case op_sizes::word:
                    type |= operand_encoding_t::flags::word;
                    break;
                case op_sizes::dword:
                    type |= operand_encoding_t::flags::dword;
                    break;
                default:
                    break;
            }
        }

        void size_from_flags() {
            if ((type & flags::byte) == flags::byte)
                size = op_sizes::byte;
            else if ((type & flags::word) == flags::word)
                size = op_sizes::word;
            else if ((type & flags::dword) == flags::dword)
                size = op_sizes::dword;
            else
                size = op_sizes::qword;
        }

        inline bool is_reg() const {
            return (type & flags::reg) == flags::reg;
        }

        inline bool is_range() const {
            return (type & flags::range) == flags::range;
        }

        inline bool is_integer() const {
            return (type & flags::integer) == flags::integer;
        }

        inline bool is_negative() const {
            return (type & flags::negative) == flags::negative;
        }

        op_sizes size = op_sizes::qword;
        flags_t type = flags::reg | flags::integer;
        operand_value_alias_t value {};

        fixup_t fixup[2];
    };

    ///////////////////////////////////////////////////////////////////////////

    struct instruction_t {
        static constexpr size_t base_size = 3;
        static constexpr size_t alignment = 4;

        size_t decode(
            common::result& r,
            uint64_t address);

        size_t encode(
            common::result& r,
            uint64_t address);

        size_t encoding_size() const;

        std::string disassemble() const;

        size_t align(uint64_t value, size_t size) const;

        void patch_branch_address(uint64_t address, uint8_t index = 0);

        op_codes op = op_codes::nop;
        op_sizes size = op_sizes::none;
        uint8_t operands_count = 0;
        operand_encoding_t operands[4];
    };

    struct meta_information_t {
        uint32_t line_number;
        uint16_t column_number;
        std::string symbol;
        std::string source_file;
    };

    ///////////////////////////////////////////////////////////////////////////

    struct dtt_slot_value_t {
        op_sizes size;
        bool is_range = false;
        bool is_integer = false;
        bool is_register = false;
        bool is_negative = false;
        union {
            register_value_alias_t* r;
            register_value_alias_t  d;
        } data {.r = nullptr};
    };

    struct dtt_slot_t {
        op_sizes size;
        uint64_t address;
        op_codes op_code;
        size_t encoding_size = 0;
        uint8_t values_count = 0;
        dtt_slot_value_t values[4];
        const void* handler = nullptr;
        dtt_slot_t* branch_target = nullptr;
    };

    struct dtt_t {
        std::vector<dtt_slot_t> slots {};
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class basic_block_type_t {
        none,
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class section_t : uint8_t {
        unknown,
        bss,
        ro_data,
        data,
        text
    };

    inline static std::string section_name(section_t type) {
        switch (type) {
            case section_t::bss:    return "bss";
            case section_t::ro_data:return "ro_data";
            case section_t::data:   return "data";
            case section_t::text:   return "text";
            default:                return "unknown";
        }
    }

    inline static section_t section_type(const std::string& name) {
        if (name == "bss")      return section_t::bss;
        if (name == "ro_data")  return section_t::ro_data;
        if (name == "data")     return section_t::data;
        if (name == "text")     return section_t::text;
        return section_t::unknown;
    }

    ///////////////////////////////////////////////////////////////////////////

    enum data_definition_type_t : uint8_t {
        none,
        initialized,
        uninitialized
    };

    ///////////////////////////////////////////////////////////////////////////

    struct align_t {
        uint8_t size = 0;
    };

    using data_value_variant_t = boost::variant<uint64_t, assembler_named_ref_t*>;
    struct data_definition_t {
        op_sizes size;
        data_definition_type_t type = data_definition_type_t::none;
        std::vector<data_value_variant_t> values {};
    };

    enum class comment_location_t : uint8_t {
        new_line,
        after_instruction
    };

    struct comment_t {
        uint8_t indent {};
        std::string value {};
        comment_location_t location = comment_location_t::new_line;
    };

    struct label_t {
        vm::label* instance = nullptr;
    };

    enum class local_type_t {
        integer,
        floating_point
    };

    struct local_t {
        int64_t offset;
        local_type_t type;
        std::string name {};
        std::string frame_offset {};
    };

    using local_list_t = std::vector<const local_t*>;

    struct frame_offset_t {
        int64_t offset;
        std::string name {};
    };

    struct meta_t {
        std::string label {};
    };

    struct reset_t {
        std::string type {};
    };

    enum class block_entry_type_t : uint8_t {
        section = 1,
        meta,
        reset,
        label,
        local,
        align,
        comment,
        blank_line,
        instruction,
        frame_offset,
        data_definition,
    };

    struct instruction_t;

    struct block_entry_t {
        block_entry_t();

        block_entry_t(const block_entry_t& other);

        explicit block_entry_t(const meta_t& meta);

        explicit block_entry_t(const label_t& label);

        explicit block_entry_t(const local_t& local);

        explicit block_entry_t(const align_t& align);

        explicit block_entry_t(const reset_t& reset);

        explicit block_entry_t(const comment_t& comment);

        explicit block_entry_t(const section_t& section);

        explicit block_entry_t(const frame_offset_t& offset);

        explicit block_entry_t(const data_definition_t& data);

        explicit block_entry_t(const instruction_t& instruction);

        template <typename T>
        T* data() {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        template <typename T>
        const T* data() const {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        uint64_t address() const;

        block_entry_type_t type() const;

        block_entry_t* address(uint64_t value);

    private:
        boost::any _data;
        uint64_t _address = 0;
        block_entry_type_t _type;
    };

    using block_entry_list_t = std::vector<block_entry_t>;

    ///////////////////////////////////////////////////////////////////////////

    enum class instruction_operand_type_t : uint8_t {
        empty,
        reg,
        named_ref,
        imm_f32,
        imm_f64,
        imm_uint,
        imm_sint,
        reg_range,
        named_ref_range
    };

    struct register_range_t {
        register_t begin {};
        register_t end {};
    };

    struct named_ref_with_offset_t {
        int64_t offset = 0;
        assembler_named_ref_t* ref = nullptr;
    };

    struct named_ref_range_t {
        assembler_named_ref_t* begin = nullptr;
        assembler_named_ref_t* end = nullptr;
    };

    struct instruction_operand_t {
        static instruction_operand_t fp();

        static instruction_operand_t sp();

        static instruction_operand_t pc();

        static instruction_operand_t empty();

        static instruction_operand_t offset(
            int64_t value,
            op_sizes size = op_sizes::byte);

        instruction_operand_t();

        explicit instruction_operand_t(
            int64_t immediate,
            op_sizes size = op_sizes::qword);

        explicit instruction_operand_t(
            uint64_t immediate,
            op_sizes size = op_sizes::qword);

        explicit instruction_operand_t(
            assembler_named_ref_t* ref,
            int64_t offset = 0);

        explicit instruction_operand_t(register_t reg);

        explicit instruction_operand_t(float immediate);

        explicit instruction_operand_t(double immediate);

        explicit instruction_operand_t(const register_range_t& range);

        explicit instruction_operand_t(const named_ref_range_t& range);

        bool is_empty() const {
            return _type == instruction_operand_type_t::empty;
        }

        op_sizes size() const {
            return _size;
        }

        void size(op_sizes size) {
            _size = size;
        }

        instruction_operand_type_t type() const {
            return _type;
        }

        template <typename T>
        T* data() {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        template <typename T>
        const T* data() const {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

    private:
        boost::any _data;
        op_sizes _size = op_sizes::qword;
        instruction_operand_type_t _type;
    };

    using instruction_operand_list_t = std::vector<instruction_operand_t>;

    ///////////////////////////////////////////////////////////////////////////

    enum class listing_source_line_type_t : uint8_t {
        blank,
        label,
        comment,
        directive,
        instruction,
        data_definition
    };

    struct listing_source_line_t {
        uint64_t address = 0;
        std::string source {};
        listing_source_line_type_t type;
    };

    struct listing_source_file_t {
        void add_source_line(
            listing_source_line_type_t type,
            uint64_t address,
            const std::string& source);

        void add_blank_lines(
            uint64_t address,
            uint16_t count = 1);

        boost::filesystem::path path;
        std::vector<listing_source_line_t> lines {};
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class symbol_type_t {
        unknown,
        u8,
        u16,
        u32,
        u64,
        f32,
        f64,
        bytes
    };

    static inline std::unordered_map<symbol_type_t, std::string> s_symbol_type_names = {
        {symbol_type_t::unknown, "unknown"},
        {symbol_type_t::u8,      "u8"},
        {symbol_type_t::u16,     "u16"},
        {symbol_type_t::u32,     "u32"},
        {symbol_type_t::u64,     "u64"},
        {symbol_type_t::f32,     "f32"},
        {symbol_type_t::f64,     "f64"},
        {symbol_type_t::bytes,   "bytes"},
    };

    static inline std::string symbol_type_name(symbol_type_t type) {
        auto it = s_symbol_type_names.find(type);
        if (it == s_symbol_type_names.end())
            return "unknown";
        return it->second;
    }

    static inline size_t size_of_symbol_type(symbol_type_t type) {
        switch (type) {
            case symbol_type_t::u8:    return 1;
            case symbol_type_t::u16:   return 2;
            case symbol_type_t::u32:   return 4;
            case symbol_type_t::u64:   return 8;
            case symbol_type_t::f32:   return 4;
            case symbol_type_t::f64:   return 8;
            default:
                return 0;
        }
    }

    static inline symbol_type_t float_symbol_type_for_size(size_t size) {
        switch (size) {
            case 4:
                return symbol_type_t::f32;
            case 8:
                return symbol_type_t::f64;
            default:
                return symbol_type_t::unknown;
        }
    }

    static inline symbol_type_t integer_symbol_type_for_size(size_t size) {
        switch (size) {
            case 1:
                return symbol_type_t::u8;
            case 2:
                return symbol_type_t::u16;
            case 4:
                return symbol_type_t::u32;
            case 8:
                return symbol_type_t::u64;
            default:
                return symbol_type_t::unknown;
        }
    }

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
            range            = 0b01000000,
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
            "CLR",
            mnemonic_t{
                op_codes::clr,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true},
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
            "PUSHM",
            mnemonic_t{
                op_codes::pushm,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register | mnemonic_operand_t::flags::range, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register | mnemonic_operand_t::flags::range, false},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register | mnemonic_operand_t::flags::range, false},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register | mnemonic_operand_t::flags::range, false},
                }
            }
        },
        {
            "POP",
            mnemonic_t{
                op_codes::pop,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register, true},
                }
            }
        },
        {
            "POPM",
            mnemonic_t{
                op_codes::popm,
                {
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register | mnemonic_operand_t::flags::range, true},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register | mnemonic_operand_t::flags::range, false},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register | mnemonic_operand_t::flags::range, false},
                    {mnemonic_operand_t::flags::integer_register | mnemonic_operand_t::flags::float_register | mnemonic_operand_t::flags::range, false},
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
            "POW",
            mnemonic_t{
                op_codes::pow,
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
        meta,
        reset,
        ilocal,
        flocal,
        block,
        end,
        frame_offset,
        db,
        dw,
        dd,
        dq,
        rb,
        rw,
        rd,
        rq
    };

    using directive_param_variant_t = boost::variant<std::string, uint64_t, int64_t>;

    struct directive_param_t {
        enum flags : uint8_t {
            none        = 0b00000000,
            string      = 0b00000001,
            number      = 0b00000010,
            symbol      = 0b00000100,
            repeating   = 0b10000000,
        };

        bool is_number() const {
            return (type & flags::number) != 0;
        }

        bool is_string() const {
            return (type & flags::string) != 0;
        }

        bool is_symbol() const {
            return (type & flags::symbol) != 0;
        }

        bool is_repeating() const {
            return (type & flags::repeating) != 0;
        }

        uint8_t type = flags::none;
        bool required = false;
    };

    struct directive_t {
        size_t required_operand_count() const {
            size_t count = 0;
            for (const auto& op : params)
                count += op.required ? 1 : 0;
            return count;
        }

        op_sizes size;
        directive_type_t type;
        std::vector<directive_param_t> params {};
    };

    inline static std::map<std::string, directive_t> s_directives = {
        {
            "END",
            directive_t{
                op_sizes::none,
                directive_type_t::end,
                {}
            }
        },
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
            "META",
            directive_t{
                op_sizes::none,
                directive_type_t::meta,
                {
                    {directive_param_t::flags::string, true},
                }
            }
        },
        {
            "BLOCK",
            directive_t{
                op_sizes::none,
                directive_type_t::block,
                {
                    {directive_param_t::flags::number, true},
                }
            }
        },
        {
            "RESET",
            directive_t{
                op_sizes::none,
                directive_type_t::reset,
                {
                    {directive_param_t::flags::string, true},
                }
            }
        },
        {
            "ILOCAL",
            directive_t{
                op_sizes::none,
                directive_type_t::ilocal,
                {
                    {directive_param_t::flags::symbol, true},
                }
            }
        },
        {
            "FLOCAL",
            directive_t{
                op_sizes::none,
                directive_type_t::flocal,
                {
                    {directive_param_t::flags::symbol, true},
                }
            }
        },
        {
            "FRAME",
            directive_t{
                op_sizes::none,
                directive_type_t::frame_offset,
                {
                    {directive_param_t::flags::string, true},
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

    enum class ffi_calling_mode_t : uint16_t {
        c_default = 1,
        c_ellipsis,
        c_ellipsis_varargs,
    };

    enum class ffi_types_t : uint16_t {
        void_type = 1,
        bool_type,
        char_type,
        short_type,
        int_type,
        long_type,
        long_long_type,
        float_type,
        double_type,
        pointer_type,
        struct_type,
        any_type
    };

    ///////////////////////////////////////////////////////////////////////////

    struct shared_library_t {
        shared_library_t();

        ~shared_library_t();

        bool initialize(
            common::result& r,
            const boost::filesystem::path& path);

        bool self_loaded() const {
            return _self_loaded;
        }

        void self_loaded(bool value) {
            _self_loaded = value;
        }

        bool initialize(common::result& r);

        inline const symbol_address_map& symbols() const {
            return _symbols;
        }

        bool exports_symbol(const std::string& symbol_name);

        inline const boost::filesystem::path& path() const {
            return _path;
        }

        void* symbol_address(const std::string& symbol_name);

    private:
        void get_library_path();

        void load_symbols(const char* path);

    private:
        bool _self_loaded = false;
        DLLib* _library = nullptr;
        symbol_address_map _symbols {};
        boost::filesystem::path _path {};
    };

    struct function_value_t {
        std::string name;
        ffi_types_t type;
        std::vector<function_value_t> fields {};
    };

    using function_value_list_t = std::vector<function_value_t>;

    struct function_signature_t {
        bool is_variadic() const {
            return calling_mode == ffi_calling_mode_t::c_ellipsis_varargs
                || calling_mode == ffi_calling_mode_t::c_ellipsis;
        }

        std::string symbol {};
        void* func_ptr = nullptr;
        function_value_t return_value {};
        shared_library_t* library = nullptr;
        function_value_list_t arguments {};
        ffi_calling_mode_t calling_mode = ffi_calling_mode_t::c_default;
        std::unordered_map<common::id_t, function_value_list_t> call_site_arguments {};
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class heap_vectors_t : uint8_t {
        top_of_stack = 0,
        bottom_of_stack,
        program_start,
        free_space_start,
        bss_start,
        bss_length,
    };

    class allocator {
    public:
        virtual ~allocator();

        virtual void reset() = 0;

        virtual void initialize(
            uint64_t address,
            uint64_t size) = 0;

        virtual uint64_t alloc(uint64_t size) = 0;

        virtual uint64_t size(uint64_t address) = 0;

        virtual uint64_t free(uint64_t address) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class assembly_symbol_type_t {
        assembler = 1,
        offset,
        module,
        label,
    };

    struct compiler_label_data_t {
        std::string label;
    };

    enum class compiler_module_data_type_t {
        reg,
        label,
        imm_f32,
        imm_f64,
        imm_integer,
    };

    struct compiler_module_data_t {
    public:
        explicit compiler_module_data_t(float value) : _data(value),
                                                       _type(compiler_module_data_type_t::imm_f32) {
        }

        explicit compiler_module_data_t(double value) : _data(value),
                                                       _type(compiler_module_data_type_t::imm_f64) {
        }

        explicit compiler_module_data_t(uint64_t value) : _data(value),
                                                          _type(compiler_module_data_type_t::imm_integer) {
        }

        explicit compiler_module_data_t(const std::string& value) : _data(value),
                                                                    _type(compiler_module_data_type_t::label) {
        }

        explicit compiler_module_data_t(const vm::register_t& value) : _data(value),
                                                                       _type(compiler_module_data_type_t::reg) {
        }

        template <typename T>
        T* data() {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        template <typename T>
        const T* data() const {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        compiler_module_data_type_t type() const {
            return _type;
        }

    private:
        boost::any _data;
        compiler_module_data_type_t _type;
    };

    struct compiler_local_data_t {
        int64_t offset = 0;
        vm::registers_t reg {};
    };

    struct assembly_symbol_result_t {
    public:
        bool is_set() const {
            return _is_set;
        }

        void data(const compiler_label_data_t& value);

        void data(const compiler_local_data_t& value);

        void data(const compiler_module_data_t& value);

        template <typename T>
        T* data() {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        template <typename T>
        const T* data() const {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

    private:
        boost::any _data;
        bool _is_set = false;
    };

    using assembly_symbol_resolver_t = std::function<bool (
        vm::assembly_symbol_type_t,
        void* data,
        const std::string&,
        vm::assembly_symbol_result_t&)>;

}