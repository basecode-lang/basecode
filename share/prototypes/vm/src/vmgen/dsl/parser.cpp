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
#include <basecode/core/stack/stack.h>
#include <basecode/core/error/system.h>
#include <basecode/core/profiler/system.h>
#include <basecode/core/profiler/stopwatch.h>
#include "types.h"

#define YARD_PROCESS() do { \
        token = &lexer::current_token(lexer); \
        active_scope = stack::top(scope_stack); \
        rule = &s_operator_rules[(u32) token->type]; \
        precedence = &s_operator_precedences[(u32) token->type]; \
        goto *rule->handler; \
    } while (0)

#define YARD_NEXT() do { \
        lexer::next_token(lexer); \
        YARD_PROCESS(); \
    } while (0)

#define EVAL_PROCESS() do { \
        rpn = &cursor->scope->statements[cursor->stmt_idx]; \
        token = &rpn->tokens[cursor->token_idx]; \
        rule = &s_operator_rules[(u32) token->type]; \
        goto *rule->handler; \
    } while (0)

#define EVAL_NEXT() do { \
        ++cursor->token_idx; \
        EVAL_PROCESS(); \
    } while (0)

namespace basecode::dsl::parser {
    using namespace basecode::string;

    static string::slice_t s_numbers[] = {
        "0"_ss,
        "1"_ss,
        "2"_ss,
        "3"_ss,
        "4"_ss,
        "5"_ss,
        "6"_ss,
        "7"_ss,
        "8"_ss,
        "9"_ss,
        "10"_ss,
        "11"_ss,
        "12"_ss,
        "13"_ss,
        "14"_ss,
        "15"_ss,
        "16"_ss,
        "17"_ss,
        "18"_ss,
        "19"_ss,
        "20"_ss,
        "21"_ss,
        "22"_ss,
        "23"_ss,
        "24"_ss,
        "25"_ss,
        "26"_ss,
        "27"_ss,
        "28"_ss,
        "29"_ss,
        "30"_ss,
        "31"_ss,
        "32"_ss,
        "33"_ss,
        "34"_ss,
        "35"_ss,
        "36"_ss,
        "37"_ss,
        "38"_ss,
        "39"_ss,
        "40"_ss,
        "41"_ss,
        "42"_ss,
        "43"_ss,
        "44"_ss,
        "45"_ss,
        "46"_ss,
        "47"_ss,
        "48"_ss,
        "49"_ss,
        "50"_ss,
        "51"_ss,
        "52"_ss,
        "53"_ss,
        "54"_ss,
        "55"_ss,
        "56"_ss,
        "57"_ss,
        "58"_ss,
        "59"_ss,
        "60"_ss,
        "61"_ss,
        "62"_ss,
        "63"_ss,
        "64"_ss,
        "65"_ss,
        "66"_ss,
        "67"_ss,
        "68"_ss,
        "69"_ss,
        "70"_ss,
        "71"_ss,
        "72"_ss,
        "73"_ss,
        "74"_ss,
        "75"_ss,
        "76"_ss,
        "77"_ss,
        "78"_ss,
        "79"_ss,
        "80"_ss,
        "81"_ss,
        "82"_ss,
        "83"_ss,
        "84"_ss,
        "85"_ss,
        "86"_ss,
        "87"_ss,
        "88"_ss,
        "89"_ss,
        "90"_ss,
        "91"_ss,
        "92"_ss,
        "93"_ss,
        "94"_ss,
        "95"_ss,
        "96"_ss,
        "97"_ss,
        "98"_ss,
        "99"_ss,
        "100"_ss,
        "101"_ss,
        "102"_ss,
        "103"_ss,
        "104"_ss,
        "105"_ss,
        "106"_ss,
        "107"_ss,
        "108"_ss,
        "109"_ss,
        "110"_ss,
        "111"_ss,
        "112"_ss,
        "113"_ss,
        "114"_ss,
        "115"_ss,
        "116"_ss,
        "117"_ss,
        "118"_ss,
        "119"_ss,
        "120"_ss,
        "121"_ss,
        "122"_ss,
        "123"_ss,
        "124"_ss,
        "125"_ss,
        "126"_ss,
        "127"_ss,
        "128"_ss,
        "129"_ss,
        "130"_ss,
        "131"_ss,
        "132"_ss,
        "133"_ss,
        "134"_ss,
        "135"_ss,
        "136"_ss,
        "137"_ss,
        "138"_ss,
        "139"_ss,
        "140"_ss,
        "141"_ss,
        "142"_ss,
        "143"_ss,
        "144"_ss,
        "145"_ss,
        "146"_ss,
        "147"_ss,
        "148"_ss,
        "149"_ss,
        "150"_ss,
        "151"_ss,
        "152"_ss,
        "153"_ss,
        "154"_ss,
        "155"_ss,
        "156"_ss,
        "157"_ss,
        "158"_ss,
        "159"_ss,
        "160"_ss,
        "161"_ss,
        "162"_ss,
        "163"_ss,
        "164"_ss,
        "165"_ss,
        "166"_ss,
        "167"_ss,
        "168"_ss,
        "169"_ss,
        "170"_ss,
        "171"_ss,
        "172"_ss,
        "173"_ss,
        "174"_ss,
        "175"_ss,
        "176"_ss,
        "177"_ss,
        "178"_ss,
        "179"_ss,
        "180"_ss,
        "181"_ss,
        "182"_ss,
        "183"_ss,
        "184"_ss,
        "185"_ss,
        "186"_ss,
        "187"_ss,
        "188"_ss,
        "189"_ss,
        "190"_ss,
        "191"_ss,
        "192"_ss,
        "193"_ss,
        "194"_ss,
        "195"_ss,
        "196"_ss,
        "197"_ss,
        "198"_ss,
        "199"_ss,
        "200"_ss,
        "201"_ss,
        "202"_ss,
        "203"_ss,
        "204"_ss,
        "205"_ss,
        "206"_ss,
        "207"_ss,
        "208"_ss,
        "209"_ss,
        "210"_ss,
        "211"_ss,
        "212"_ss,
        "213"_ss,
        "214"_ss,
        "215"_ss,
        "216"_ss,
        "217"_ss,
        "218"_ss,
        "219"_ss,
        "220"_ss,
        "221"_ss,
        "222"_ss,
        "223"_ss,
        "224"_ss,
        "225"_ss,
        "226"_ss,
        "227"_ss,
        "228"_ss,
        "229"_ss,
        "230"_ss,
        "231"_ss,
        "232"_ss,
        "233"_ss,
        "234"_ss,
        "235"_ss,
        "236"_ss,
        "237"_ss,
        "238"_ss,
        "239"_ss,
        "240"_ss,
        "241"_ss,
        "242"_ss,
        "243"_ss,
        "244"_ss,
        "245"_ss,
        "246"_ss,
        "247"_ss,
        "248"_ss,
        "249"_ss,
        "250"_ss,
        "251"_ss,
        "252"_ss,
        "253"_ss,
        "254"_ss,
        "255"_ss,
    };

