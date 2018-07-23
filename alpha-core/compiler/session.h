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
#include <vm/terp.h>
#include <filesystem>
#include <fmt/format.h>
#include <vm/assembler.h>
#include <common/defer.h>
#include <parser/parser.h>
#include <common/source_file.h>
#include "compiler_types.h"
#include "elements/program.h"

namespace basecode::compiler {

    class session {
    public:
        session(
            const session_options_t& options,
            const path_list_t& source_files);

        virtual ~session();

        void finalize();

        vm::terp& terp();

        void raise_phase(
            session_compile_phase_t phase,
            const std::filesystem::path& source_file);

        vm::assembler& assembler();

        compiler::program& program();

        bool compile(common::result& r);

        syntax::ast_node_shared_ptr parse(
            common::result& r,
            const std::filesystem::path& path);

        syntax::ast_node_shared_ptr parse(
            common::result& r,
            common::source_file* source_file);

        bool initialize(common::result& r);

        common::source_file* pop_source_file();

        const session_options_t& options() const;

        common::source_file* current_source_file();

        std::vector<common::source_file*> source_files();

        void push_source_file(common::source_file* source_file);

        common::source_file* add_source_file(const std::filesystem::path& path);

        common::source_file* find_source_file(const std::filesystem::path& path);

    private:
        void write_code_dom_graph(const std::filesystem::path& path);

    private:
        vm::terp _terp;
        vm::assembler _assembler;
        compiler::program _program;
        session_options_t _options {};
        std::stack<common::source_file*> _source_file_stack {};
        std::map<std::string, common::source_file> _source_files {};
    };

};

