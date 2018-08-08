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

#include <chrono>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <vm/terp.h>
#include <functional>
#include <fmt/format.h>
#include <unordered_map>
#include <common/colorizer.h>
#include <compiler/session.h>
#include <common/source_file.h>
#include <common/hex_formatter.h>
#include <common/string_support.h>
#include "ya_getopt.h"

static constexpr size_t heap_size = (1024 * 1024) * 32;
static constexpr size_t stack_size = (1024 * 1024) * 8;

static void print_results(basecode::common::result& r) {
    auto has_messages = !r.messages().empty();

    if (has_messages)
        fmt::print("\n");

    auto messages = r.messages();
    for (size_t i = 0; i < messages.size(); i++) {
        const auto& msg = messages[i];
        fmt::print(
            "[{}] {}{}\n",
            msg.code(),
            msg.is_error() ? "ERROR: " : "WARNING: ",
            msg.message());
        if (!msg.details().empty()) {
            fmt::print("{}\n", msg.details());
        }
        if (i < messages.size() - 1)
            fmt::print("\n");
    }
}

static void usage() {
    fmt::print("usage: bac "
               "[-?|--help] "
               "[-v|--verbose] "
               "[-G{{filename}}|--ast={{filename}}] "
               "[-H{{filename}}|--code_dom={{filename}}] "
               "file\n");
}

int main(int argc, char** argv) {
    using namespace std::chrono;

    int opt = -1;
    bool help_flag = false;
    bool verbose_flag = false;
    bool output_ast_graphs = false;
    boost::filesystem::path code_dom_graph_file_name;
    std::unordered_map<std::string, std::string> definitions {};

    static struct option long_options[] = {
        {"help",    ya_no_argument,       nullptr, 0  },
        {"verbose", ya_no_argument,       nullptr, 0  },
        {"ast",     ya_no_argument,       0,       'G'},
        {"code_dom",ya_required_argument, 0,       'H'},
        {0,         0,                    0,       0  },
    };

    while (true) {
        int option_index = -1;
        opt = ya_getopt_long(
            argc,
            argv,
            "?vGH:D:",
            long_options,
            &option_index);
        if (opt == -1) {
            break;
        }

        switch (opt) {
            case 0: {
                switch (option_index) {
                    case 0:
                        help_flag = true;
                        break;
                    case 1:
                        verbose_flag = true;
                        break;
                    case 2:
                        output_ast_graphs = true;
                        break;
                    case 3:
                        code_dom_graph_file_name = ya_optarg;
                        break;
                    default:
                        abort();
                }
                break;
            }
            case '?':
                help_flag = true;
                break;
            case 'v':
                verbose_flag = true;
                break;
            case 'G':
                output_ast_graphs = true;
                break;
            case 'H':
                code_dom_graph_file_name = ya_optarg;
                break;
            case 'D': {
                auto parts = basecode::common::string_to_list(ya_optarg, '=');
                std::string value;
                if (parts.size() == 2)
                    value = parts[1];
                basecode::common::trim(parts[0]);
                definitions.insert(std::make_pair(parts[0], value));
                break;
            }
            default:
                break;
        }
    }

    if (help_flag) {
        usage();
        return 1;
    }

    high_resolution_clock::time_point start = high_resolution_clock::now();
    defer({
        high_resolution_clock::time_point end = high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        fmt::print(
            "\n{} {}\n",
            basecode::common::colorizer::colorize(
                "compilation time (in μs):",
                basecode::common::term_colors_t::cyan),
            duration);
    });

    std::vector<boost::filesystem::path> source_files {};
    while (ya_optind < argc) {
        boost::filesystem::path source_file_path(argv[ya_optind++]);
        source_files.push_back(source_file_path);
    }

    if (source_files.empty()) {
        usage();
        return 1;
    }

    basecode::compiler::session_options_t session_options {
        .verbose = verbose_flag,
        .heap_size = heap_size,
        .stack_size = stack_size,
        .output_ast_graphs = output_ast_graphs,
        .dom_graph_file = code_dom_graph_file_name,
        .compiler_path = boost::filesystem::system_complete(argv[0]).remove_filename(),
        .compile_callback = [&](
                basecode::compiler::session_compile_phase_t phase,
                const boost::filesystem::path& source_file) {
            switch (phase) {
                case basecode::compiler::session_compile_phase_t::start:
                    fmt::print(
                        "{} {}\n",
                        basecode::common::colorizer::colorize(
                            "module:",
                            basecode::common::term_colors_t::cyan),
                        source_file.filename().string());
                    break;
                case basecode::compiler::session_compile_phase_t::success:
                case basecode::compiler::session_compile_phase_t::failed:
                    break;
            }
        },
        .definitions = definitions
    };

    basecode::compiler::session compilation_session(
        session_options,
        source_files);
    basecode::common::result r;

    defer({
        if (r.is_failed())
            print_results(r);
        compilation_session.finalize();
    });

    if (!compilation_session.initialize(r)) {
        return 1;
    } else {
        if (!compilation_session.compile(r))
            return 1;
    }

    return 0;
}