    static operator_precedence_t s_operator_precedences[] = {
        // non-operators
        [(u32) token_type_t::none]                   = {},
        [(u32) token_type_t::ident]                  = {},
        [(u32) token_type_t::comment]                = {},
        [(u32) token_type_t::str_lit]                = {},
        [(u32) token_type_t::hex_lit]                = {},
        [(u32) token_type_t::dec_lit]                = {},
        [(u32) token_type_t::bin_lit]                = {},
        [(u32) token_type_t::arg_count]              = {},
        [(u32) token_type_t::scope_end]              = {},
        [(u32) token_type_t::scope_begin]            = {},
        [(u32) token_type_t::end_of_program]         = {},

        // grammatical operators
        [(u32) token_type_t::comma]                  = {.op = true},
        [(u32) token_type_t::params_end]             = {.op = true},
        [(u32) token_type_t::params_begin]           = {.op = true},
        [(u32) token_type_t::addr_op_end]            = {.op = true},
        [(u32) token_type_t::addr_op_begin]          = {.op = true},
        [(u32) token_type_t::statement_terminator]   = {.op = true},

        // procedure call operator
        [(u32) token_type_t::call_op]                = {.op = true, .left = {-1, 17}},

        // member access operator
        [(u32) token_type_t::dot_op]                 = {.op = true, .left = {2, 16}},

        // XXX
        [(u32) token_type_t::imm_op]                 = {.op = true, .left = {1, 16}},

        // unary prefix/postfix increment/decrement operators
        [(u32) token_type_t::prefix_inc_op]          = {.op = true, .left = {1, 16}, .right = {1, 14}},
        [(u32) token_type_t::prefix_dec_op]          = {.op = true, .left = {1, 16}, .right = {1, 14}},
        [(u32) token_type_t::postfix_inc_op]         = {.op = true, .left = {1, 16}, .right = {1, 14}},
        [(u32) token_type_t::postfix_dec_op]         = {.op = true, .left = {1, 16}, .right = {1, 14}},

        // unary operators
        [(u32) token_type_t::neg_op]                 = {.op = true, .right = {1, 14}},
        [(u32) token_type_t::pos_op]                 = {.op = true, .right = {1, 14}},
        [(u32) token_type_t::lnot_op]                = {.op = true, .right = {1, 14}},
        [(u32) token_type_t::bnot_op]                = {.op = true, .right = {1, 14}},

        // binary arithmetic operators
        [(u32) token_type_t::div_op]                 = {.op = true, .left = {2, 15}},
        [(u32) token_type_t::mul_op]                 = {.op = true, .left = {2, 15}},
        [(u32) token_type_t::mod_op]                 = {.op = true, .left = {2, 15}},

        [(u32) token_type_t::sub_op]                 = {.op = true, .left = {2, 13}},
        [(u32) token_type_t::add_op]                 = {.op = true, .left = {2, 13}},

        [(u32) token_type_t::shl_op]                 = {.op = true, .left = {2, 12}},
        [(u32) token_type_t::shr_op]                 = {.op = true, .left = {2, 12}},
        [(u32) token_type_t::rol_op]                 = {.op = true, .left = {2, 12}},
        [(u32) token_type_t::ror_op]                 = {.op = true, .left = {2, 12}},

        // relational operators
        [(u32) token_type_t::lt_op]                  = {.op = true, .left = {2, 11}},
        [(u32) token_type_t::lte_op]                 = {.op = true, .left = {2, 11}},
        [(u32) token_type_t::gt_op]                  = {.op = true, .left = {2, 11}},
        [(u32) token_type_t::gte_op]                 = {.op = true, .left = {2, 11}},

        [(u32) token_type_t::eq_op]                  = {.op = true, .left = {2, 10}},
        [(u32) token_type_t::range_op]               = {.op = true, .left = {2, 10}},
        [(u32) token_type_t::not_eq_op]              = {.op = true, .left = {2, 10}},

        [(u32) token_type_t::band_op]                = {.op = true, .left = {2, 9}},
        [(u32) token_type_t::bxor_op]                = {.op = true, .left = {2, 8}},
        [(u32) token_type_t::bor_op]                 = {.op = true, .left = {2, 7}},

        [(u32) token_type_t::land_op]                = {.op = true, .left = {2, 6}},
        [(u32) token_type_t::lor_op]                 = {.op = true, .left = {2, 5}},

        // assignment operator
        [(u32) token_type_t::assignment_op]          = {.op = true, .right = {2, 4}},

        // signal operator
        [(u32) token_type_t::signal_op]              = {.op = true, .right = {2, 2}},
    };

