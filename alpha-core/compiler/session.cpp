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

#include <fstream>
#include "session.h"
#include "elements/program.h"
#include "code_dom_formatter.h"

namespace basecode::compiler {

    session::session(
            const session_options_t& options,
            const path_list_t& source_files) : _source_files(source_files),
                                               _options(options) {
    }

    session::~session() {
    }

    void session::raise_phase(
            session_compile_phase_t phase,
            const std::filesystem::path& source_file) {
        if (_options.compile_callback == nullptr)
            return;
        _options.compile_callback(phase, source_file);
    }

    void session::finalize() {
        _listing.write(stdout);
    }

    void session::write_code_dom_graph(
            compiler::program* program,
            const std::filesystem::path& path) {
        FILE* output_file = nullptr;
        if (!path.empty()) {
            output_file = fopen(path.c_str(), "wt");
        }
        defer({
            if (output_file != nullptr)
                fclose(output_file);
        });

        compiler::code_dom_formatter formatter(program, output_file);
        formatter.format(fmt::format("Code DOM Graph: {}", path.string()));
    }

    syntax::ast_node_shared_ptr session::parse(
            common::result& r,
            const std::filesystem::path& source_file) {
        std::ifstream input_stream(source_file);
        if (input_stream.is_open()) {
            syntax::parser alpha_parser(input_stream);
            auto module_node = alpha_parser.parse(r);
            if (module_node != nullptr && !r.is_failed()) {
                if (_options.verbose && !_options.ast_graph_file.empty())
                    alpha_parser.write_ast_graph(_options.ast_graph_file, module_node);
            }
            return module_node;
        } else {
            r.add_message(
                "S001",
                fmt::format("unable to open source file: {}", source_file.string()),
                true);
        }
        return nullptr;
    }

    vm::assembly_listing& session::listing() {
        return _listing;
    }

    const path_list_t& session::source_files() const {
        return _source_files;
    }

    const session_options_t& session::options() const {
        return _options;
    }

    void session::post_processing(compiler::program* program) {
        if (_options.verbose && !_options.dom_graph_file.empty()) {
            write_code_dom_graph(program, _options.dom_graph_file);
        }
    }

};