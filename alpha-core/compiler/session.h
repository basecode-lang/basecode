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
#include <fmt/format.h>
#include <vm/assembler.h>
#include <common/defer.h>
#include <parser/parser.h>
#include <boost/filesystem.hpp>
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

        bool run();

        void error(
            const std::string& code,
            const std::string& message,
            const common::source_location& location);

        void error(
            compiler::element* element,
            const std::string& code,
            const std::string& message,
            const common::source_location& location);

        bool compile();

        void finalize();

        vm::terp& terp();

        bool initialize();

        common::result& result();

        vm::assembler& assembler();

        compiler::program& program();

        emit_context_t& emit_context();

        common::source_file* pop_source_file();

        const session_options_t& options() const;

        common::source_file* current_source_file();

        std::vector<common::source_file*> source_files();

        void push_source_file(common::source_file* source_file);

        syntax::ast_node_shared_ptr parse(common::source_file* source_file);

        syntax::ast_node_shared_ptr parse(const boost::filesystem::path& path);

        common::source_file* add_source_file(const boost::filesystem::path& path);

        common::source_file* find_source_file(const boost::filesystem::path& path);

    private:
        void raise_phase(
            session_compile_phase_t phase,
            const boost::filesystem::path& source_file);

        void write_code_dom_graph(const boost::filesystem::path& path);

    private:
        vm::terp _terp;
        common::result _result;
        vm::assembler _assembler;
        compiler::program _program;
        emit_context_t _emit_context;
        session_options_t _options {};
        std::stack<common::source_file*> _source_file_stack {};
        std::map<std::string, common::source_file> _source_files {};
    };

};

