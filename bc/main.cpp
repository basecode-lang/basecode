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

#define YA_GETOPT_NO_COMPAT_MACRO

#include <chrono>
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <functional>
#include <ya_getopt.h>
#include <fmt/format.h>
#include <unordered_map>
#include <basecode/compiler.h>

namespace compiler = basecode::compiler;
namespace common = basecode::common;
namespace vm = basecode::vm;
namespace fs = boost::filesystem;

using namespace std::literals;

static constexpr size_t heap_size = (1024 * 1024) * 32;
static constexpr size_t stack_size = (1024 * 1024) * 8;

static std::string pad_to(
        const std::string& str,
        const size_t num,
        const char padding = ' ') {
    if (num > str.size()) {
        auto padded = str;
        padded.insert(padded.begin(), num - padded.size(), padding);
        return padded;
    } else {
        return str;
    }
}

static void print_results(const basecode::common::result& r) {
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
    fmt::print(
        "usage: bc "
        "[-?|--help] "
        "[-v|--verbose] "
        "[--debugger] "
        "[--no-color] "
        "[-G] "
        "[-M{{path}} ...] "
        "[-H{{filename}}|--code_dom={{filename}}] "
        "file [-- option ...]\n");
}

static size_t format_session_task(
        const compiler::session_task_t* task,
        size_t indent,
        common::term_stream* stream) {
    auto elapsed = task->elapsed.count();
    auto time_color = !task->subtasks.empty() ?
        common::term_colors_t::light_gray :
        common::term_colors_t::blue;
    std::string indent_spaces(indent, ' ');

    stream
        ->color(common::term_colors_t::default_color, common::term_colors_t::cyan)
        ->append(fmt::format("[{}] ", compiler::session_task_category_to_name(task->category)), 14)
        ->color(common::term_colors_t::default_color, common::term_colors_t::green)
        ->append(fmt::format("{}{}", indent_spaces, task->name), 54)
        ->color(common::term_colors_t::default_color, time_color)
        ->append(pad_to(std::to_string(elapsed), 12))
        ->append("\n"sv);

    for (const auto& subtask : task->subtasks)
        format_session_task(&subtask, indent + 2, stream);

    return elapsed;
}

static void output_results(
        compiler::session& session,
        common::term_stream* term_stream) {
    const auto& r = session.result();
    if (r.is_failed())
        print_results(r);
    session.finalize();

    size_t total_time = 0;

    term_stream
        ->color(common::term_colors_t::default_color, common::term_colors_t::cyan)
        ->append("\ncompiler task time breakdown (in μs):\n"sv);

    for (const compiler::session_task_t& task : session.tasks())
        total_time += format_session_task(&task, 0, term_stream);

    term_stream
        ->color(common::term_colors_t::default_color, common::term_colors_t::yellow)
        ->append(""sv, 14)
        ->append("total execution time"sv, 54)
        ->append(pad_to(std::to_string(total_time), 12))
        ->append("\n\n"sv);

    fmt::print(term_stream->format());
}

int main(int argc, char** argv) {
    using namespace std::chrono;

    const auto is_redirected = !isatty(fileno(stdout));

    int opt = -1;
    bool debugger = false;
    bool help_flag = false;
    bool verbose_flag = false;
    bool output_ast_graphs = false;
    fs::path code_dom_graph_file_name;
    bool color_enabled = !is_redirected;
    std::vector<fs::path> module_paths {};
    std::vector<fs::path> source_files {};
    std::unordered_map<std::string, std::string> definitions {};

    static struct option long_options[] = {
        {"help",    ya_no_argument,       nullptr, 0  },
        {"verbose", ya_no_argument,       nullptr, 0  },
        {"ast",     ya_no_argument,       0,       'G'},
        {"code_dom",ya_required_argument, 0,       'H'},
        {"no-color",ya_no_argument,       0,       0  },
        {"debugger",ya_no_argument,       0,       0  },
        {0,         0,                    0,       0  },
    };

    while (true) {
        int option_index = -1;
        opt = ya_getopt_long(
            argc,
            argv,
            "?vGM:H:D:",
            long_options,
            &option_index);
        if (opt == -1) {
            if (ya_optind < argc) {
                if (strcmp(argv[ya_optind - 1], "--") == 0) {
                    source_files.emplace_back(argv[ya_optind - 2]);
                } else {
                    source_files.emplace_back(argv[ya_optind]);
                }
            }
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
                    case 4:
                        color_enabled = false;
                        break;
                    case 5:
                        debugger = true;
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
            case 'M':
                module_paths.emplace_back(ya_optarg);
                break;
            case 'H':
                code_dom_graph_file_name = ya_optarg;
                break;
            case 'D': {
                auto parts = common::string_to_list(ya_optarg, '=');
                std::string value;
                if (parts.size() == 2)
                    value = parts[1];
                common::trim(parts[0]);
                definitions.insert(std::make_pair(parts[0], value));
                break;
            }
            default: {
                break;
            }
        }

    }

    if (help_flag || source_files.empty()) {
        usage();
        return 1;
    }

    std::vector<std::string> meta_options {};

    while (ya_optind < argc) {
        meta_options.emplace_back(argv[ya_optind++]);
    }

    common::term_stream_builder term_builder(color_enabled);
    vm::default_allocator allocator {};
    compiler::session_options_t session_options {
        .verbose = verbose_flag,
        .heap_size = heap_size,
        .debugger = debugger,
        .stack_size = stack_size,
        .output_ast_graphs = output_ast_graphs,
        .allocator = &allocator,
        .compiler_path = fs::system_complete(argv[0]).remove_filename(),
        .meta_options = meta_options,
        .module_paths = module_paths,
        .dom_graph_file = code_dom_graph_file_name,
        .definitions = definitions,
        .compile_callback = [&term_builder](
                compiler::session_compile_phase_t phase,
                compiler::session_module_type_t module_type,
                const fs::path& source_file) {
            switch (phase) {
                case compiler::session_compile_phase_t::start: {
                    auto file_name = source_file.filename().string();
                    std::string module_path {};
                    auto label = "program:";
                    if (module_type == compiler::session_module_type_t::module) {
                        label = " module:";
                        module_path = source_file.parent_path().filename().string();
                        if (file_name == "module.bc")
                            file_name.clear();
                        else
                            file_name = "/" + file_name;
                    }
                    fmt::print(
                        "{} {}{}\n",
                        term_builder.colorize(label, common::term_colors_t::cyan),
                        module_path,
                        file_name);
                    break;
                }
                case compiler::session_compile_phase_t::success:
                case compiler::session_compile_phase_t::failed: {
                    break;
                }
            }
        },
        .term_builder = &term_builder,
    };

    compiler::session compilation_session(
        session_options,
        source_files);

    defer({
        std::stringstream stream;
        auto term_stream = term_builder.use_stream(stream);
        output_results(compilation_session, term_stream.get());
    });

    if (!compilation_session.initialize())
        return 1;
    else
        if (!compilation_session.compile())
            return 1;

    return 0;
}