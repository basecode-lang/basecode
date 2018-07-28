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
#include <cstdlib>
#include <functional>
#include <fmt/format.h>
#include <basecode/compiler.h>
#include "ya_getopt.h"

static constexpr size_t heap_size = (1024 * 1024) * 32;
static constexpr size_t stack_size = (1024 * 1024) * 8;

static void print_results(basecode::common::result& r) {
	const auto has_messages = !r.messages().empty();

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
               " file\n");
}

int main(int argc, char** argv) {
    using namespace std::chrono;

	auto opt = -1;
	auto help_flag = false;
	auto verbose_flag = false;
    boost::filesystem::path ast_graph_file_name;
    boost::filesystem::path code_dom_graph_file_name;

    static struct option long_options[] = {
        {"help",    ya_no_argument,       nullptr, 0  },
        {"verbose", ya_no_argument,       nullptr, 0  },
        {"ast",     ya_required_argument, 0,       'G'},
        {"code_dom",ya_required_argument, 0,       'H'},
        {0,         0,                    0,       0  },
    };

    while (true) {
        int option_index = -1;
        opt = ya_getopt_long(
            argc,
            argv,
            "?vG:H:",
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
                        ast_graph_file_name = ya_optarg;
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
                ast_graph_file_name = ya_optarg;
                break;
            case 'H':
                code_dom_graph_file_name = ya_optarg;
                break;
            default:
                break;
        }
    }

    if (help_flag) {
        usage();
        return 1;
    }

	const auto start = high_resolution_clock::now();
    defer({
        const auto end = high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        fmt::print("\ncompilation time (in μs): {}\n", duration);
    });

    std::vector<boost::filesystem::path> source_files {};
    while (ya_optind < argc) {
	    const boost::filesystem::path source_file_path(argv[ya_optind++]);
        source_files.push_back(source_file_path);
    }

    if (source_files.empty()) {
        usage();
        return 1;
    }

	basecode::compiler::session_options_t session_options{};
    session_options.verbose = verbose_flag,
	session_options.heap_size = heap_size,
	session_options.stack_size = stack_size,
	session_options.ast_graph_file = ast_graph_file_name,
	session_options.dom_graph_file = code_dom_graph_file_name,
	session_options.compile_callback = [&](
            basecode::compiler::session_compile_phase_t phase,
            const boost::filesystem::path& source_file) {
        switch (phase) {
            case basecode::compiler::session_compile_phase_t::start:
                fmt::print("{}\n", source_file.filename().string());
                break;
            case basecode::compiler::session_compile_phase_t::success:
            case basecode::compiler::session_compile_phase_t::failed:
                break;
        }
	};

    basecode::compiler::session compilation_session(
        session_options,
        source_files);
    basecode::common::result r;
    if (!compilation_session.initialize(r)) {
        print_results(r);
        return 1;
    } else {
        if (!compilation_session.compile(r)) {
            print_results(r);
            return 1;
        } else {
            fmt::print("\n");
            compilation_session.finalize();
            return 0;
       }
    }
}