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
#include <cstdio>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <common/defer.h>
#include <boost/filesystem.hpp>
#include "compiler_types.h"

namespace basecode::compiler {

    class session final {
    public:
        static constexpr uint8_t trap_putc = 0x01;
        static constexpr uint8_t trap_getc = 0x02;

    public:
        session(
            const session_options_t& options,
            const path_list_t& source_files);

        ~session();

        bool run();

        void error(
            compiler::module* module,
            const std::string& code,
            const std::string& message,
            const common::source_location& location);

        vm::ffi& ffi();

        bool compile();

        void finalize();

        bool time_task(
            const std::string& name,
            const session_task_callable_t& callable,
            bool include_in_total = true);

        vm::terp& terp();

        bool initialize();

        void enable_run();

        element_map& elements();

        common::result& result();

        type_list_t used_types();

        element_builder& builder();

        vm::assembler& assembler();

        ast_evaluator& evaluator();

        vm::allocator* allocator();

        compiler::program& program();

        void disassemble(FILE* file);

        syntax::ast_builder& ast_builder();

        const element_map& elements() const;

        common::source_file* pop_source_file();

        const path_list_t& source_files() const;

        compiler::scope_manager& scope_manager();

        const session_options_t& options() const;

        const compiler::program& program() const;

        const session_task_list_t& tasks() const;

        common::source_file* current_source_file();

        void track_used_type(compiler::type* type);

        compiler::byte_code_emitter& byte_code_emitter();

        const string_intern_map& interned_strings() const;

        common::source_file* source_file(common::id_t id);

        const compiler::scope_manager& scope_manager() const;

        void push_source_file(common::source_file* source_file);

        syntax::ast_node_t* parse(common::source_file* source_file);

        common::id_t intern_string(compiler::string_literal* literal);

        syntax::ast_node_t* parse(const boost::filesystem::path& path);

        compiler::module* compile_module(common::source_file* source_file);

        common::source_file* source_file(const boost::filesystem::path& path);

        common::source_file* add_source_file(const boost::filesystem::path& path);

    private:
        void raise_phase(
            session_compile_phase_t phase,
            session_module_type_t module_type,
            const boost::filesystem::path& source_file);

        bool type_check();

        bool execute_directives();

        void initialize_core_types();

        bool resolve_assembly_symbol(
            vm::assembly_symbol_type_t type,
            void* data,
            const std::string& symbol,
            vm::assembly_symbol_result_t& result);

        bool fold_constant_expressions();

        bool resolve_unknown_identifiers();

        void initialize_built_in_procedures();

        bool resolve_unknown_types(bool final);

        bool fold_elements_of_type(element_type_t type);

        bool should_read_variable(compiler::element* element);

        void write_code_dom_graph(const boost::filesystem::path& path);

    private:
        bool _run = false;
        ast_map_t _asts {};
        common::result _result;
        vm::ffi* _ffi = nullptr;
        module_map_t _modules {};
        vm::terp* _terp = nullptr;
        type_set_t _used_types {};
        path_list_t _source_files {};
        session_options_t _options {};
        session_task_list_t _tasks {};
        element_map* _elements = nullptr;
        element_builder* _builder = nullptr;
        vm::assembler* _assembler = nullptr;
        compiler::program* _program = nullptr;
        ast_evaluator* _ast_evaluator = nullptr;
        source_file_stack_t _source_file_stack {};
        source_file_map_t _source_file_registry {};
        syntax::ast_builder* _ast_builder = nullptr;
        source_file_path_map_t _source_file_paths {};
        string_intern_map* _interned_strings = nullptr;
        compiler::byte_code_emitter* _emitter = nullptr;
        compiler::scope_manager* _scope_manager = nullptr;
    };

};

