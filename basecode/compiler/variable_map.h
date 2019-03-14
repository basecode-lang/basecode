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

    enum class variable_state_t : uint8_t {
        unknown,
        filled,
        spilled
    };

    struct variable_t {
        std::string label {};
        variable_type_t type {};
        int64_t frame_offset = 0;
        variable_state_t state {};
        number_class_t number_class {};
        offset_result_t field_offset {};
        compiler::identifier* identifier = nullptr;
    };

    struct temp_pool_entry_t {
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

        bool build(
            compiler::block* block,
            compiler::procedure_type* proc_type = nullptr);

        void reset();

        bool initialize();

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

