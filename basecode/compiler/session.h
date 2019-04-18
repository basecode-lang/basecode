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
            path_list_t source_files);

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
            session_task_category_t category,
            const std::string& name,
            const session_task_callable_t& callable);

        vm::terp& terp();

        bool initialize();

        void enable_run();

        element_map& elements();

        vm::label_map& labels();

        string_set_t& strings();

        common::result& result();

        type_list_t used_types();

        element_builder& builder();

        vm::assembler& assembler();

        ast_evaluator& evaluator();

        vm::allocator* allocator();

        compiler::program& program();

        void disassemble(FILE* file);

        session_task_t* current_task();

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

        vm::register_allocator* register_allocator();

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
        static bool should_read_variable(compiler::element* element);

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

        bool resolve_unknown_types(bool final);

        bool fold_elements_of_type(element_type_t type);

        void write_code_dom_graph(const boost::filesystem::path& path);

    private:
        bool _run = false;
        ast_map_t _asts {};
        common::result _result;
        vm::ffi* _ffi = nullptr;
        parser_map_t _parsers{};
        module_map_t _modules {};
        string_set_t _strings {};
        vm::terp* _terp = nullptr;
        type_set_t _used_types {};
        path_list_t _source_files {};
        session_options_t _options {};
        session_task_list_t _tasks {};
        element_map* _elements = nullptr;
        vm::label_map* _labels = nullptr;
        element_builder* _builder = nullptr;
        vm::assembler* _assembler = nullptr;
        session_task_stack_t _task_stack {};
        compiler::program* _program = nullptr;
        ast_evaluator* _ast_evaluator = nullptr;
        source_file_stack_t _source_file_stack {};
        source_file_map_t _source_file_registry {};
        syntax::ast_builder* _ast_builder = nullptr;
        source_file_path_map_t _source_file_paths {};
        string_intern_map* _interned_strings = nullptr;
        compiler::byte_code_emitter* _emitter = nullptr;
        compiler::scope_manager* _scope_manager = nullptr;
        vm::register_allocator* _register_allocator = nullptr;
    };

}

