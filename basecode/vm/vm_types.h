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

namespace basecode::vm {

    class terp;
    class label;
    class symbol;
    class segment;
    class assembler;
    class stack_frame;
    class assembly_listing;
    class instruction_block;
    class register_allocator;

    using symbol_list_t = std::vector<symbol*>;

    ///////////////////////////////////////////////////////////////////////////

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

    using segment_list_t = std::vector<segment*>;

    enum class segment_type_t {
        code,
        data,
        stack,
        constant,
    };

    static inline std::unordered_map<segment_type_t, std::string> s_segment_type_names = {
        {segment_type_t::code,     "code"},
        {segment_type_t::data,     "data"},
        {segment_type_t::stack,    "stack"},
        {segment_type_t::constant, "constant"}
    };

    static inline std::string segment_type_name(segment_type_t type) {
        auto it = s_segment_type_names.find(type);
        if (it == s_segment_type_names.end())
            return "unknown";
        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////

    enum class stack_frame_entry_type_t : uint8_t {
        local = 1,
        parameter,
        return_slot
    };

    inline static std::string stack_frame_entry_type_name(stack_frame_entry_type_t type) {
        switch (type) {
            case stack_frame_entry_type_t::local:
                return "local";
            case stack_frame_entry_type_t::parameter:
                return "parameter";
            case stack_frame_entry_type_t::return_slot:
                return "return_slot";
        }
        return "unknown";
    }

    struct stack_frame_entry_t {
        int32_t offset;
        std::string name;
        stack_frame_entry_type_t type;
    };

    ///////////////////////////////////////////////////////////////////////////

    struct label_ref_t {
        common::id_t id;
        std::string name;
        label* resolved = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class instruction_block_type_t {
        basic,
        procedure
    };

    enum class section_t : uint8_t {
        unknown,
        bss = 1,
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
            default:
                return "unknown";
        }
        return "unknown";
    }

    inline static section_t section_type(const std::string& name) {
        if (name == "bss")      return section_t::bss;
        if (name == "ro_data")  return section_t::ro_data;
        if (name == "data")     return section_t::data;
        if (name == "text")     return section_t::text;
        return section_t::unknown;
    }

    enum data_definition_type_t : uint8_t {
        none,
        initialized,
        uninitialized
    };

    struct align_t {
        uint8_t size = 0;
    };

    using data_value_variant_t = boost::variant<uint64_t, label_ref_t*>;
    struct data_definition_t {
        op_sizes size;
        data_definition_type_t type = data_definition_type_t::none;
        std::vector<data_value_variant_t> values {};
    };

    struct comment_t {
        uint8_t indent {};
        std::string value {};
    };

    struct label_t {
        vm::label* instance = nullptr;
    };

    enum class block_entry_type_t : uint8_t {
        section = 1,
        comment,
        label,
        blank_line,
        align,
        instruction,
        data_definition,
    };

    struct instruction_t;

    struct block_entry_t {
        block_entry_t();

        block_entry_t(const label_t& label);

        block_entry_t(const align_t& align);

        block_entry_t(const comment_t& comment);

        block_entry_t(const section_t& section);

        block_entry_t(const block_entry_t& other);

        block_entry_t(const data_definition_t& data);

        block_entry_t(const instruction_t& instruction);

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

    struct listing_source_line_t {
        uint64_t address = 0;
        std::string source {};
    };

    struct listing_source_file_t {
        void add_source_line(
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

};