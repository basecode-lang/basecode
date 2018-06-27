// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include <fstream>
#include <fmt/ostream.h>
#include <parser/lexer.h>
#include <parser/ast_formatter.h>
#include <compiler/elements/program.h>
#include "bytecode_emitter.h"
#include "code_dom_formatter.h"

namespace basecode::compiler {

    bytecode_emitter::bytecode_emitter(
        const bytecode_emitter_options_t& options): _terp(options.heap_size, options.stack_size),
                                                    _options(options) {
    }

    bytecode_emitter::~bytecode_emitter() {
    }

    bool bytecode_emitter::compile_files(
            common::result& r,
            const std::vector<std::filesystem::path>& source_files) {
        for (const auto& source_file : source_files) {
            fmt::print("Compile: {} ... ", source_file.string());
            if (!std::filesystem::exists(source_file)) {
                fmt::print("FAILED, file does not exist.\n");
                continue;
            }
            std::ifstream input_stream(source_file);
            if (input_stream.is_open()) {
                if (!compile(r, input_stream))
                    fmt::print("FAILED.\n");
                else
                    fmt::print("PASSED.\n");
                input_stream.close();
            }
        }
        return !r.is_failed();
    }

    bool bytecode_emitter::initialize(common::result& r) {
        return _terp.initialize(r);
    }

    bool bytecode_emitter::compile(common::result& r, std::istream& input) {
        compile_stream(r, input);
        return !r.is_failed();
    }

    bool bytecode_emitter::compile_stream(common::result& r, std::istream& input) {
        syntax::parser alpha_parser(input);
        auto program_node = alpha_parser.parse(r);
        if (program_node != nullptr && !r.is_failed()) {
            if (_options.verbose) {
                auto close_required = false;
                FILE* ast_output_file = stdout;
                if (!_options.ast_graph_file_name.empty()) {
                    ast_output_file = fopen(
                        _options.ast_graph_file_name.c_str(),
                        "wt");
                    close_required = true;
                }

                syntax::ast_formatter formatter(
                    program_node,
                    ast_output_file);
                formatter.format(fmt::format(
                    "AST Graph: {}",
                    _options.ast_graph_file_name.string()));

                if (close_required)
                    fclose(ast_output_file);
            }

            compiler::program program(_options);
            if (program.initialize(r, program_node)) {
                if (_options.verbose) {
                    auto close_required = false;
                    FILE* code_dom_output_file = stdout;
                    if (!_options.code_dom_graph_file_name.empty()) {
                        code_dom_output_file = fopen(
                            _options.code_dom_graph_file_name.c_str(),
                            "wt");
                        close_required = true;
                    }

                    compiler::code_dom_formatter formatter(
                        &program,
                        code_dom_output_file);
                    formatter.format(fmt::format(
                        "Code DOM Graph: {}",
                        _options.code_dom_graph_file_name.string()));

                    if (close_required)
                        fclose(code_dom_output_file);
                }
            }
        }

        return !r.is_failed();
    }

}