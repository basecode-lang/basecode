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

#include <chrono>
#include <cstdint>
#include <functional>
#include <parser/ast.h>
#include <vm/vm_types.h>
#include <unordered_map>
#include <common/rune.h>
#include <common/result.h>
#include <common/id_pool.h>
#include <common/source_file.h>
#include <boost/filesystem.hpp>
#include "elements/element_types.h"

namespace basecode::compiler {

    class session;
    class program;
    class element_map;
    class variable_map;
    class ast_evaluator;
    class scope_manager;
    class element_builder;
    class string_intern_map;
    class byte_code_emitter;
    class code_dom_formatter;

    using path_list_t = std::vector<boost::filesystem::path>;
    using source_file_stack_t = std::stack<common::source_file*>;
    using source_file_list_t = std::vector<common::source_file*>;
    using module_map_t = std::unordered_map<std::string, module*>;
    using ast_map_t = std::unordered_map<std::string, syntax::ast_node_t*>;
    using source_file_map_t = std::unordered_map<common::id_t, common::source_file>;
    using address_register_map_t = std::unordered_map<common::id_t, vm::register_t>;
    using source_file_path_map_t = std::unordered_map<std::string, common::source_file*>;

    ///////////////////////////////////////////////////////////////////////////

    enum class number_class_t {
        none,
        integer,
        floating_point,
    };

    static inline vm::local_type_t number_class_to_local_type(number_class_t type) {
        switch (type) {
            case number_class_t::none:
            case number_class_t::integer: {
                return vm::local_type_t::integer;
            }
            case number_class_t::floating_point: {
                return vm::local_type_t::floating_point;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    enum class session_compile_phase_t : uint8_t {
        start,
        success,
        failed
    };

    enum class session_module_type_t : uint8_t {
        program,
        module
    };

    using session_compile_callback = std::function<void (
        session_compile_phase_t,
        session_module_type_t,
        const boost::filesystem::path&)>;

    using session_meta_options_t = std::vector<std::string>;
    using session_module_paths_t = std::vector<boost::filesystem::path>;
    using session_definition_map_t = std::unordered_map<std::string, std::string>;

    struct session_options_t {
        bool verbose = false;
        size_t heap_size = 0;
        bool debugger = false;
        size_t stack_size = 0;
        size_t ffi_heap_size = 4096;
        bool output_ast_graphs = false;
        vm::allocator* allocator = nullptr;
        boost::filesystem::path compiler_path;
        session_meta_options_t meta_options {};
        session_module_paths_t module_paths {};
        boost::filesystem::path dom_graph_file;
        session_definition_map_t definitions {};
        session_compile_callback compile_callback;
    };

    using session_task_callable_t = std::function<bool ()>;

    struct session_task_t {
        std::string name;
        bool include_in_total;
        std::chrono::microseconds elapsed;
    };

    using session_task_list_t = std::vector<session_task_t>;

    ///////////////////////////////////////////////////////////////////////////

    struct identifier_by_section_t {
        element_list_t* variable_section(vm::section_t value) {
            auto it = sections.find(value);
            if (it == sections.end()) {
                return nullptr;
            }
            return &it->second;
        }

        std::unordered_set<common::id_t> identifiers {};
        std::map<vm::section_t, element_list_t> sections {};
    };

    enum class identifier_usage_t : uint8_t {
        heap = 1,
        stack
    };

    struct temp_pool_entry_t;

    struct emit_result_t {
        bool is_assign_target = false;
        infer_type_result_t type_result {};
        std::vector<temp_pool_entry_t*> temps {};
        vm::instruction_operand_list_t operands {};
    };

    using flow_control_value_map_t = std::unordered_map<uint16_t, boost::any>;

    struct flow_control_t {
        bool fallthrough = false;
        flow_control_value_map_t values {};
        vm::basic_block* predecessor = nullptr;
        vm::assembler_named_ref_t* exit_label = nullptr;
        vm::assembler_named_ref_t* continue_label = nullptr;
    };

    using flow_control_stack_t = std::stack<flow_control_t>;

    struct offset_result_t {
        std::string path {};
        std::string base_label {};
        compiler::field* field = nullptr;
        compiler::identifier_reference* base_ref = nullptr;
    };
}