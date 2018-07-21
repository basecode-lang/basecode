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

#include <cstdio>
#include <string>
#include <vm/terp.h>
#include <filesystem>
#include <fmt/format.h>
#include <vm/assembler.h>
#include <common/defer.h>
#include <parser/parser.h>
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
            const std::filesystem::path& source_file);

        bool initialize(common::result& r);

        const path_list_t& source_files() const;

        const session_options_t& options() const;

    private:
        void write_code_dom_graph(const std::filesystem::path& path);

    private:
        vm::terp _terp;
        vm::assembler _assembler;
        compiler::program _program;
        path_list_t _source_files {};
        session_options_t _options {};
    };

};

