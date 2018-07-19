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
#include <filesystem>
#include <fmt/format.h>
#include <common/defer.h>
#include <parser/parser.h>
#include <vm/assembly_listing.h>
#include "elements/element_types.h"

namespace basecode::compiler {

    using path_list_t = std::vector<std::filesystem::path>;

    enum class session_compile_phase_t : uint8_t {
        start,
        success,
        failed
    };

    using session_compile_callback = std::function<void (
        session_compile_phase_t,
        const std::filesystem::path&)>;

    struct session_options_t {
        bool verbose = false;
        std::filesystem::path ast_graph_file;
        std::filesystem::path dom_graph_file;
        session_compile_callback compile_callback;
    };

    class session {
    public:
        session(
            const session_options_t& options,
            const path_list_t& source_files);

        virtual ~session();

        void finalize();

        void raise_phase(
            session_compile_phase_t phase,
            const std::filesystem::path& source_file);

        vm::assembly_listing& listing();

        syntax::ast_node_shared_ptr parse(
            common::result& r,
            const std::filesystem::path& source_file);

        const path_list_t& source_files() const;

        const session_options_t& options() const;

        void post_processing(compiler::program* program);

    private:
        void write_code_dom_graph(
            compiler::program* program,
            const std::filesystem::path& path);

    private:
        path_list_t _source_files {};
        session_options_t _options {};
        vm::assembly_listing _listing {};
    };

};

