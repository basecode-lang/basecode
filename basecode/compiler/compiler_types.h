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
#include <string_view>
#include <vm/vm_types.h>
#include <unordered_map>
#include <unordered_set>
#include <common/rune.h>
#include <parser/parser.h>
#include <common/result.h>
#include <common/id_pool.h>
#include <common/source_file.h>
#include <boost/filesystem.hpp>
#include <common/term_stream_builder.h>
#include "elements/element_types.h"

namespace basecode::compiler {

    class session;
    class program;
    class element_map;
    class variable_map;
    class ast_evaluator;
    class scope_manager;
    class element_builder;
    class type_name_builder;
    class string_intern_map;
    class byte_code_emitter;
    class code_dom_formatter;

    using path_list_t = std::vector<boost::filesystem::path>;
    using source_file_stack_t = std::stack<common::source_file*>;
    using source_file_list_t = std::vector<common::source_file*>;
    using module_map_t = std::unordered_map<std::string, module*>;
    using parser_map_t = std::unordered_map<std::string, syntax::parser>;
    using ast_map_t = std::unordered_map<std::string, syntax::ast_node_t*>;
    using source_file_map_t = std::unordered_map<common::id_t, common::source_file>;
    using address_register_map_t = std::unordered_map<common::id_t, vm::register_t>;
    using source_file_path_map_t = std::unordered_map<std::string, common::source_file*>;

    ///////////////////////////////////////////////////////////////////////////

    field_map_t clone(
        compiler::session& session,
        compiler::block* new_scope,
        const field_map_t& fields);

    label_list_t clone(
        compiler::session& session,
        compiler::block* new_scope,
        const label_list_t& list);

    element_list_t clone(
        compiler::session& session,
        compiler::block* new_scope,
        const element_list_t& list);

    type_reference_list_t clone(
        compiler::session& session,
        compiler::block* new_scope,
        const type_reference_list_t& list);

    identifier_reference_list_t clone(
        compiler::session& session,
        compiler::block* new_scope,
        const identifier_reference_list_t& list);

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
        return vm::local_type_t::integer;
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
        boost::filesystem::path cfg_graph_file;
        session_definition_map_t definitions {};
        session_compile_callback compile_callback;
        common::term_stream_builder* term_builder = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class session_task_category_t : uint8_t {
        parser,
        general,
        compiler,
        assembler,
        evaluator,
        virtual_machine,
        byte_code_emitter,
        native_code_emitter,
    };

    static inline std::unordered_map<session_task_category_t, std::string_view> s_task_categories = {
        {session_task_category_t::parser,               "parser     "sv},
        {session_task_category_t::general,              "general    "sv},
        {session_task_category_t::compiler,             "compiler   "sv},
        {session_task_category_t::assembler,            "assembler  "sv},
        {session_task_category_t::evaluator,            "evaluator  "sv},
        {session_task_category_t::virtual_machine,      "interpreter"sv},
        {session_task_category_t::byte_code_emitter,    "byte code  "sv},
        {session_task_category_t::native_code_emitter,  "native code"sv},
    };

    static inline std::string_view session_task_category_to_name(session_task_category_t category) {
        auto it = s_task_categories.find(category);
        if (it == std::end(s_task_categories))
            return "general"sv;
        return it->second;
    }

    struct session_task_t;

    using session_task_list_t = std::vector<session_task_t>;
    using session_task_stack_t = std::stack<session_task_t*>;

    struct session_task_t {
        session_task_t(
                const std::string& name,
                session_task_category_t category) : name(name),
                                                    category(category) {
        }

        std::string name;
        session_task_list_t subtasks {};
        std::chrono::microseconds elapsed;
        session_task_category_t category {};
    };

    using session_task_callable_t = std::function<bool (session_task_t*)>;

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
        bool omit_rts = false;
        bool is_assign_target = false;
        infer_type_result_t type_result {};
        std::vector<temp_pool_entry_t*> temps {};
        vm::instruction_operand_list_t operands {};
        vm::op_sizes target_size = vm::op_sizes::none;
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
        std::string label_name() const;

        std::string path {};
        uint64_t from_end = 0;
        field_list_t fields {};
        uint64_t from_start = 0;
        compiler::identifier_reference* base_ref = nullptr;
    };

}