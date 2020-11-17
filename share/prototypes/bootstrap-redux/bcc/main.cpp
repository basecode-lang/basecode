// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#define YA_GETOPT_NO_COMPAT_MACRO
#include <ya_getopt.h>

#include <spdlog/logger.h>
#include <basecode/defer.h>
#include <basecode/result.h>
#include <basecode/signals/hook.h>
#include <basecode/memory/system.h>
#include <basecode/errors/errors.h>
#include <basecode/format/format.h>
#include <basecode/context/context.h>
#include <basecode/workspace/session.h>
#include <basecode/strings/transforms.h>
#include <basecode/logging/spd_logger.h>
#include <basecode/memory/trace_allocator.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <basecode/language/core/lexer/lexer.h>
#include <basecode/language/core/parser/parser.h>

#define TRACE_ALLOCATOR 0
using namespace basecode;

[[maybe_unused]] static void usage() {
    format::print(
        "usage: bcc "
        "[-?|--help] "
        "[-v|--verbose] "
        "[--no-color] "
        "[-G] "
        "[-M{{path}} ...] "
        "file [-- option ...]\n");
}

[[maybe_unused]] static void print_results(const result_t& r) {
    const auto has_messages = !r.messages().empty();

    if (has_messages)
        format::print("\n");

    auto messages = r.messages();
    for (size_t i = 0; i < messages.size(); i++) {
        const auto& msg = messages[i];
        format::print(
            "[{}] {}{}\n",
            msg.code(),
            msg.is_error() ? "ERROR: " : "WARNING: ",
            msg.message());
        if (!msg.details().empty())
            format::print("{}\n", msg.details());
        if (i < messages.size() - 1)
            format::print("\n");
    }
}

int run(int argc, char* argv[]) {
    const auto is_redirected = !isatty(fileno(stdout));

    int opt = -1;
    bool help_flag{};
    bool verbose_flag{};
    path_list_t module_paths{};
    path_list_t source_files{};
    string_map_t definitions{};
    bool color_enabled = !is_redirected;

    static struct option long_options[] = {
        {"help",    ya_no_argument,       nullptr, 0  },
        {"verbose", ya_no_argument,       nullptr, 0  },
        {"no-color",ya_no_argument,       0,       0  },
        {0, 0,                    0,       0  },
    };

    while (true) {
        int option_index = -1;
        opt = ya_getopt_long(
            argc,
            argv,
            "?vM:D:",
            long_options,
            &option_index);
        if (opt == -1) {
            if (ya_optind < argc) {
                if (strcmp(argv[ya_optind - 1], "--") == 0) {
                    source_files.emplace(argv[ya_optind - 2]);
                } else {
                    source_files.emplace(argv[ya_optind]);
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
                        color_enabled = false;
                        break;
                    default:
                        usage();
                        return 1;
                }
                break;
            }
            case '?':
                help_flag = true;
                break;
            case 'v':
                verbose_flag = true;
                break;
            case 'M':
                module_paths.emplace(ya_optarg);
                break;
            case 'D': {
                string_t arg(ya_optarg, strlen(ya_optarg));
                const auto& parts = strings::string_to_list(arg, '=');
                string_t key, value;
                key = parts[0];
                key.trim();
                if (parts.size() == 2)
                    value = parts[1];
                definitions.insert(key, value);
                break;
            }
            default: {
                usage();
                return 1;
            }
        }

    }

    if (help_flag || source_files.empty()) {
        usage();
        return 1;
    }

    string_list_t meta_options {};

    while (ya_optind < argc) {
        const auto arg = argv[ya_optind++];
        meta_options.emplace(arg, strlen(arg));
    }

    workspace::session_options_t options{};
    workspace::session_t session(options);
    utf8::source_buffer_t buffer(options.allocator);
    result_t r(options.allocator);

    defer(print_results(r));

    const auto& file = source_files[0];
    if (!buffer.load(r, file))
        return 1;

    entity_list_t tokens{};
    language::core::lexer::lexer_t lexer(
        session,
        buffer);
    if (!lexer.tokenize(r, tokens))
        return 1;

    language::core::parser::parser_t parser(
        session,
        buffer,
        tokens);
    if (!parser.initialize(r))
        return 1;

    entity_t module_node = parser.parse(r);
    if (module_node == null_entity)
        return 1;

    language::core::ast::write_dot_graph(
        r,
        session,
        "test.dot",
        module_node);

    return 0;
}

int main(int argc, char* argv[]) {
    int rc = 0;

    memory::initialize();
    defer({
        memory::shutdown();
        context::shutdown();
    });

    {
        memory::allocator_t* default_allocator{};
#if TRACE_ALLOCATOR
        memory::trace_allocator_t tracer(memory::default_scratch_allocator());
        default_allocator = &tracer;
#else
        default_allocator = memory::default_scratch_allocator();
#endif
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "../logs/bcc.log",
            (1024*1024) * 5,
            32);
        file_sink->set_level(spdlog::level::info);

        spdlog::logger spd_logger(
            "bcc",
            {console_sink, file_sink});
        spd_logger.set_level(spdlog::level::info);

        logging::spd_logger_t logger(&spd_logger);
        context::initialize(default_allocator, &logger);

        result_t r;
        if (!errors::initialize(r)) return 1;
        if (!signals::initialize(r)) return 1;

        rc = run(argc, argv);

        errors::shutdown();
        signals::shutdown();
    }

    return rc;
}