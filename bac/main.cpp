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
#include <common/hex_formatter.h>
#include <vm/instruction_emitter.h>
#include <compiler/bytecode_emitter.h>
#include "ya_getopt.h"

static constexpr size_t heap_size = (1024 * 1024) * 32;
static constexpr size_t stack_size = (1024 * 1024) * 8;

static void print_results(basecode::common::result& r) {
    auto has_messages = !r.messages().empty();

    if (has_messages)
        fmt::print("\n");

    for (const auto& msg : r.messages()) {
        fmt::print(
            "[{}] {}{}\n",
            msg.code(),
            msg.is_error() ? "ERROR: " : "WARNING: ",
            msg.message());
        if (!msg.details().empty()) {
            fmt::print("{}\n", msg.details());
        }
    }

    if (has_messages)
        fmt::print("\n");
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

    int opt = -1;
    bool help_flag = false;
    bool verbose_flag = false;
    std::filesystem::path ast_graph_file_name;
    std::filesystem::path code_dom_graph_file_name;

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

    high_resolution_clock::time_point start = high_resolution_clock::now();

    basecode::compiler::bytecode_emitter_options_t compiler_options {
        .verbose = verbose_flag,
        .heap_size = heap_size,
        .stack_size = stack_size,
        .ast_graph_file_name = ast_graph_file_name,
        .code_dom_graph_file_name = code_dom_graph_file_name,
    };
    basecode::compiler::bytecode_emitter compiler(compiler_options);
    basecode::common::result r;
    int rc = 0;

    if (!compiler.initialize(r)) {
        rc = 1;
    } else {
        std::vector<std::filesystem::path> source_files {};
        while (ya_optind < argc) {
            std::filesystem::path source_file_path(argv[ya_optind++]);
            source_files.push_back(source_file_path);
        }

        if (source_files.empty()) {
            usage();
            rc = 1;
        } else {
            basecode::vm::assembly_listing listing {};
            if (!compiler.compile_files(r, listing, source_files)) {
                rc = 1;
            } else {
                high_resolution_clock::time_point end = high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                fmt::print("compilation time (in μs): {}\n", duration);
                fmt::print("\n");
                listing.write(stdout);
            }
        }
    }

    print_results(r);

    return rc;
}