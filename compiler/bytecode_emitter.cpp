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
#include <parser/lexer.h>
#include "bytecode_emitter.h"
#include "constant_expression_evaluator.h"

namespace basecode::compiler {

    bytecode_emitter::bytecode_emitter(
        const bytecode_emitter_options_t& options): _terp(options.heap_size, options.stack_size),
                                                    _global_scope(nullptr, nullptr),
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

    void bytecode_emitter::build_scope_tree(
            common::result& r,
            compiler::scope* scope,
            const syntax::ast_node_shared_ptr& node) {
        if (scope == nullptr || node == nullptr)
            return;

        for (auto& child_node : node->children) {
            auto child_scope = scope->add_child_scope(child_node);
            build_scope_tree(r, child_scope, child_node);
        }
    }

    void bytecode_emitter::apply_constant_folding(
            common::result& r,
            const syntax::ast_node_shared_ptr& node) {
        constant_expression_evaluator evaluator(&_global_scope);
        auto result_node = evaluator.evaluate(r, node);
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
            build_scope_tree(r, &_global_scope, program_node);
            apply_constant_folding(r, program_node);

            if (_options.verbose) {
                auto close_required = false;
                FILE* ast_output_file = stdout;
                if (!_options.ast_graph_file_name.empty()) {
                    ast_output_file = fopen(_options.ast_graph_file_name.c_str(), "wt");
                    close_required = true;
                }
                syntax::ast_formatter formatter(program_node, ast_output_file);
                formatter.format_graph_viz();

                if (close_required)
                    fclose(ast_output_file);
            }
        }
        return !r.is_failed();
    }

}