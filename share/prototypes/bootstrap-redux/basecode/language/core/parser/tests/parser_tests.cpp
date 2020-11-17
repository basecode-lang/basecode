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

#include <catch2/catch.hpp>
#include <basecode/defer.h>
#include <basecode/format/format.h>
#include <basecode/language/core/lexer/lexer.h>
#include <basecode/language/core/parser/parser.h>

namespace basecode {

    using namespace std::literals;
    using namespace basecode;
    using namespace basecode::language::core;

    TEST_CASE("parser_t::parse identifier & identifier_references") {
        workspace::session_options_t options{};
        workspace::session_t session(options);
        utf8::source_buffer_t buffer(options.allocator);
        result_t r(options.allocator);

        defer(fmt::print("{}", r));

        const adt::string_t source =
            "c := a * b;"sv
        ;

        REQUIRE(buffer.load(r, source));

        entity_list_t tokens{};
        lexer::lexer_t lexer(session, buffer);
        REQUIRE(lexer.tokenize(r, tokens));
        REQUIRE(!r.is_failed());
        REQUIRE(r.messages().empty());

        parser::parser_t parser(session, buffer, tokens);
        REQUIRE(parser.initialize(r));

        entity_t module_node = parser.parse(r);
        REQUIRE(module_node != (entity_t)entt::null);

        ast::write_dot_graph(
            r,
            session,
            "test-identifier.dot",
            module_node);
    }

    TEST_CASE("parser_t::parse produces valid ast") {
        workspace::session_options_t options{};
        workspace::session_t session(options);
        utf8::source_buffer_t buffer(options.allocator);
        result_t r(options.allocator);

        defer(fmt::print("{}", r));

        const adt::string_t source =
            "// this is a line comment\n"
            "// which should accumulate into the first statement, below.\n"
            "-- testing alternate line comment syntax\n"
            "@no_fold \"foo\";\n"
            "-- constants\n"
            "true;\n"
            "false;\n"
            "nil;\n"
            "/* floating point number literals */\n"
            "3.145;\n"
            "-3.145;\n"
            "-- unary operators\n"
            "h := -(3 * 3);\n"
            "j := /* will become false */ !true;\n"
            "k := ~%1111_1111;\n"
            "/* binary operators */\n"
            "a := 6 * (6 + 4);\n"
            "vec.x := 2.5;\n"
            "foo[2 * 2] := 66;\n"
            "2 == 2 && 3 == 1;\n"
            "2 == 2 || 3 == 3;\n"
            "// multiple assignment\n"
            "a, b, c := 3, 4, 2;\n"sv
            "/* compound assignments */\n"
            "x +:= 1;\n"sv
            "x -:= 1;\n"sv
            "x *:= 2;\n"sv
            "x /:= 2;\n"sv
            "x %:= 2;\n"sv
            "x |:= %0000_1111;\n"
            "x &:= %1111_0000;\n"
            "power := 2 ** 3 ** 4;\n"
            "chr := ptr^;\n"sv
            ;

        REQUIRE(buffer.load(r, source));

        entity_list_t tokens{};
        lexer::lexer_t lexer(session, buffer);
        REQUIRE(lexer.tokenize(r, tokens));
        REQUIRE(!r.is_failed());
        REQUIRE(r.messages().empty());

        parser::parser_t parser(session, buffer, tokens);
        REQUIRE(parser.initialize(r));

        entity_t module_node = parser.parse(r);
        REQUIRE(module_node != (entity_t)entt::null);

        ast::write_dot_graph(
            r,
            session,
            "test.dot",
            module_node);
    }

}
