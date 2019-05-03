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

    using namespace std::literals;

    enum class variable_type_t : uint8_t {
        temporary,
        parameter,
        module,
        local,
        return_parameter
    };

    static inline std::string_view variable_type_name(variable_type_t type) {
        switch (type) {
            case variable_type_t::local:            return "local"sv;
            case variable_type_t::module:           return "module"sv;
            case variable_type_t::return_parameter: return "return"sv;
            case variable_type_t::temporary:        return "temporary"sv;
            case variable_type_t::parameter:        return "parameter"sv;
        }
        return ""sv;
    }

    static inline std::string_view variable_type_to_group(variable_type_t type) {
        switch (type) {
            case variable_type_t::module:
            case variable_type_t::temporary:        return ""sv;
            case variable_type_t::local:            return "local"sv;
            case variable_type_t::parameter:        return "parameter"sv;
            case variable_type_t::return_parameter: return "return"sv;
        }
        return ""sv;
    }

    struct variable_t {
        enum flags_t : uint8_t {
            none        = 0b00000000,
            initialized = 0b00000001,
            filled      = 0b00000010,
            spilled     = 0b00000100,
            must_init   = 0b00001000,
            used        = 0b00010000,
            in_block    = 0b00100000,
            pointer     = 0b01000000,
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
    using variable_stack_t = std::stack<variable_t*>;
    using variable_set_t = std::unordered_set<variable_t*>;
    using const_variable_list_t = std::vector<const variable_t*>;
    using variable_map_t = std::map<std::string, variable_t>;
    using temp_pool_entry_list_t = std::vector<temp_pool_entry_t*>;
    using grouped_variable_list_t = std::vector<const_variable_list_t>;
    using temp_pool_map_t = std::unordered_map<common::id_t, temp_pool_entry_t>;

    struct group_variable_result_t {
        grouped_variable_list_t ints {};
        grouped_variable_list_t floats {};
    };

    class variable_map {
    public:
        explicit variable_map(compiler::session& session);

        bool use(
            vm::basic_block* basic_block,
            vm::assembler_named_ref_t* named_ref,
            bool is_assign_target = false);

        bool build(
            compiler::block* block,
            compiler::procedure_type* proc_type = nullptr);

        bool deref(
            vm::basic_block* basic_block,
            emit_result_t& arg_result,
            emit_result_t& result);

        bool assign(
            vm::basic_block* basic_block,
            emit_result_t& lhs,
            emit_result_t& rhs,
            bool requires_copy = false,
            bool array_subscript = false);

        void reset();

        bool address_of(
            vm::basic_block* basic_block,
            emit_result_t& arg_result,
            vm::instruction_operand_t& temp_operand);

        bool initialize();

        variable_list_t temps();

        void save_locals_to_stack(
            vm::basic_block* basic_block,
            const group_variable_result_t& groups);

        variable_list_t variables();

        void restore_locals_from_stack(
            vm::basic_block* basic_block,
            const group_variable_result_t& groups);

        variable_t* find(const std::string& name);

        void release_temp(temp_pool_entry_t* entry);

        identifier_by_section_t& module_variables();

        group_variable_result_t group_variables(const variable_set_t& excluded);

        temp_pool_entry_t* retain_temp(number_class_t number_class = number_class_t::integer);

    private:
        void create_sections();

        void apply_variable_range(
            const const_variable_list_t& list,
            vm::instruction_operand_list_t& operands,
            bool reverse);

        void clear_filled(const variable_t* var);

        bool group_module_variables_into_sections();

        bool find_local_variables(compiler::block* block);

        bool find_referenced_module_variables(compiler::block* block);

        bool find_return_variables(compiler::procedure_type* proc_type);

        bool find_parameter_variables(compiler::procedure_type* proc_type);

        temp_pool_entry_t* find_available_temp(number_class_t number_class);

        bool is_related_to_type(const variable_t* var, variable_type_t type);

    private:
        temp_pool_map_t _temps {};
        compiler::session& _session;
        variable_map_t _variables {};
        identifier_by_section_t _module_variables {};
    };

}
