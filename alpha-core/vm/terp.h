// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
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
#include <filesystem>
#include <fmt/format.h>
#include <unordered_map>
#include <common/result.h>
#include <dyncall/dyncall.h>
#include <dynload/dynload.h>
#include <dyncall/dyncall_struct.h>
#include <dyncall/dyncall_signature.h>

namespace basecode::vm {

    enum i_registers_t : uint8_t {
        i0,
        i1,
        i2,
        i3,
        i4,
        i5,
        i6,
        i7,
        i8,
        i9,

        i10,
        i11,
        i12,
        i13,
        i14,
        i15,
        i16,
        i17,
        i18,
        i19,

        i20,
        i21,
        i22,
        i23,
        i24,
        i25,
        i26,
        i27,
        i28,
        i29,

        i30,
        i31,
        i32,
        i33,
        i34,
        i35,
        i36,
        i37,
        i38,
        i39,

        i40,
        i41,
        i42,
        i43,
        i44,
        i45,
        i46,
        i47,
        i48,
        i49,

        i50,
        i51,
        i52,
        i53,
        i54,
        i55,
        i56,
        i57,
        i58,
        i59,

        i60,
        i61,
        i62,
        i63,

        pc,
        sp,
        fr,
        sr
    };

    ///////////////////////////////////////////////////////////////////////////

    enum f_registers_t : uint8_t {
        f0,
        f1,
        f2,
        f3,
        f4,
        f5,
        f6,
        f7,
        f8,
        f9,

        f10,
        f11,
        f12,
        f13,
        f14,
        f15,
        f16,
        f17,
        f18,
        f19,

        f20,
        f21,
        f22,
        f23,
        f24,
        f25,
        f26,
        f27,
        f28,
        f29,

        f30,
        f31,
        f32,
        f33,
        f34,
        f35,
        f36,
        f37,
        f38,
        f39,

        f40,
        f41,
        f42,
        f43,
        f44,
        f45,
        f46,
        f47,
        f48,
        f49,

        f50,
        f51,
        f52,
        f53,
        f54,
        f55,
        f56,
        f57,
        f58,
        f59,

        f60,
        f61,
        f62,
        f63,
    };

    ///////////////////////////////////////////////////////////////////////////

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

    ///////////////////////////////////////////////////////////////////////////

    enum class op_codes : uint8_t {
        nop = 1,
        alloc,
        free,
        size,
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
        swap,
        trap,
        ffi,
        meta,
        exit
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class op_sizes : uint8_t {
        none,
        byte,
        word,
        dword,
        qword
    };

    ///////////////////////////////////////////////////////////////////////////

    struct operand_encoding_t {
        using flags_t = uint8_t;

        enum flags : uint8_t {
            none     = 0b00000000,
            constant = 0b00000000,
            reg      = 0b00000001,
            integer  = 0b00000010,
            negative = 0b00000100,
            prefix   = 0b00001000,
            postfix  = 0b00010000,
        };

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

        flags_t type = flags::reg | flags::integer;
        union {
            uint8_t r8;
            uint64_t u64;
            double d64;
        } value;
    };

    ///////////////////////////////////////////////////////////////////////////

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
                const std::filesystem::path& path);

        bool initialize(common::result& r);

        inline const std::filesystem::path& path() const {
            return _path;
        }

        inline const symbol_address_map& symbols() const {
            return _symbols;
        }

        bool exports_symbol(const std::string& symbol_name);

        void* symbol_address(const std::string& symbol_name);

    private:
        void get_library_path();

        void load_symbols(const char* path);

    private:
        DLLib* _library = nullptr;
        std::filesystem::path _path {};
        symbol_address_map _symbols {};
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

        static constexpr uint64_t mask_byte        = 0b0000000000000000000000000000000000000000000000000000000011111111;
        static constexpr uint64_t mask_byte_clear  = ~mask_byte;

        static constexpr uint64_t mask_word        = 0b0000000000000000000000000000000000000000000000001111111111111111;
        static constexpr uint64_t mask_word_clear  = ~mask_word;

        static constexpr uint64_t mask_dword       = 0b0000000000000000000000000000000011111111111111111111111111111111;
        static constexpr uint64_t mask_dword_clear = ~mask_dword;

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
            const std::filesystem::path& path);

        std::vector<uint64_t> jump_to_subroutine(
            common::result& r,
            uint64_t address);

        void swi(uint8_t index, uint64_t address);

        const register_file_t& register_file() const;

        void heap_free_space_begin(uint64_t address);

        uint64_t heap_vector(heap_vectors_t vector) const;

        void dump_heap(uint64_t offset, size_t size = 256);

        const meta_information_t& meta_information() const;

        std::string disassemble(const instruction_t& inst) const;

        void heap_vector(heap_vectors_t vector, uint64_t address);

        std::string disassemble(common::result& r, uint64_t address);

        void register_trap(uint8_t index, const trap_callable& callable);

        shared_library_t* shared_library(const std::filesystem::path& path);

    private:
        bool has_overflow(
            uint64_t lhs,
            uint64_t rhs,
            uint64_t result,
            op_sizes size);

        uint64_t set_zoned_value(
            uint64_t source,
            uint64_t value,
            op_sizes size);

        bool get_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t& value) const;

        bool get_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            double& value) const;

        void free_heap_block_list();

        bool set_target_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t value);

        bool set_target_operand_value(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            double value);

        void execute_trap(uint8_t index);

        bool get_constant_address_or_pc_with_offset(
            common::result& r,
            const instruction_t& inst,
            uint8_t operand_index,
            uint64_t inst_size,
            uint64_t& address);

        bool has_carry(uint64_t value, op_sizes size);

        bool is_negative(uint64_t value, op_sizes size);

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

        inline uint8_t op_size_in_bytes(op_sizes size) const {
            switch (size) {
                case op_sizes::byte:  return 1;
                case op_sizes::word:  return 2;
                case op_sizes::dword: return 4;
                case op_sizes::qword: return 8;
                default:              return 0;
            }
        }

    private:
        inline static std::map<op_codes, std::string> s_op_code_names = {
            {op_codes::nop,    "NOP"},
            {op_codes::alloc,  "ALLOC"},
            {op_codes::free,   "FREE"},
            {op_codes::size,   "SIZE"},
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
            {op_codes::ffi,    "FFI"},
            {op_codes::meta,   "META"},
            {op_codes::exit,   "EXIT"},
        };

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