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

#include "compiler_types.h"

namespace basecode::compiler {

    enum class variable_type_t : uint8_t {
        temporary,
        parameter,
        module,
        local,
        return_value
    };

    static inline std::string variable_type_to_group(variable_type_t type) {
        switch (type) {
            case variable_type_t::module:
            case variable_type_t::temporary:    return "";
            case variable_type_t::local:        return "local";
            case variable_type_t::parameter:    return "parameter";
            case variable_type_t::return_value: return "return";
        }
    }

    struct variable_t {
        enum flags_t : uint8_t {
            none        = 0b00000000,
            initialized = 0b00000001,
            filled      = 0b00000010,
            spilled     = 0b00000100,
            must_init   = 0b00001000,
            used        = 0b00010000,
        };

        using flags_value_t = uint8_t;

        bool flag(flags_t f) const;

        size_t size_in_bytes() const;

        void flag(flags_t f, bool value);

        std::string label {};
        flags_value_t state {};
        variable_type_t type {};
        int64_t frame_offset = 0;
        number_class_t number_class {};
        offset_result_t field_offset {};
        compiler::identifier* identifier = nullptr;
    };

    struct temp_pool_entry_t {
        std::string name() const {
            return variable->label;
        }

        common::id_t id;
        bool available = false;
        variable_t* variable = nullptr;
    };

    using variable_list_t = std::vector<variable_t*>;
    using variable_map_t = std::map<std::string, variable_t>;
    using temp_pool_entry_list_t = std::vector<temp_pool_entry_t*>;
    using temp_pool_map_t = std::unordered_map<common::id_t, temp_pool_entry_t>;

    class variable_map {
    public:
        explicit variable_map(compiler::session& session);

        bool use(
            vm::basic_block* basic_block,
            vm::assembler_named_ref_t* named_ref,
            bool load_on_use = false);

        bool build(
            compiler::block* block,
            compiler::procedure_type* proc_type = nullptr);

        bool assign(
            vm::basic_block* basic_block,
            emit_result_t& lhs,
            emit_result_t& rhs,
            bool requires_copy = false);

        void reset();

        bool address_of(
            vm::basic_block* basic_block,
            emit_result_t& arg_result,
            vm::instruction_operand_t& temp_operand);

        bool initialize();

        variable_list_t temps();

        variable_list_t variables();

        variable_t* find(const std::string& name);

        void release_temp(temp_pool_entry_t* entry);

        identifier_by_section_t& module_variables();

        temp_pool_entry_t* retain_temp(number_class_t number_class = number_class_t::integer);

    private:
        void create_sections();

        bool group_module_variables_into_sections();

        bool find_local_variables(compiler::block* block);

        bool find_referenced_module_variables(compiler::block* block);

        bool find_return_variables(compiler::procedure_type* proc_type);

        bool find_parameter_variables(compiler::procedure_type* proc_type);

        temp_pool_entry_t* find_available_temp(number_class_t number_class);

    private:
        temp_pool_map_t _temps {};
        compiler::session& _session;
        variable_map_t _variables {};
        identifier_by_section_t _module_variables {};
    };

}

