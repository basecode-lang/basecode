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

    static inline vm::assembler_named_ref_type_t variable_type_to_named_ref_type(variable_type_t type) {
        switch (type) {
            case variable_type_t::module:           return vm::assembler_named_ref_type_t::label;
            case variable_type_t::local:
            case variable_type_t::temporary:
            case variable_type_t::parameter:
            case variable_type_t::return_parameter: return vm::assembler_named_ref_type_t::local;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    struct temp_pool_entry_t;

    struct variable_t {
        using flags_value_t = uint8_t;

        enum flags_t : uint8_t {
            none        = 0b00000000,
            live        = 0b00000001,
            filled      = 0b00000010,
            spilled     = 0b00000100,
            excluded    = 0b00001000,
            modified    = 0b00010000,
            initialize  = 0b00100000,
        };

        bool is_live() const;

        void mark_modified();

        bool is_filled() const;

        bool is_spilled() const;

        bool is_excluded() const;

        bool is_modified() const;

        void transition_to_live();

        void transition_to_killed();

        void transition_to_filled();

        size_t size_in_bytes() const;

        void transition_to_spilled();

        void transition_to_excluded();

        std::string label {};
        flags_value_t state {};
        variable_type_t type {};
        int64_t frame_offset = 0;
        number_class_t number_class {};
        offset_result_t field_offset {};
        temp_pool_entry_t* temp = nullptr;
        vm::op_sizes op_size = vm::op_sizes::qword;
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

    struct variable_map_config_t {
        uint16_t max_register_count[2] = {32, 32};
    };

    struct fill_target_pair_t {
        vm::assembler_named_ref_t* dest = nullptr;
        vm::assembler_named_ref_t* offset = nullptr;
    };

    struct spill_target_pair_t {
        vm::assembler_named_ref_t* src = nullptr;
        vm::assembler_named_ref_t* offset = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    class variable_context {
    public:
        variable_context(
            compiler::session& session,
            common::id_t id,
            vm::basic_block* locals_block);

        common::id_t id() const;

        variable_list_t temps();

        bool read(
            vm::basic_block** basic_block,
            compiler::identifier_reference* ref,
            vm::instruction_operand_t& result);

        bool read(
            vm::basic_block** basic_block,
            compiler::identifier* var,
            vm::instruction_operand_t& result,
            const compiler::offset_result_t& offset_result);

        bool deref(
            vm::basic_block** basic_block,
            emit_result_t& expr,
            vm::instruction_operand_t& result);

        bool assign(
            vm::basic_block** basic_block,
            compiler::element* element,
            emit_result_t& lhs,
            emit_result_t& rhs);

        bool activate(
            vm::basic_block** basic_block,
            compiler::identifier* var,
            const offset_result_t& offset_result);

        bool activate(
            vm::basic_block** basic_block,
            compiler::identifier_reference* ref);

        bool deactivate(
            vm::basic_block** basic_block,
            compiler::identifier* var,
            const offset_result_t& offset_result);

        bool address_of(
            vm::basic_block** basic_block,
            emit_result_t& expr,
            vm::instruction_operand_t& result);

        bool deactivate_scope(
            vm::basic_block** basic_block,
            compiler::block* scope);

        size_t locals_size() const;

        void save_locals_to_stack(
            vm::basic_block* basic_block,
            const group_variable_result_t& groups);

        variable_list_t variables();

        void restore_locals_from_stack(
            vm::basic_block* basic_block,
            const group_variable_result_t& groups);

        void add_scope(compiler::block* scope);

        void release_temp(temp_pool_entry_t* entry);

        variable_t* find_variable(const std::string& name);

        void release_temps(const temp_pool_entry_list_t& temps);

        temp_pool_entry_t* find_temp(number_class_t number_class);

        group_variable_result_t group_variables(const variable_set_t& excluded);

        temp_pool_entry_t* retain_temp(number_class_t number_class = number_class_t::integer);

    private:
        static std::string get_variable_name(
            compiler::identifier* var,
            const offset_result_t& offset_result);

    private:
        bool fill(
            vm::basic_block** basic_block,
            variable_t* var_info,
            vm::instruction_operand_t& result);

        bool spill(
            vm::basic_block** basic_block,
            variable_t* var_info);

        void apply_variable_range(
            const const_variable_list_t& list,
            vm::instruction_operand_list_t& operands,
            bool reverse);

        bool prepare_fill_target_pair(
            variable_t* var_info,
            fill_target_pair_t& target_pair);

        bool prepare_spill_target_pair(
            variable_t* var_info,
            spill_target_pair_t& target_pair);

        bool use_frame_pointer(const variable_t* var_info) const;

        bool should_variable_fill(const variable_t* var_info) const;

        bool should_variable_spill(const variable_t* var_info) const;

        bool is_related_to_type(const variable_t* var, variable_type_t type);

    private:
        common::id_t _id{};
        size_t _local_offset = 0;
        temp_pool_map_t _temps{};
        size_t _return_offset = 0;
        variable_map_t _variables{};
        compiler::session& _session;
        size_t _parameter_offset = 0;
        variable_map_config_t _config{};
        compiler::block_list_t _scope_blocks{};
        vm::basic_block* _locals_block = nullptr;
    };

    using variable_context_stack_t = std::stack<variable_context*>;
    using variable_context_map_t = std::unordered_map<common::id_t, variable_context>;

    ///////////////////////////////////////////////////////////////////////////

    class variable_map {
    public:
        explicit variable_map(compiler::session& session);

    public:
        void reset();

        bool initialize();

        identifier_by_section_t& module_variables();

        variable_context* find_context(common::id_t id);

        variable_context* make_context(vm::basic_block* locals_block);

    private:
        void create_sections();

        bool group_module_variables_into_sections();

    private:
        compiler::session& _session;
        variable_context_map_t _contexts{};
        identifier_by_section_t _module_variables {};
    };

}
