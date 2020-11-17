// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/defer.h>
#include <basecode/core/types.h>
#include <basecode/vmgen/dsl/types.h>
#include <basecode/core/format/system.h>
#include <basecode/core/memory/system.h>
#include <basecode/core/profiler/system.h>
#include <basecode/core/string/formatters.h>
#include <basecode/core/profiler/stopwatch.h>

using namespace basecode;
using namespace basecode::string;

s32 main(s32 argc, char** argv) {
    memory::initialize();
    auto ctx = context::make(memory::default_allocator());
    context::push(&ctx);
    defer(context::pop());
    defer(memory::shutdown());

    if (profiler::initialize() != profiler::init_result_t::ok)
        return 1;

    auto buf = source::buffer::make();
    auto rc = source::buffer::load("../assets/instructions.ig"_ss, buf);
    if (rc != source::buffer::load_result_t::ok)
        return 1;
    defer(source::buffer::free(buf));

    auto lexer = dsl::lexer::make(&buf);
    defer(dsl::lexer::free(lexer));

    auto parser = dsl::parser::make(&lexer);
    defer(dsl::parser::free(parser));

    profiler::stopwatch_t total_time{};
    profiler::start(total_time);

    if (!dsl::lexer::tokenize(lexer))
        return 1;

    if (!dsl::parser::parse(parser))
        return 1;

    profiler::stop(total_time);
    profiler::print_elapsed_time("total execution time"_ss, 40, profiler::elapsed(total_time));

    format::print("\n-------------\n");

    profiler::stopwatch_t ast_linear_time{};
    profiler::start(ast_linear_time);

    // ---------- page ------ ~4080   |
    //  H    F    F    F    F    F    |
    // 0..3 0..3 0..3 0..3 0..3 0..3  |
    //       id
    auto cursor = dsl::ast::first_header(parser.ast);
    while (cursor.ok) {
        format::print(
            "{}(bytes={}, field_capacity={}) {{\n",
            dsl::ast::node_header_type_name((dsl::node_header_type_t) cursor.header->type),
            cursor.header->size,
            (cursor.header->size / sizeof(dsl::node_field_t)) - 1);

        while (dsl::ast::next_field(cursor)) {
            format::print(
                "\t{:<7} = {}",
                dsl::ast::node_field_type_name((dsl::node_field_type_t) cursor.field->type),
                cursor.field->value);
            switch ((dsl::node_field_type_t) cursor.field->type) {
                case dsl::node_field_type_t::token: {
                    const auto& token = lexer.tokens[cursor.field->value - 1];
                    format::print("\n\t{:<7} = '{}'", "slice"_ss, token.slice);
                    break;
                }
                default: {
                    break;
                }
            }
            format::print("\n");
        }

        format::print("}}\n");
        dsl::ast::next_header(cursor);
    }

    profiler::stop(ast_linear_time);
    profiler::print_elapsed_time("ast linear walk time"_ss, 40, profiler::elapsed(ast_linear_time));

    profiler::stopwatch_t ast_index_time{};
    profiler::start(ast_index_time);

    for (u32 id = 1; id < parser.ast.id; ++id) {
        auto cursor1 = dsl::ast::get_header(parser.ast, id);
//        format::print(
//            "kind: {}, type: {}, size: {}\n",
//            dsl::ast::node_kind_name((dsl::node_kind_t) cursor1.header->kind),
//            dsl::ast::node_header_type_name((dsl::node_header_type_t) cursor1.header->type),
//            cursor1.header->size);
        while (dsl::ast::next_field(cursor1)) {
//            format::print(
//                "\tkind: {}, type: {}, value: {}",
//                dsl::ast::node_kind_name((dsl::node_kind_t) cursor1.field->kind),
//                dsl::ast::node_field_type_name((dsl::node_field_type_t) cursor1.field->type),
//                cursor1.field->value);
            switch ((dsl::node_field_type_t) cursor1.field->type) {
                case dsl::node_field_type_t::token: {
                    const auto& token = lexer.tokens[cursor1.field->value - 1];
//                    format::print(", slice: {}", token.slice);
                    break;
                }
                default: {
                    break;
                }
            }
//            format::print("\n");
        }
    }

    profiler::stop(ast_index_time);
    profiler::print_elapsed_time("ast index walk time"_ss, 40, profiler::elapsed(ast_index_time));

#if TRACY_ENABLE
    getc(stdin);
#endif

    return 0;
}