    static b8 is_token_lower_precedence(
        stack::stack_t<token_t>&,
        const token_t&,
        const operator_precedence_t&);

    static b8 infix_to_rpn(parser_t&, scope_t&);

    static u0 format_scope(const scope_t&, u32);

    u0 free(parser_t& parser) {
        ast::free(parser.ast);
        intern::pool::free(parser.intern_pool);
    }

    b8 parse(parser_t& parser) {
        static operator_rule_t s_operator_rules[] = {
            // non-operators
            [(u32) token_type_t::none]                  = {.handler = &&_error},
            [(u32) token_type_t::ident]                 = {.handler = &&_push_ident},
            [(u32) token_type_t::comment]               = {.handler = &&_push_comment},
            [(u32) token_type_t::str_lit]               = {.handler = &&_push_str_lit},
            [(u32) token_type_t::hex_lit]               = {.handler = &&_push_hex_lit},
            [(u32) token_type_t::dec_lit]               = {.handler = &&_push_dec_lit},
            [(u32) token_type_t::bin_lit]               = {.handler = &&_push_bin_lit},
            [(u32) token_type_t::scope_end]             = {.handler = &&_pop_scope},
            [(u32) token_type_t::scope_begin]           = {.handler = &&_push_scope},

            // grammatical operators
            [(u32) token_type_t::comma]                 = {.handler = &&_error},
            [(u32) token_type_t::arg_count]             = {.handler = &&_arg_count},
            [(u32) token_type_t::params_end]            = {.handler = &&_error},
            [(u32) token_type_t::params_begin]          = {.handler = &&_error},
            [(u32) token_type_t::addr_op_end]           = {.handler = &&_error},
            [(u32) token_type_t::addr_op_begin]         = {.handler = &&_push_call_op},
            [(u32) token_type_t::end_of_program]        = {.handler = &&_end_of_program},
            [(u32) token_type_t::statement_terminator]  = {.handler = &&_statement_terminator},

            // procedure call operator
            [(u32) token_type_t::call_op]               = {.handler = &&_push_call_op},

            // member access operator
            [(u32) token_type_t::dot_op]                = {.handler = &&_push_call_op},

            // XXX
            [(u32) token_type_t::imm_op]                = {.handler = &&_push_unary_op, .type = {.unary = unary_op_type_t::imm}},

            // unary prefix/postfix increment/decrement operators
            [(u32) token_type_t::prefix_inc_op]         = {.handler = &&_push_unary_op, .type = {.unary = unary_op_type_t::pre_inc}},
            [(u32) token_type_t::prefix_dec_op]         = {.handler = &&_push_unary_op, .type = {.unary = unary_op_type_t::pre_dec}},
            [(u32) token_type_t::postfix_inc_op]        = {.handler = &&_push_unary_op, .type = {.unary = unary_op_type_t::post_inc}},
            [(u32) token_type_t::postfix_dec_op]        = {.handler = &&_push_unary_op, .type = {.unary = unary_op_type_t::post_dec}},

            // unary operators
            [(u32) token_type_t::neg_op]                = {.handler = &&_push_unary_op, .type = {.unary = unary_op_type_t::neg}},
            [(u32) token_type_t::pos_op]                = {.handler = &&_push_unary_op, .type = {.unary = unary_op_type_t::pos}},
            [(u32) token_type_t::lnot_op]               = {.handler = &&_push_unary_op, .type = {.unary = unary_op_type_t::lnot}},
            [(u32) token_type_t::bnot_op]               = {.handler = &&_push_unary_op, .type = {.unary = unary_op_type_t::bnot}},

            // binary arithmetic operators
            [(u32) token_type_t::div_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::div}},
            [(u32) token_type_t::mul_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::mul}},
            [(u32) token_type_t::mod_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::mod}},

            [(u32) token_type_t::sub_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::sub}},
            [(u32) token_type_t::add_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::add}},

            [(u32) token_type_t::shl_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::shl}},
            [(u32) token_type_t::shr_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::shr}},
            [(u32) token_type_t::rol_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::rol}},
            [(u32) token_type_t::ror_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::ror}},

            // relational operators
            [(u32) token_type_t::lt_op]                 = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::lt}},
            [(u32) token_type_t::lte_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::lte}},
            [(u32) token_type_t::gt_op]                 = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::gt}},
            [(u32) token_type_t::gte_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::gte}},

            [(u32) token_type_t::eq_op]                 = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::eq}},
            [(u32) token_type_t::range_op]              = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::range}},
            [(u32) token_type_t::not_eq_op]             = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::neq}},

            [(u32) token_type_t::band_op]               = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::band}},
            [(u32) token_type_t::bxor_op]               = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::bxor}},
            [(u32) token_type_t::bor_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::bor}},

            [(u32) token_type_t::land_op]               = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::land}},
            [(u32) token_type_t::lor_op]                = {.handler = &&_push_binary_op, .type = {.binary = binary_op_type_t::lor}},

            // assignment operator
            [(u32) token_type_t::assignment_op]         = {.handler = &&_push_assignment_op},

            // signal operator
            [(u32) token_type_t::signal_op]             = {.handler = &&_push_signal_op},
        };

        auto root_scope = make_scope();
        defer(free_scope(root_scope));

        if (!infix_to_rpn(parser, root_scope))
            return false;
        //format_scope(root_scope, 0);

        profiler::stopwatch_t watch{};
        profiler::start(watch);
        defer({
            profiler::stop(watch);
            profiler::print_elapsed_time("parser: rpn_eval"_ss, 40, profiler::elapsed(watch));
        });

        stack::stack_t<scope_cursor_t> scope_stack;
        stack::init(scope_stack, parser.allocator);
        stack::reserve(scope_stack, 64, false);

        stack::stack_t<u32> arg_count_stack;
        stack::init(arg_count_stack, parser.allocator);
        stack::reserve(arg_count_stack, 32, false);

        stack::stack_t<id_t> terminal_stack;
        stack::init(terminal_stack, parser.allocator);
        stack::reserve(terminal_stack, 32, false);

        stack::stack_t<node_cursor_t> ast_scope_stack;
        stack::init(ast_scope_stack, parser.allocator);
        stack::reserve(ast_scope_stack, 32, false);

        auto root_scope_cursor = ast::write_header(
            parser.ast,
            node_header_type_t::scope,
            root_scope.children.size + root_scope.statements.size);
        stack::push(ast_scope_stack, root_scope_cursor);

        auto root_cursor = stack::push(scope_stack);
        root_cursor->scope = &root_scope;
        root_cursor->stmt_idx = 0;
        root_cursor->token_idx = -1;
        defer(stack::pop(scope_stack));

        rpn_t* rpn{};
        token_t* token{};
        operator_rule_t* rule{};
        auto cursor = stack::top(scope_stack);

        EVAL_NEXT();

        _error: {
            // XXX
            error::print(stderr, *parser.lexer->buf, "parser error");
            return false;
        }

        _arg_count: {
            u32 count = atoi((const char*) token->slice.data);
            stack::push(arg_count_stack, count);
            EVAL_NEXT();
        }

        _push_ident: {
            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::ident,
                1);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _pop_scope: {
            auto ast_scope_cursor = stack::top(ast_scope_stack);
            while (!stack::empty(terminal_stack)) {
                auto top = stack::top(terminal_stack);
                ast::write_field(*ast_scope_cursor, node_field_type_t::child, *top);
                stack::pop(terminal_stack);
            }

            stack::pop(ast_scope_stack);
            stack::pop(scope_stack);
            cursor = stack::top(scope_stack);
            goto _statement_terminator;
        }

        _push_scope: {
            auto current_scope_cursor = stack::top(ast_scope_stack);
            auto ast_scope_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::scope,
                rpn->scope->children.size + rpn->scope->statements.size);
            ast::write_field(*current_scope_cursor, node_field_type_t::scope, ast_scope_cursor.id);
            ast::write_field(ast_scope_cursor, node_field_type_t::parent, current_scope_cursor->id);
            stack::push(ast_scope_stack, ast_scope_cursor);

            cursor = stack::push(scope_stack);
            cursor->scope = rpn->scope;
            cursor->token_idx = -1;
            cursor->stmt_idx = 0;
            EVAL_NEXT();
        }

        _push_str_lit: {
            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::str_lit,
                1);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _push_bin_lit: {
            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::num_lit,
                2);
            ast::write_field(ast_cursor, node_field_type_t::radix, 2);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _push_dec_lit: {
            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::num_lit,
                2);
            ast::write_field(ast_cursor, node_field_type_t::radix, 10);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _push_hex_lit: {
            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::num_lit,
                2);
            ast::write_field(ast_cursor, node_field_type_t::radix, 16);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _push_comment: {
            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::comment,
                1);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _push_call_op: {
            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::call,
                terminal_stack.size + 1);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            auto arg_count = *stack::top(arg_count_stack);
            stack::pop(arg_count_stack);
            while (arg_count > 0) {
                auto arg = *stack::top(terminal_stack);
                ast::write_field(ast_cursor, node_field_type_t::arg, arg);
                stack::pop(terminal_stack);
                --arg_count;
            }
            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _push_unary_op: {
            auto rhs_id = *stack::top(terminal_stack);
            stack::pop(terminal_stack);

            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::unary,
                3);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            ast::write_field(ast_cursor, node_field_type_t::rhs, rhs_id);
            ast::write_field(ast_cursor, node_field_type_t::flags, (u32) rule->type.unary);

            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _end_of_program: {
            // XXX:
            return true;
        }

        _push_binary_op: {
            auto rhs_id = *stack::top(terminal_stack);
            stack::pop(terminal_stack);

            auto lhs_id = *stack::top(terminal_stack);
            stack::pop(terminal_stack);

            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::binary,
                4);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            ast::write_field(ast_cursor, node_field_type_t::lhs, lhs_id);
            ast::write_field(ast_cursor, node_field_type_t::rhs, rhs_id);
            ast::write_field(ast_cursor, node_field_type_t::flags, (u32) rule->type.binary);

            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _push_signal_op: {
            auto rhs_id = *stack::top(terminal_stack);
            stack::pop(terminal_stack);

            auto lhs_id = *stack::top(terminal_stack);
            stack::pop(terminal_stack);

            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::signal,
                3);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            ast::write_field(ast_cursor, node_field_type_t::lhs, lhs_id);
            ast::write_field(ast_cursor, node_field_type_t::rhs, rhs_id);

            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _push_assignment_op: {
            auto rhs_id = *stack::top(terminal_stack);
            stack::pop(terminal_stack);

            auto lhs_id = *stack::top(terminal_stack);
            stack::pop(terminal_stack);

            auto ast_cursor = ast::write_header(
                parser.ast,
                node_header_type_t::assignment,
                3);
            ast::write_field(ast_cursor, node_field_type_t::token, token->id);
            ast::write_field(ast_cursor, node_field_type_t::lhs, lhs_id);
            ast::write_field(ast_cursor, node_field_type_t::rhs, rhs_id);

            stack::push(terminal_stack, ast_cursor.id);
            EVAL_NEXT();
        }

        _statement_terminator: {
            cursor->token_idx = -1;
            ++cursor->stmt_idx;
            EVAL_NEXT();
        }

        return true;
    }

    parser_t make(lexer_t* lexer, memory::allocator_t* allocator) {
        parser_t parser{};
        init(parser, lexer, allocator);
        return parser;
    }

    u0 init(parser_t& parser, lexer_t* lexer, memory::allocator_t* allocator) {
        parser.lexer = lexer;
        parser.allocator = allocator;
        ast::init(parser.ast, allocator);
        intern::pool::init(parser.intern_pool, allocator);
    }

    ///////////////////////////////////////////////////////////////////////

    static b8 is_token_lower_precedence(
            stack::stack_t<token_t>& operators,
            const token_t& token,
            const operator_precedence_t& precedence) {
        auto top_op = stack::top(operators);
        const auto& top_precedence = s_operator_precedences[(u32) top_op->type];
        return (precedence.left.precedence > 0 && precedence.left.precedence  <= top_precedence.left.precedence)
           || (precedence.right.precedence > 0 && precedence.right.precedence <  top_precedence.right.precedence);
    }

    static u0 format_indent(u32 indent) {
        if (indent == 0) return;
        fmt::print("{:<{}}", " ", indent);
    }

    static u0 format_newline(u32 indent) {
        fmt::print("\n");
        format_indent(indent);
    }

    static u0 append_arg_count_token(parser_t& parser, rpn_t& rpn) {
        auto& arg_count_token = array::append(rpn.tokens);
        arg_count_token.id = parser.lexer->tokens.size + 1;
        arg_count_token.type = token_type_t::arg_count;
        arg_count_token.slice = s_numbers[rpn.arg_count + 1];
    }

    static b8 infix_to_rpn(parser_t& parser, scope_t& root_scope) {
        static operator_rule_t s_operator_rules[] = {
            // non-operators
            [(u32) token_type_t::none]                  = {.handler = &&_error},
            [(u32) token_type_t::skip]                  = {.handler = &&_skip},
            [(u32) token_type_t::ident]                 = {.handler = &&_ident},
            [(u32) token_type_t::comment]               = {.handler = &&_push_comment},
            [(u32) token_type_t::str_lit]               = {.handler = &&_push_operand},
            [(u32) token_type_t::hex_lit]               = {.handler = &&_push_operand},
            [(u32) token_type_t::dec_lit]               = {.handler = &&_push_operand},
            [(u32) token_type_t::bin_lit]               = {.handler = &&_push_operand},
            [(u32) token_type_t::scope_end]             = {.handler = &&_pop_scope},
            [(u32) token_type_t::scope_begin]           = {.handler = &&_push_scope},

            // grammatical operators
            [(u32) token_type_t::comma]                 = {.handler = &&_comma},
            [(u32) token_type_t::params_end]            = {.handler = &&_params_end},
            [(u32) token_type_t::params_begin]          = {.handler = &&_params_begin},
            [(u32) token_type_t::addr_op_end]           = {.handler = &&_addr_op_end},
            [(u32) token_type_t::addr_op_begin]         = {.handler = &&_addr_op_begin},
            [(u32) token_type_t::end_of_program]        = {.handler = &&_end_of_program},
            [(u32) token_type_t::statement_terminator]  = {.handler = &&_statement_terminator},

            // procedure call operator
            [(u32) token_type_t::call_op]               = {.handler = &&_push_operator},

            // member access operator
            [(u32) token_type_t::dot_op]                = {.handler = &&_dot_operator},

            // XXX
            [(u32) token_type_t::imm_op]                = {.handler = &&_push_operator},

            // unary prefix/postfix increment/decrement operators
            [(u32) token_type_t::prefix_inc_op]         = {.handler = &&_push_operator},
            [(u32) token_type_t::prefix_dec_op]         = {.handler = &&_push_operator},
            [(u32) token_type_t::postfix_inc_op]        = {.handler = &&_push_operator},
            [(u32) token_type_t::postfix_dec_op]        = {.handler = &&_push_operator},

            // unary operators
            [(u32) token_type_t::neg_op]                = {.handler = &&_push_operator},
            [(u32) token_type_t::pos_op]                = {.handler = &&_push_operator},
            [(u32) token_type_t::lnot_op]               = {.handler = &&_push_operator},
            [(u32) token_type_t::bnot_op]               = {.handler = &&_push_operator},

            // binary arithmetic operators
            [(u32) token_type_t::div_op]                = {.handler = &&_push_operator},
            [(u32) token_type_t::mul_op]                = {.handler = &&_push_operator},
            [(u32) token_type_t::mod_op]                = {.handler = &&_push_operator},

            [(u32) token_type_t::sub_op]                = {.handler = &&_push_operator},
            [(u32) token_type_t::add_op]                = {.handler = &&_push_operator},

            [(u32) token_type_t::shl_op]                = {.handler = &&_push_operator},
            [(u32) token_type_t::shr_op]                = {.handler = &&_push_operator},
            [(u32) token_type_t::rol_op]                = {.handler = &&_push_operator},
            [(u32) token_type_t::ror_op]                = {.handler = &&_push_operator},

            // relational operators
            [(u32) token_type_t::lt_op]                 = {.handler = &&_push_operator},
            [(u32) token_type_t::lte_op]                = {.handler = &&_push_operator},
            [(u32) token_type_t::gt_op]                 = {.handler = &&_push_operator},
            [(u32) token_type_t::gte_op]                = {.handler = &&_push_operator},

            [(u32) token_type_t::eq_op]                 = {.handler = &&_push_operator},
            [(u32) token_type_t::range_op]              = {.handler = &&_push_operator},
            [(u32) token_type_t::not_eq_op]             = {.handler = &&_push_operator},

            [(u32) token_type_t::band_op]               = {.handler = &&_push_operator},
            [(u32) token_type_t::bxor_op]               = {.handler = &&_push_operator},
            [(u32) token_type_t::bor_op]                = {.handler = &&_push_operator},

            [(u32) token_type_t::land_op]               = {.handler = &&_push_operator},
            [(u32) token_type_t::lor_op]                = {.handler = &&_push_operator},

            // assignment operator
            [(u32) token_type_t::assignment_op]         = {.handler = &&_push_operator},

            // signal operator
            [(u32) token_type_t::signal_op]             = {.handler = &&_push_operator},
        };

        profiler::stopwatch_t watch{};
        profiler::start(watch);
        defer({
            profiler::stop(watch);
            profiler::print_elapsed_time("parser: infix_to_rpn"_ss, 40, profiler::elapsed(watch));
        });

        auto& lexer = *parser.lexer;
        stack::stack_t<scope_t*> scope_stack;
        stack::init(scope_stack, parser.allocator);
        stack::reserve(scope_stack, 64, false);

        stack::push(scope_stack, &root_scope);
        defer(stack::pop(scope_stack));

        scope_t* active_scope{};
        token_t* token{};
        operator_rule_t* rule{};
        operator_precedence_t* precedence{};

        auto scratch_rpn = make_rpn();
        auto operators = stack::make<token_t>();
        stack::reserve(operators, 32, false);

        YARD_PROCESS();

        _skip: {
            YARD_NEXT();
        }

        _error: {
            // XXX
            error::print(stderr, *parser.lexer->buf, "parser error");
            return false;
        }

        _ident: {
            if (lexer::peek_token(lexer, 1).type == token_type_t::params_begin) {
                token->type = token_type_t::call_op;
                stack::push(operators, *token);
            } else {
                array::append(scratch_rpn.tokens, *token);
            }
            YARD_NEXT();
        }

        _comma: {
            while (!stack::empty(operators)) {
                auto op_token = stack::top(operators);
                if (op_token->type == token_type_t::params_begin
                ||  op_token->type == token_type_t::addr_op_begin) {
                    break;
                }
                array::append(scratch_rpn.tokens, *op_token);
                stack::pop(operators);
            }
            scratch_rpn.arg_count++;
            YARD_NEXT();
        }

        _pop_scope: {
            array::append(scratch_rpn.tokens, *token);
            array::append(active_scope->statements, scratch_rpn);
            array::reset(scratch_rpn.tokens);
            stack::pop(scope_stack);
            YARD_NEXT();
        }

        _push_scope: {
            auto& scope = make_child_scope(*active_scope, lexer.allocator);
            stack::push(scope_stack, &scope);

            scratch_rpn.scope = &scope;
            array::append(scratch_rpn.tokens, *token);
            array::append(active_scope->statements, scratch_rpn);
            reset_rpn(scratch_rpn);
            YARD_NEXT();
        }

        _push_operand: {
            array::append(scratch_rpn.tokens, *token);
            YARD_NEXT();
        }

        _push_comment: {
            array::append(scratch_rpn.tokens, *token);

            auto& statement_terminator = array::append(scratch_rpn.tokens);
            statement_terminator.type = token_type_t::statement_terminator;

            array::append(active_scope->statements, scratch_rpn);
            array::reset(scratch_rpn.tokens);
            YARD_NEXT();
        }

        _dot_operator: {
            scratch_rpn.arg_count = 1;
            append_arg_count_token(parser, scratch_rpn);
            stack::push(operators, *token);
            YARD_NEXT();
        }

        _push_operator: {
            while (!stack::empty(operators)) {
                if (is_token_lower_precedence(operators, *token, *precedence)) {
                    auto op_token = stack::top(operators);
                    array::append(scratch_rpn.tokens, *op_token);
                    stack::pop(operators);
                    continue;
                }
                break;
            }
            stack::push(operators, *token);
            YARD_NEXT();
        }

        _end_of_program: {
            array::append(scratch_rpn.tokens, *token);
            array::append(active_scope->statements, scratch_rpn);
            return true;
        }

        _addr_op_end: {
            while (!stack::empty(operators)) {
                auto op_token = stack::top(operators);
                stack::pop(operators);
                if (op_token->type == token_type_t::addr_op_begin) {
                    append_arg_count_token(parser, scratch_rpn);
                    array::append(scratch_rpn.tokens, *op_token);
                    break;
                } else {
                    array::append(scratch_rpn.tokens, *op_token);
                }
            }
            YARD_NEXT();
        }

        _addr_op_begin: {
            scratch_rpn.arg_count = {};
            stack::push(operators, *token);
            YARD_NEXT();
        }

        _params_end: {
            while (!stack::empty(operators)) {
                auto op_token = stack::top(operators);
                if (op_token->type == token_type_t::params_begin)
                    break;
                array::append(scratch_rpn.tokens, *op_token);
                stack::pop(operators);
            }
            stack::pop(operators);
            if (stack::top(operators)->type == token_type_t::call_op) {
                append_arg_count_token(parser, scratch_rpn);
                auto op_token = stack::top(operators);
                array::append(scratch_rpn.tokens, *op_token);
                stack::pop(operators);
            }
            YARD_NEXT();
        }

        _params_begin: {
            if (lexer::peek_token(lexer, 1).type == token_type_t::params_end) {
                scratch_rpn.arg_count = -1;
            } else {
                scratch_rpn.arg_count = {};
            }
            stack::push(operators, *token);
            YARD_NEXT();
        }

        _statement_terminator: {
            while (!stack::empty(operators)) {
                auto op_token = stack::top(operators);
                array::append(scratch_rpn.tokens, *op_token);
                stack::pop(operators);
            }
            array::append(scratch_rpn.tokens, *token);
            array::append(active_scope->statements, scratch_rpn);
            array::reset(scratch_rpn.tokens);
            YARD_NEXT();
        }

        return true;
    }

    [[maybe_unused]] static u0 format_scope(const scope_t& scope, u32 indent) {
        format_indent(indent);
        for (const auto& s : scope.statements) {
            for (const auto& t : s.tokens) {
                switch (t.type) {
                    case token_type_t::comment: {
                        fmt::print("//{}", t.slice);
                        format_newline(indent);
                        break;
                    }
                    case token_type_t::pos_op: {
                        fmt::print("pos ");
                        break;
                    }
                    case token_type_t::neg_op: {
                        fmt::print("neg ");
                        break;
                    }
                    case token_type_t::scope_end: {
                        fmt::print("{}", t.slice);
                        break;
                    }
                    case token_type_t::scope_begin: {
                        fmt::print("{}\n", t.slice);
                        format_scope(*s.scope, indent + 4);
                        format_newline(indent);
                        break;
                    }
                    case token_type_t::prefix_inc_op:
                    case token_type_t::prefix_dec_op: {
                        fmt::print("prefix:{} ", t.slice);
                        break;
                    }
                    case token_type_t::postfix_inc_op:
                    case token_type_t::postfix_dec_op: {
                        fmt::print("postfix:{} ", t.slice);
                        break;
                    }
                    case token_type_t::end_of_program: {
                        fmt::print("***\n");
                        break;
                    }
                    case token_type_t::statement_terminator: {
                        if (t.slice.length > 0) {
                            fmt::print("{}", t.slice);
                            format_newline(indent);
                        }
                        break;
                    }
                    default: {
                        fmt::print("{} ", t.slice);
                        break;
                    }
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    u0 free_rpn(rpn_t& expr) {
        array::free(expr.tokens);
    }

    u0 reset_rpn(rpn_t& expr) {
        expr.scope = {};
        array::reset(expr.tokens);
    }

    u0 free_scope(scope_t& scope) {
        for (auto& s : scope.statements)
            free_rpn(s);
        array::free(scope.statements);
        for (auto& s : scope.children)
            free_scope(s);
        array::free(scope.children);
    }

    rpn_t make_rpn(memory::allocator_t* allocator) {
        rpn_t expr;
        init_rpn(expr, allocator);
        return expr;
    }

    scope_t make_scope(memory::allocator_t* allocator) {
        scope_t scope;
        init_scope(scope, allocator);
        return scope;
    }

    u0 init_rpn(rpn_t& expr, memory::allocator_t* allocator) {
        expr.scope = {};
        expr.arg_count = {};
        array::init(expr.tokens, allocator);
    }

    u0 init_scope(scope_t& scope, memory::allocator_t* allocator) {
        array::init(scope.children, allocator);
        array::reserve(scope.children, 32, false);
        array::init(scope.statements, allocator);
    }

    scope_t& make_child_scope(scope_t& scope, memory::allocator_t* allocator) {
        auto& child_scope = scope.children[scope.children.size++];
        init_scope(child_scope, allocator);
        return child_scope;
    }
}