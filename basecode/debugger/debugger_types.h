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
#include <functional>
#include <configure.h>
#include <unordered_map>
#include <boost/any.hpp>
#include <vm/vm_types.h>
#include <common/result.h>
#include <common/id_pool.h>
#include <compiler/compiler_types.h>

#define CTRL(c) ((c) & 0x1f)

namespace basecode::debugger {

    class window;
    class environment;
    class stack_window;
    class output_window;
    class memory_window;
    class footer_window;
    class header_window;
    class errors_window;
    class command_window;
    class assembly_window;
    class registers_window;

    ///////////////////////////////////////////////////////////////////////////

    enum class breakpoint_type_t : uint8_t {
        simple,
        flag_set,
        flag_clear,
        register_equals
    };

    struct breakpoint_t {
        bool enabled = false;
        uint64_t address = 0;
        breakpoint_type_t type = breakpoint_type_t::simple;
    };

    using breakpoint_map_t = std::unordered_map<uint64_t, breakpoint_t>;

    ///////////////////////////////////////////////////////////////////////////

    enum class debugger_state_t : uint8_t {
        stopped,
        running,
        single_step,
        break_s,
        errored,
        ended,
        command_entry,
        command_execute
    };

    enum class registers_display_mode_t : uint8_t {
        integers,
        floats
    };

    ///////////////////////////////////////////////////////////////////////////

    enum command_parameter_type_t : uint8_t {
        none        = 0b00000000,
        command     = 0b00000001,
        number      = 0b00000010,
        symbol      = 0b00000100,
        string_t    = 0b00001000,
        register_t  = 0b00010000,
    };

    enum class command_type_t : uint8_t {
        unknown,
        help,
        find,
        goto_line,
        show_memory,
        read_memory,
        write_memory
    };

    struct command_parameter_prototype_t {
        bool required;
        std::string name;
        uint8_t types = command_parameter_type_t::none;
    };

    struct command_prototype_t {
        enum size_flags_t : uint8_t {
            none  = 0b00000000,
            byte  = 0b00000001,
            word  = 0b00000010,
            dword = 0b00000100,
            qword = 0b00001000
        };

        size_flags_t suffix_to_size(const std::string& suffix);

        command_type_t type;
        uint8_t sizes = size_flags_t::none;
        std::map<std::string, command_parameter_prototype_t> params {};
    };

    struct command_data_t {
        bool parse(common::result& r);

        std::string name;
        vm::op_sizes size = vm::op_sizes::none;
        command_type_t type = command_type_t::unknown;
    };

    struct number_data_t {
        syntax::conversion_result_t parse(double& value) const;

        syntax::conversion_result_t parse(uint64_t& value) const;

        uint8_t radix;
        std::string input;
        bool is_float = false;
    };

    struct symbol_data_t {
        std::string input;
    };

    struct string_data_t {
        std::string input;
    };

    struct register_data_t {
        bool parse(common::result& r);

        std::string input;
        vm::op_sizes size;
        int64_t offset = 0;
        vm::registers_t value;
        vm::register_type_t type;
    };

    struct command_parameter_t {
    public:
        command_parameter_t();

        explicit command_parameter_t(const symbol_data_t& value);

        explicit command_parameter_t(const string_data_t& value);

        explicit command_parameter_t(const number_data_t& value);

        explicit command_parameter_t(const register_data_t& value);

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

        command_parameter_type_t type() const;

    private:
        boost::any _data;
        command_parameter_type_t _type = command_parameter_type_t::none;
    };

    using command_parameter_map_t = std::map<std::string, command_parameter_t>;

    struct command_t {
        command_data_t command {};
        command_parameter_map_t params {};
    };

    inline static std::unordered_map<std::string, command_prototype_t> s_commands = {
        {
            "help",
            {
                command_type_t::help,
                command_prototype_t::size_flags_t::none,
                {
                    {
                        "command_name",
                        {
                            false,
                            "command_name",
                            command_parameter_type_t::symbol
                        }
                    }
                }
            }
        },
        {
            ":",
            {
                command_type_t::goto_line,
                command_prototype_t::size_flags_t::none,
                {
                    {
                        "line_number",
                        {
                            true,
                            "line_number",
                            command_parameter_type_t::number
                        }
                    }
                }
            }
        },
        {
            "/",
            {
                command_type_t::find,
                command_prototype_t::size_flags_t::none,
                {
                    {
                        "needle",
                        {
                            true,
                            "needle",
                            command_parameter_type_t::symbol |
                                command_parameter_type_t::string_t
                        }
                    }
                }
            }
        },
        {
            "m",
            {
                command_type_t::show_memory,
                command_prototype_t::size_flags_t::none,
                {
                    {
                        "address",
                        {
                            true,
                            "address",
                            command_parameter_type_t::number |
                                command_parameter_type_t::register_t |
                                command_parameter_type_t::symbol
                        }
                    }
                }
            }
        },
        {
            "w",
            {
                command_type_t::write_memory,
                command_prototype_t::size_flags_t::byte |
                    command_prototype_t::size_flags_t::word |
                    command_prototype_t::size_flags_t::dword |
                    command_prototype_t::size_flags_t::qword,
                {
                    {
                        "address",
                        {
                            true,
                            "address",
                            command_parameter_type_t::number |
                                command_parameter_type_t::register_t |
                                command_parameter_type_t::symbol
                        }
                    },
                    {
                        "value",
                        {
                            true,
                            "value",
                            command_parameter_type_t::number |
                                command_parameter_type_t::register_t |
                                command_parameter_type_t::symbol
                        }
                    }
                }
            }
        },
    };

};