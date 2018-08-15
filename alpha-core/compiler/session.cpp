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
#include "elements/module.h"
#include "elements/program.h"
#include "code_dom_formatter.h"

namespace basecode::compiler {

    session::session(
            const session_options_t& options,
            const path_list_t& source_files) : _terp(options.heap_size, options.stack_size),
                                               _assembler(&_terp),
                                               _program(&_terp, &_assembler),
                                               _options(options) {
        for (const auto& path : source_files) {
            if (path.is_relative()) {
                add_source_file(boost::filesystem::absolute(path));
            } else {
                add_source_file(path);
            }
        }
    }

    session::~session() {
    }

    vm::terp& session::terp() {
        return _terp;
    }

    void session::raise_phase(
            session_compile_phase_t phase,
            const boost::filesystem::path& source_file) {
        if (_options.compile_callback == nullptr)
            return;
        _options.compile_callback(phase, source_file);
    }

    void session::finalize() {
        if (_options.verbose) {
            try {
                _program.disassemble(stdout);
                if (!_options.dom_graph_file.empty())
                    write_code_dom_graph(_options.dom_graph_file);
            } catch (const fmt::format_error& e) {
                fmt::print("fmt::format_error caught: {}\n", e.what());
            }
        }
    }

    bool session::compile() {
        return _program.compile(*this);
    }

    bool session::initialize() {
        _terp.initialize(_result);
        _assembler.initialize(_result);
        return !_result.is_failed();
    }

    common::result& session::result() {
        return _result;
    }

    vm::assembler& session::assembler() {
        return _assembler;
    }

    compiler::program& session::program() {
        return _program;
    }

    void session::error(
            const std::string& code,
            const std::string& message,
            const common::source_location& location) {
        auto source_file = current_source_file();
        if (source_file == nullptr)
            return;
        source_file->error(_result, code, message, location);
    }

    void session::error(
            compiler::element* element,
            const std::string& code,
            const std::string& message,
            const common::source_location& location) {
        element->module()
                ->source_file()
                ->error(_result, code, message, location);
    }

    common::source_file* session::pop_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        auto source_file = _source_file_stack.top();
        _source_file_stack.pop();
        return source_file;
    }

    const session_options_t& session::options() const {
        return _options;
    }

    common::source_file* session::current_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        return _source_file_stack.top();
    }

    std::vector<common::source_file*> session::source_files() {
        std::vector<common::source_file*> list {};
        for (auto& it : _source_files) {
            list.push_back(&it.second);
        }
        return list;
    }

    void session::push_source_file(common::source_file* source_file) {
        _source_file_stack.push(source_file);
    }

    void session::write_code_dom_graph(const boost::filesystem::path& path) {
        FILE* output_file = nullptr;
        if (!path.empty()) {
            output_file = fopen(path.string().c_str(), "wt");
        }
        defer({
            if (output_file != nullptr)
                fclose(output_file);
        });

        compiler::code_dom_formatter formatter(&_program, output_file);
        formatter.format(fmt::format("Code DOM Graph: {}", path.string()));
    }

    syntax::ast_node_shared_ptr session::parse(common::source_file* source_file) {
        raise_phase(session_compile_phase_t::start, source_file->path());
        defer({
            if (_result.is_failed()) {
              raise_phase(session_compile_phase_t::failed, source_file->path());
            } else {
              raise_phase(session_compile_phase_t::success, source_file->path());
            }
        });

        if (source_file->empty()) {
            if (!source_file->load(_result))
                return nullptr;
        }

        syntax::parser alpha_parser(source_file);
        auto module_node = alpha_parser.parse(_result);
        if (module_node != nullptr && !_result.is_failed()) {
            if (_options.output_ast_graphs) {
                boost::filesystem::path ast_file_path(source_file->path().parent_path());
                auto filename = source_file->path()
                    .filename()
                    .replace_extension("")
                    .string();
                filename += "-ast";
                ast_file_path.append(filename);
                ast_file_path.replace_extension(".dot");
                alpha_parser.write_ast_graph(
                    ast_file_path,
                    module_node);
            }
        }

        return module_node;
    }

    syntax::ast_node_shared_ptr session::parse(const boost::filesystem::path& path) {
        auto source_file = find_source_file(path);
        if (source_file == nullptr) {
            source_file = add_source_file(path);
        }
        return parse(source_file);
    }

    common::source_file* session::add_source_file(const boost::filesystem::path& path) {
        auto it = _source_files.insert(std::make_pair(
            path.string(),
            common::source_file(path)));
        if (!it.second)
            return nullptr;
        return &it.first->second;
    }

    common::source_file* session::find_source_file(const boost::filesystem::path& path) {
        auto it = _source_files.find(path.string());
        if (it == _source_files.end())
            return nullptr;
        return &it->second;
    }

};