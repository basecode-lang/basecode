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

#include <cstdint>
#include <functional>
#include <vm/vm_types.h>
#include <unordered_map>
#include <common/rune.h>
#include <common/result.h>
#include <common/id_pool.h>
#include <boost/filesystem.hpp>
#include "elements/element_types.h"

namespace basecode::compiler {

    class session;
    class variable;
    class element_map;
    class ast_evaluator;
    class scope_manager;
    class element_builder;
    class string_intern_map;
    class code_dom_formatter;

    using path_list_t = std::vector<boost::filesystem::path>;

    ///////////////////////////////////////////////////////////////////////////

    enum class type_access_model_t {
        none,
        value,
        pointer
    };

    enum class type_number_class_t {
        none,
        integer,
        floating_point,
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class identifier_usage_t : uint8_t {
        heap = 1,
        stack
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class session_compile_phase_t : uint8_t {
        start,
        success,
        failed
    };

    using session_compile_callback = std::function<void (
        session_compile_phase_t,
        const boost::filesystem::path&)>;

    struct session_options_t {
        bool verbose = false;
        size_t heap_size = 0;
        size_t stack_size = 0;
        size_t ffi_heap_size = 4096;
        bool output_ast_graphs = false;
        vm::allocator* allocator = nullptr;
        boost::filesystem::path dom_graph_file;
        boost::filesystem::path compiler_path;
        session_compile_callback compile_callback;
        std::unordered_map<std::string, std::string> definitions {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct variable_handle_t {
        variable_handle_t() = default;

        virtual ~variable_handle_t();

        void set(
            variable* instance,
            bool activate = true);

        void skip_deactivate();

        bool is_valid() const {
            return _instance != nullptr;
        }

        variable* get() const {
            return _instance;
        }

        variable* operator->() const {
            return _instance;
        }

    private:
        bool _skip_deactivate = false;
        variable* _instance = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    struct emit_context_t {
        vm::control_flow_t* flow_control = nullptr;
    };

    struct emit_result_t {
        std::vector<vm::instruction_operand_t> operands {};
    };
}