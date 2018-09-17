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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#include <map>
#include <set>
#include <string>
#include <memory>
#include <cstdint>
#include <functional>
#include <fmt/format.h>
#include <unordered_map>
#include <common/result.h>
#include <dyncall/dyncall.h>
#include <dynload/dynload.h>
#include <boost/filesystem.hpp>
#include <dyncall/dyncall_struct.h>
#include <dyncall/dyncall_signature.h>

namespace basecode::vm {

    enum class op_sizes : uint8_t {
        none,
        byte,
        word,
        dword,
        qword
    };

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
                .number = registers_t::pc,
                .type = register_type_t::pc,
            };
        }

        static register_t sp() {
            return register_t {
                .number = registers_t::sp,
                .type = register_type_t::sp,
            };
        }

        static register_t fp() {
            return register_t {
                .number = registers_t::fp,
                .type = register_type_t::fp,
            };
        }

        static register_t empty() {
            return register_t {
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
        return 0;
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

        bool flags(flags_t f) const {
            return (r[register_fr].qw & f) != 0;
        }

        void flags(flags_t f, bool value) {
            if (value)
                r[register_fr].qw |= f;
            else
                r[register_fr].qw &= ~f;
        }

        register_value_alias_t r[number_total_registers];
    };

    ///////////////////////////////////////////////////////////////////////////

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

    union operand_value_alias_t {
        uint8_t  r;
        uint64_t u;
        int64_t  s;
        float    f;
        double   d;
    };

    struct operand_value_t {
        register_type_t type;
        operand_value_alias_t alias {
            .u = 0
        };
    };

    struct operand_encoding_t {
        using flags_t = uint8_t;

        enum flags : uint8_t {
            none        = 0b00000000,
            constant    = 0b00000000,
            reg         = 0b00000001,
            integer     = 0b00000010,
            negative    = 0b00000100,
            prefix      = 0b00001000,
            postfix     = 0b00010000,
            unresolved  = 0b00100000,
        };

        void clear_unresolved() {
            type &= ~flags::unresolved;
        }

        inline bool is_reg() const {
            return (type & flags::reg) != 0;
        }

        inline bool is_prefix() const {
            return (type & flags::prefix) != 0;
        }

        inline bool is_postfix() const {
            return (type & flags::postfix) != 0;
        }

        inline bool is_integer() const {
            return (type & flags::integer) != 0;
        }

        inline bool is_negative() const {
            return (type & flags::negative) != 0;
        }

        inline bool is_unresolved() const {
            return (type & flags::unresolved) != 0;
        }

        flags_t type = flags::reg | flags::integer;
        operand_value_alias_t value;
    };

    ///////////////////////////////////////////////////////////////////////////

    using id_resolve_callable = std::function<std::string (uint64_t)>;

    struct instruction_t {
        static constexpr size_t base_size = 3;
        static constexpr size_t alignment = 4;

        size_t decode(
            common::result& r,
            uint8_t* heap,
            uint64_t address);

        size_t encode(
            common::result& r,
            uint8_t* heap,
            uint64_t address);

        size_t encoding_size() const;

        size_t align(uint64_t value, size_t size) const;

        void patch_branch_address(uint64_t address, uint8_t index = 0);

        std::string disassemble(const id_resolve_callable& id_resolver = nullptr) const;

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
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class heap_vectors_t : uint8_t {
        top_of_stack = 0,
        bottom_of_stack,
        program_start,
        free_space_start,
    };

    struct heap_block_t {
        enum flags_t : uint8_t {
            none       = 0b00000000,
            allocated  = 0b00000001,
        };

        inline void mark_allocated() {
            flags |= flags_t::allocated;
        }

        inline void clear_allocated() {
            flags &= ~flags_t::allocated;
        }

        inline bool is_free() const {
            return (flags & flags_t::allocated) == 0;
        }

        inline bool is_allocated() const {
            return (flags & flags_t::allocated) != 0;
        }

        uint64_t size = 0;
        uint64_t address = 0;
        heap_block_t* prev = nullptr;
        heap_block_t* next = nullptr;
        uint8_t flags = flags_t::none;
    };

    ///////////////////////////////////////////////////////////////////////////

    using symbol_address_map = std::unordered_map<std::string, void*>;

    struct shared_library_t {
        shared_library_t() {
        }

        ~shared_library_t() {
            if (_library != nullptr)
                dlFreeLibrary(_library);
        }

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
        ~function_value_t();

        DCstruct* struct_meta_info();

        void push(DCCallVM* vm, uint64_t value);

        std::string name;
        ffi_types_t type;
        std::vector<function_value_t> fields {};

    private:
        void add_struct_fields(DCstruct* s);

    private:
        DCstruct* _struct_meta_data = nullptr;
    };

    struct function_signature_t {
        void apply_calling_convention(DCCallVM* vm);

        uint64_t call(DCCallVM* vm, uint64_t address);

        std::string symbol {};
        void* func_ptr = nullptr;
        function_value_t return_value {};
        shared_library_t* library = nullptr;
        std::vector<function_value_t> arguments {};
        ffi_calling_mode_t calling_mode = ffi_calling_mode_t::c_default;
    };

    ///////////////////////////////////////////////////////////////////////////

    class terp;

    struct icache_entry_t {
        size_t size;
        instruction_t inst;
    };

    class instruction_cache {
    public:
        explicit instruction_cache(terp* terp);

        void reset();

        size_t fetch_at(
            common::result& r,
            uint64_t address,
            instruction_t& inst);

        size_t fetch(common::result& r, instruction_t& inst);

    private:
        terp* _terp = nullptr;
        std::unordered_map<uint64_t, icache_entry_t> _cache {};
    };

    ///////////////////////////////////////////////////////////////////////////

    class terp {
    public:
        using trap_callable = std::function<void (terp*)>;

        static constexpr uint64_t mask_byte_negative  = 0b0000000000000000000000000000000000000000000000000000000010000000;
        static constexpr uint64_t mask_word_negative  = 0b0000000000000000000000000000000000000000000000001000000000000000;
        static constexpr uint64_t mask_dword_negative = 0b0000000000000000000000000000000010000000000000000000000000000000;
        static constexpr uint64_t mask_qword_negative = 0b1000000000000000000000000000000000000000000000000000000000000000;

        static constexpr size_t interrupt_vector_table_start = 0;
        static constexpr size_t interrupt_vector_table_size = 16;
        static constexpr size_t interrupt_vector_table_end = interrupt_vector_table_start +
            (sizeof(uint64_t) * interrupt_vector_table_size);

        static constexpr size_t heap_vector_table_start = interrupt_vector_table_end;
        static constexpr size_t heap_vector_table_size = 16;
        static constexpr size_t heap_vector_table_end = heap_vector_table_start
            + (sizeof(uint16_t) * heap_vector_table_size);

        static constexpr size_t program_start = heap_vector_table_end;

        static constexpr uint8_t trap_out_of_memory = 0xff;
        static constexpr uint8_t trap_invalid_ffi_call = 0xfe;

        terp(
            size_t heap_size,
            size_t stack_size);

        virtual ~terp();

        void reset();

        uint64_t pop();

        uint64_t peek() const;

        bool has_exited() const;

        inline uint8_t* heap() {
            return _heap;
        }

        size_t heap_size() const;

        void push(uint64_t value);

        size_t stack_size() const;

        bool run(common::result& r);

        bool step(common::result& r);

        void dump_shared_libraries();

        uint64_t alloc(uint64_t size);

        bool register_foreign_function(
            common::result& r,
            function_signature_t& signature);

        uint64_t free(uint64_t address);

        uint64_t size(uint64_t address);

        void remove_trap(uint8_t index);

        bool initialize(common::result& r);

        void dump_state(uint8_t count = 16);

        shared_library_t* load_shared_library(
            common::result& r,
            const boost::filesystem::path& path);

        std::vector<uint64_t> jump_to_subroutine(
            common::result& r,
            uint64_t address);

        void swi(uint8_t index, uint64_t address);

        const register_file_t& register_file() const;

        void heap_free_space_begin(uint64_t address);

        uint64_t heap_vector(heap_vectors_t vector) const;

        void dump_heap(uint64_t offset, size_t size = 256);

        const meta_information_t& meta_information() const;

        uint64_t read(op_sizes size, uint64_t address) const;

        void heap_vector(heap_vectors_t vector, uint64_t address);

        void write(op_sizes size, uint64_t address, uint64_t value);

        std::string disassemble(common::result& r, uint64_t address);

        void register_trap(uint8_t index, const trap_callable& callable);

        shared_library_t* shared_library(const boost::filesystem::path& path);

    private:
        bool has_overflow(
            uint64_t lhs,
            uint64_t rhs,
            uint64_t result,
            op_sizes size);

        void set_zoned_value(
            register_type_t type,
            register_value_alias_t& reg,
            uint64_t value,
            op_sizes size);

        bool get_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            operand_value_t& value) const;

        void free_heap_block_list();

        bool set_target_operand_value(
            common::result& r,
            const operand_encoding_t& operand,
            op_sizes size,
            const operand_value_t& value);

        void execute_trap(uint8_t index);

        bool get_constant_address_or_pc_with_offset(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t inst_size,
            operand_value_t& address);

        bool has_carry(const operand_value_t& value, op_sizes size);

        bool is_negative(const operand_value_t& value, op_sizes size);

    private:
        bool _exited = false;
        size_t _heap_size = 0;
        size_t _stack_size = 0;
        uint8_t* _heap = nullptr;
        instruction_cache _icache;
        DCCallVM* _call_vm = nullptr;
        register_file_t _registers {};
        meta_information_t _meta_information {};
        heap_block_t* _head_heap_block = nullptr;
        std::unordered_map<uint8_t, trap_callable> _traps {};
        std::unordered_map<uint64_t, heap_block_t*> _address_blocks {};
        std::unordered_map<void*, function_signature_t> _foreign_functions {};
        std::unordered_map<std::string, shared_library_t> _shared_libraries {};
    };

};

#pragma clang diagnostic pop