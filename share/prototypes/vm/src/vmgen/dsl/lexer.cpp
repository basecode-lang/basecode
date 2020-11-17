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
#include <basecode/core/error/system.h>
#include <basecode/core/profiler/system.h>
#include <basecode/core/profiler/stopwatch.h>
#include <basecode/core/string/ascii_string.h>
#include "types.h"

#define PROCESS() do { \
        ch = CURSOR(buf); \
        char_type = s_char_type_map[(u32) ch]; \
        rule = &s_token_rules[(u32) char_type]; \
        goto *rule->tokenizer; \
    } while (0)

#define NEXT() do { \
        source::buffer::next_char(buf); \
        PROCESS(); \
    } while (0)

namespace basecode::dsl::lexer {
    using namespace basecode::string;

    static string::slice_t s_token_type_names[] = {
        [(u32)  token_type_t::none]                 =  "none"_ss,
        [(u32)  token_type_t::skip]                 =  "skip"_ss,
        [(u32)  token_type_t::ident]                =  "ident"_ss,
        [(u32)  token_type_t::comma]                =  "comma"_ss,
        [(u32)  token_type_t::eq_op]                =  "eq_op"_ss,
        [(u32)  token_type_t::lt_op]                =  "lt_op"_ss,
        [(u32)  token_type_t::gt_op]                =  "gt_op"_ss,
        [(u32)  token_type_t::pos_op]               =  "pos_op"_ss,
        [(u32)  token_type_t::neg_op]               =  "neg_op"_ss,
        [(u32)  token_type_t::lte_op]               =  "lte_op"_ss,
        [(u32)  token_type_t::gte_op]               =  "gte_op"_ss,
        [(u32)  token_type_t::imm_op]               =  "imm_op"_ss,
        [(u32)  token_type_t::add_op]               =  "add_op"_ss,
        [(u32)  token_type_t::sub_op]               =  "sub_op"_ss,
        [(u32)  token_type_t::mul_op]               =  "mul_op"_ss,
        [(u32)  token_type_t::div_op]               =  "div_op"_ss,
        [(u32)  token_type_t::mod_op]               =  "mod_op"_ss,
        [(u32)  token_type_t::bor_op]               =  "bor_op"_ss,
        [(u32)  token_type_t::lor_op]               =  "lor_op"_ss,
        [(u32)  token_type_t::dot_op]               =  "dot_op"_ss,
        [(u32)  token_type_t::shl_op]               =  "shl_op"_ss,
        [(u32)  token_type_t::shr_op]               =  "shr_op"_ss,
        [(u32)  token_type_t::rol_op]               =  "rol_op"_ss,
        [(u32)  token_type_t::ror_op]               =  "ror_op"_ss,
        [(u32)  token_type_t::call_op]              =  "call_op"_ss,
        [(u32)  token_type_t::comment]              =  "comment"_ss,
        [(u32)  token_type_t::str_lit]              =  "str_lit"_ss,
        [(u32)  token_type_t::hex_lit]              =  "hex_lit"_ss,
        [(u32)  token_type_t::dec_lit]              =  "dec_lit"_ss,
        [(u32)  token_type_t::bin_lit]              =  "bin_lit"_ss,
        [(u32)  token_type_t::lnot_op]              =  "lnot_op"_ss,
        [(u32)  token_type_t::bnot_op]              =  "bnot_op"_ss,
        [(u32)  token_type_t::bxor_op]              =  "bxor_op"_ss,
        [(u32)  token_type_t::band_op]              =  "band_op"_ss,
        [(u32)  token_type_t::land_op]              =  "land_op"_ss,
        [(u32)  token_type_t::range_op]             =  "range_op"_ss,
        [(u32)  token_type_t::arg_count]            =  "arg_count"_ss,
        [(u32)  token_type_t::signal_op]            =  "signal_op"_ss,
        [(u32)  token_type_t::scope_end]            =  "scope_end"_ss,
        [(u32)  token_type_t::not_eq_op]            =  "not_eq_op"_ss,
        [(u32)  token_type_t::params_end]           =  "params_end"_ss,
        [(u32)  token_type_t::scope_begin]          =  "scope_begin"_ss,
        [(u32)  token_type_t::addr_op_end]          =  "addr_op_end"_ss,
        [(u32)  token_type_t::params_begin]         =  "params_begin"_ss,
        [(u32)  token_type_t::addr_op_begin]        =  "add_op_begin"_ss,
        [(u32)  token_type_t::assignment_op]        =  "assignment_op"_ss,
        [(u32)  token_type_t::prefix_dec_op]        =  "prefix_dec_op"_ss,
        [(u32)  token_type_t::prefix_inc_op]        =  "prefix_inc_op"_ss,
        [(u32)  token_type_t::postfix_dec_op]       =  "postfix_dec_op"_ss,
        [(u32)  token_type_t::postfix_inc_op]       =  "postfix_inc_op"_ss,
        [(u32)  token_type_t::end_of_program]       =  "end_of_program"_ss,
        [(u32)  token_type_t::statement_terminator] =  "statement_terminator"_ss,
    };

    static string::slice_t s_char_type_names[] = {
        [(u32)  character_type_t::none]             = "none"_ss,
        [(u32)  character_type_t::ws]               = "ws"_ss,
        [(u32)  character_type_t::at]               = "at"_ss,
        [(u32)  character_type_t::eof]              = "eof"_ss,
        [(u32)  character_type_t::plus]             = "plus"_ss,
        [(u32)  character_type_t::hash]             = "hash"_ss,
        [(u32)  character_type_t::bang]             = "bang"_ss,
        [(u32)  character_type_t::pipe]             = "pipe"_ss,
        [(u32)  character_type_t::text]             = "text"_ss,
        [(u32)  character_type_t::colon]            = "colon"_ss,
        [(u32)  character_type_t::caret]            = "caret"_ss,
        [(u32)  character_type_t::minus]            = "minus"_ss,
        [(u32)  character_type_t::tilde]            = "tilde"_ss,
        [(u32)  character_type_t::comma]            = "comma"_ss,
        [(u32)  character_type_t::period]           = "period"_ss,
        [(u32)  character_type_t::equals]           = "equals"_ss,
        [(u32)  character_type_t::dollar]           = "dollar"_ss,
        [(u32)  character_type_t::digits]           = "digits"_ss,
        [(u32)  character_type_t::percent]          = "percent"_ss,
        [(u32)  character_type_t::newline]          = "newline"_ss,
        [(u32)  character_type_t::asterisk]         = "asterisk"_ss,
        [(u32)  character_type_t::question]         = "question"_ss,
        [(u32)  character_type_t::ampersand]        = "ampersand"_ss,
        [(u32)  character_type_t::semicolon]        = "semicolon"_ss,
        [(u32)  character_type_t::less_than]        = "less_than"_ss,
        [(u32)  character_type_t::left_paren]       = "left_paren"_ss,
        [(u32)  character_type_t::left_brace]       = "left_brace"_ss,
        [(u32)  character_type_t::back_slash]       = "back_slash"_ss,
        [(u32)  character_type_t::underscore]       = "underscore"_ss,
        [(u32)  character_type_t::right_brace]      = "right_brace"_ss,
        [(u32)  character_type_t::right_paren]      = "right_paren"_ss,
        [(u32)  character_type_t::left_bracket]     = "left_bracket"_ss,
        [(u32)  character_type_t::greater_than]     = "greater_than"_ss,
        [(u32)  character_type_t::single_quote]     = "single_quote"_ss,
        [(u32)  character_type_t::double_quote]     = "double_quote"_ss,
        [(u32)  character_type_t::accent_grave]     = "accent_grave"_ss,
        [(u32)  character_type_t::right_bracket]    = "right_bracket"_ss,
        [(u32)  character_type_t::forward_slash]    = "forward_slash"_ss,
    };

    static character_type_t s_char_type_map[256] = {
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::newline,       character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::bang,         character_type_t::double_quote,  character_type_t::hash,
        character_type_t::dollar,        character_type_t::percent,      character_type_t::ampersand,     character_type_t::single_quote,
        character_type_t::left_paren,    character_type_t::right_paren,  character_type_t::asterisk,      character_type_t::plus,
        character_type_t::comma,         character_type_t::minus,        character_type_t::period,        character_type_t::forward_slash,
        character_type_t::digits,        character_type_t::digits,       character_type_t::digits,        character_type_t::digits,
        character_type_t::digits,        character_type_t::digits,       character_type_t::digits,        character_type_t::digits,
        character_type_t::digits,        character_type_t::digits,       character_type_t::colon,         character_type_t::semicolon,
        character_type_t::less_than,     character_type_t::equals,       character_type_t::greater_than,  character_type_t::question,
        character_type_t::at,            character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::left_bracket,
        character_type_t::back_slash,    character_type_t::right_bracket,character_type_t::caret,         character_type_t::underscore,
        character_type_t::accent_grave,  character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::text,
        character_type_t::text,          character_type_t::text,         character_type_t::text,          character_type_t::left_brace,
        character_type_t::pipe,          character_type_t::right_brace,  character_type_t::tilde,         character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::ws,
        character_type_t::ws,            character_type_t::ws,           character_type_t::ws,            character_type_t::eof
    };

    static nary_rule_t s_nary_rules[] = {
        [(u32)  token_type_t::none]                 =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::skip]                 =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::ident]                =  {.plus = token_type_t::add_op, .minus = token_type_t::sub_op},
        [(u32)  token_type_t::comma]                =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::eq_op]                =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::lt_op]                =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::gt_op]                =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::pos_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::neg_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::lte_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::gte_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::imm_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::add_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::sub_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::mul_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::div_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::mod_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::bor_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::lor_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::dot_op]               =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::shl_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::shr_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::rol_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::ror_op]               =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::call_op]              =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::comment]              =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::str_lit]              =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::hex_lit]              =  {.plus = token_type_t::add_op, .minus = token_type_t::sub_op},
        [(u32)  token_type_t::dec_lit]              =  {.plus = token_type_t::add_op, .minus = token_type_t::sub_op},
        [(u32)  token_type_t::bin_lit]              =  {.plus = token_type_t::add_op, .minus = token_type_t::sub_op},
        [(u32)  token_type_t::lnot_op]              =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::bnot_op]              =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::bxor_op]              =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::band_op]              =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::land_op]              =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::range_op]             =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::arg_count]            =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::signal_op]            =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::scope_end]            =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::not_eq_op]            =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::params_end]           =  {.plus = token_type_t::add_op, .minus = token_type_t::sub_op},
        [(u32)  token_type_t::scope_begin]          =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::addr_op_end]          =  {.plus = token_type_t::add_op, .minus = token_type_t::sub_op},
        [(u32)  token_type_t::params_begin]         =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::addr_op_begin]        =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::assignment_op]        =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::prefix_dec_op]        =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::prefix_inc_op]        =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::postfix_dec_op]       =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::postfix_inc_op]       =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
        [(u32)  token_type_t::end_of_program]       =  {.plus = token_type_t::none,   .minus = token_type_t::none},
        [(u32)  token_type_t::statement_terminator] =  {.plus = token_type_t::pos_op, .minus = token_type_t::neg_op},
    };

    u0 free(lexer_t& lexer) {
        array::free(lexer.tokens);
    }

    b8 tokenize(lexer_t& lexer) {
        static token_rule_t s_token_rules[] = {
            [(u32) character_type_t::none]              = {.tokenizer = &&unexpected_character, .type = token_type_t::none},
            [(u32) character_type_t::ws]                = {.tokenizer = &&_next,                .type = token_type_t::none},
            [(u32) character_type_t::at]                = {.tokenizer = &&unsupported,          .type = token_type_t::none},
            [(u32) character_type_t::eof]               = {.tokenizer = &&_eof,                 .type = token_type_t::none},
            [(u32) character_type_t::plus]              = {.tokenizer = &&_plus_op,             .type = token_type_t::none},
            [(u32) character_type_t::hash]              = {.tokenizer = &&_make_token,          .type = token_type_t::imm_op},
            [(u32) character_type_t::bang]              = {.tokenizer = &&_bang_op,             .type = token_type_t::lnot_op},
            [(u32) character_type_t::pipe]              = {.tokenizer = &&_or_op,               .type = token_type_t::bor_op},
            [(u32) character_type_t::text]              = {.tokenizer = &&_ident,               .type = token_type_t::ident},
            [(u32) character_type_t::colon]             = {.tokenizer = &&unsupported,          .type = token_type_t::none},
            [(u32) character_type_t::caret]             = {.tokenizer = &&_make_token,          .type = token_type_t::bxor_op},
            [(u32) character_type_t::minus]             = {.tokenizer = &&_minus_op,            .type = token_type_t::none},
            [(u32) character_type_t::tilde]             = {.tokenizer = &&_make_token,          .type = token_type_t::bnot_op},
            [(u32) character_type_t::comma]             = {.tokenizer = &&_make_token,          .type = token_type_t::comma},
            [(u32) character_type_t::period]            = {.tokenizer = &&_dot_op,              .type = token_type_t::dot_op},
            [(u32) character_type_t::equals]            = {.tokenizer = &&_eq_op,               .type = token_type_t::assignment_op},
            [(u32) character_type_t::dollar]            = {.tokenizer = &&_hex_lit,             .type = token_type_t::hex_lit},
            [(u32) character_type_t::digits]            = {.tokenizer = &&_dec_lit,             .type = token_type_t::dec_lit},
            [(u32) character_type_t::percent]           = {.tokenizer = &&_bin_lit,             .type = token_type_t::bin_lit},
            [(u32) character_type_t::newline]           = {.tokenizer = &&_newline,             .type = token_type_t::none},
            [(u32) character_type_t::asterisk]          = {.tokenizer = &&_make_token,          .type = token_type_t::mul_op},
            [(u32) character_type_t::question]          = {.tokenizer = &&unsupported,          .type = token_type_t::none},
            [(u32) character_type_t::ampersand]         = {.tokenizer = &&_and_op,              .type = token_type_t::band_op},
            [(u32) character_type_t::semicolon]         = {.tokenizer = &&_make_token,          .type = token_type_t::statement_terminator},
            [(u32) character_type_t::less_than]         = {.tokenizer = &&_lt_op,               .type = token_type_t::lt_op},
            [(u32) character_type_t::left_paren]        = {.tokenizer = &&_make_token,          .type = token_type_t::params_begin},
            [(u32) character_type_t::left_brace]        = {.tokenizer = &&_make_token,          .type = token_type_t::scope_begin},
            [(u32) character_type_t::back_slash]        = {.tokenizer = &&unsupported,          .type = token_type_t::none},
            [(u32) character_type_t::underscore]        = {.tokenizer = &&_ident,               .type = token_type_t::ident},
            [(u32) character_type_t::right_brace]       = {.tokenizer = &&_make_token,          .type = token_type_t::scope_end},
            [(u32) character_type_t::right_paren]       = {.tokenizer = &&_make_token,          .type = token_type_t::params_end},
            [(u32) character_type_t::left_bracket]      = {.tokenizer = &&_make_token,          .type = token_type_t::addr_op_begin},
            [(u32) character_type_t::greater_than]      = {.tokenizer = &&_gt_op,               .type = token_type_t::gt_op},
            [(u32) character_type_t::single_quote]      = {.tokenizer = &&_string,              .type = token_type_t::str_lit},
            [(u32) character_type_t::double_quote]      = {.tokenizer = &&unsupported,          .type = token_type_t::none},
            [(u32) character_type_t::accent_grave]      = {.tokenizer = &&unsupported,          .type = token_type_t::none},
            [(u32) character_type_t::right_bracket]     = {.tokenizer = &&_make_token,          .type = token_type_t::addr_op_end},
            [(u32) character_type_t::forward_slash]     = {.tokenizer = &&_forward_slash,       .type = token_type_t::div_op},
        };

        profiler::stopwatch_t watch{};
        profiler::start(watch);
        defer({
            profiler::stop(watch);
            profiler::print_elapsed_time("lexer"_ss, 40, profiler::elapsed(watch));
        });

        auto& first_token = array::append(lexer.tokens);
        first_token.id = lexer.tokens.size;
        first_token.type = token_type_t::skip;

        auto& buf = *lexer.buf;

        u8 ch{};
        token_rule_t* rule{};
        character_type_t char_type{};

        PROCESS();

        _eof: {
            auto& end_of_program = array::append(lexer.tokens);
            end_of_program.type = token_type_t::end_of_program;
            return true;
        }

        _next: {
            NEXT();
        }

        _newline: {
            source::buffer::next_line(buf);
            NEXT();
        }

        _forward_slash: {
            if (PEEK(buf, 1) != '/')
                goto _make_token;
            source::buffer::next_char(buf);
            source::buffer::next_char(buf);
            const auto start_pos = buf.idx;
            while (CURSOR(buf) != '\n')
                source::buffer::next_char(buf);
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = token_type_t::comment;
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos);
            NEXT();
        }

        _make_token: {
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = rule->type;
            token.slice = string::make_slice(
                buf.data + buf.idx,
                1);
            NEXT();
        }

        _hex_lit: {
            source::buffer::next_char(buf);
            const auto start_pos = buf.idx;
            while (isxdigit(CURSOR(buf)) || CURSOR(buf) == '_')
                source::buffer::next_char(buf);
            const auto end_pos = buf.idx;
            source::buffer::prev_char(buf);
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = token_type_t::hex_lit;
            token.slice = string::make_slice(
                buf.data + start_pos,
                end_pos - start_pos);
            NEXT();
        }

        _bin_lit: {
            source::buffer::next_char(buf);
            const auto start_pos = buf.idx;
            while (CURSOR(buf) == '0' || CURSOR(buf) == '1' || CURSOR(buf) == '_')
                source::buffer::next_char(buf);
            const auto end_pos = buf.idx;
            source::buffer::prev_char(buf);
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = token_type_t::bin_lit;
            token.slice = string::make_slice(
                buf.data + start_pos,
                end_pos - start_pos);
            NEXT();
        }

        _dec_lit: {
            const auto start_pos = buf.idx;
            while (isdigit(CURSOR(buf)) || CURSOR(buf) == '_')
                source::buffer::next_char(buf);
            const auto end_pos = buf.idx;
            source::buffer::prev_char(buf);
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = token_type_t::dec_lit;
            token.slice = string::make_slice(
                buf.data + start_pos,
                end_pos - start_pos);
            NEXT();
        }

        _string: {
            source::buffer::next_char(buf);
            const auto start_pos = buf.idx;
            while (CURSOR(buf) != '\'')
                source::buffer::next_char(buf);
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = token_type_t::str_lit;
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos);
            NEXT();
        }

        _ident: {
            const auto start_pos = buf.idx;
            while (isalnum(CURSOR(buf)) || CURSOR(buf) == '_')
                source::buffer::next_char(buf);
            source::buffer::prev_char(buf);
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = token_type_t::ident;
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos + 1);
            NEXT();
        }

        _eq_op: {
            const auto start_pos = buf.idx;
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = PEEK(buf, 1) == '=' ? token_type_t::eq_op : token_type_t::assignment_op;
            if (token.type == token_type_t::eq_op)
                source::buffer::next_char(buf);
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos + 1);
            NEXT();
        }

        _gt_op: {
            const auto start_pos = buf.idx;
            token_type_t token_type{};
            switch (PEEK(buf, 1)) {
                case '=': {
                    token_type = token_type_t::gte_op;
                    source::buffer::next_char(buf);
                    break;
                }
                case '>': {
                    source::buffer::next_char(buf);
                    token_type = PEEK(buf, 1) == '>' ? token_type_t::ror_op : token_type_t::shr_op;
                    if (token_type == token_type_t::ror_op)
                        source::buffer::next_char(buf);
                    break;
                }
                default: {
                    token_type = token_type_t::gt_op;
                    break;
                }
            }
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = token_type;
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos + 1);
            NEXT();
        }

        _lt_op: {
            const auto start_pos = buf.idx;
            token_type_t token_type{};
            switch (PEEK(buf, 1)) {
                case '-': {
                    source::buffer::next_char(buf);
                    token_type = token_type_t::signal_op;
                    break;
                }
                case '<': {
                    source::buffer::next_char(buf);
                    token_type = PEEK(buf, 1) == '<' ? token_type_t::rol_op : token_type_t::shl_op;
                    if (token_type == token_type_t::rol_op)
                        source::buffer::next_char(buf);
                    break;
                }
                case '=': {
                    source::buffer::next_char(buf);
                    token_type = token_type_t::lte_op;
                    break;
                }
                default: {
                    token_type = token_type_t::lt_op;
                    break;
                }
            }
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = token_type;
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos + 1);
            NEXT();
        }

        _or_op: {
            const auto start_pos = buf.idx;
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = PEEK(buf, 1) == '|' ? token_type_t::lor_op : token_type_t::bor_op  ;
            if (token.type == token_type_t::lor_op)
                source::buffer::next_char(buf);
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos + 1);
            NEXT();
        }

        _and_op: {
            const auto start_pos = buf.idx;
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = PEEK(buf, 1) == '&' ? token_type_t::land_op : token_type_t::band_op;
            if (token.type == token_type_t::land_op)
                source::buffer::next_char(buf);
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos + 1);
            NEXT();
        }

        _bang_op: {
            const auto start_pos = buf.idx;
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = PEEK(buf, 1) == '=' ? token_type_t::not_eq_op : token_type_t::lnot_op;
            if (token.type == token_type_t::not_eq_op)
                source::buffer::next_char(buf);
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos + 1);
            NEXT();
        }

        _plus_op: {
            const auto start_pos = buf.idx;
            const auto& prev_token = lexer.tokens[lexer.tokens.size - 1];
            token_type_t type;
            if (PEEK(buf, 1) == '+') {
                auto is_postfix = prev_token.type == token_type_t::ident;
                type = is_postfix ? token_type_t::postfix_inc_op : token_type_t::prefix_inc_op;
                source::buffer::next_char(buf);
            } else {
                type = s_nary_rules[(u32) prev_token.type].plus;
            }
            auto& token = array::append(lexer.tokens);
            token.type = type;
            token.id = lexer.tokens.size;
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos + 1);
            NEXT();
        }

        _minus_op: {
            const auto start_pos = buf.idx;
            const auto& prev_token = lexer.tokens[lexer.tokens.size - 1];
            token_type_t type;
            if (PEEK(buf, 1) == '-') {
                auto is_postfix = prev_token.type == token_type_t::ident;
                type = is_postfix ? token_type_t::postfix_dec_op : token_type_t::prefix_dec_op;
                source::buffer::next_char(buf);
            } else {
                type = s_nary_rules[(u32) prev_token.type].minus;
            }
            auto& token = array::append(lexer.tokens);
            token.type = type;
            token.id = lexer.tokens.size;
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos + 1);
            NEXT();
        }

        _dot_op: {
            const auto start_pos = buf.idx;
            auto& token = array::append(lexer.tokens);
            token.id = lexer.tokens.size;
            token.type = PEEK(buf, 1) == '.' ? token_type_t::range_op : token_type_t::dot_op;
            if (token.type == token_type_t::range_op)
                source::buffer::next_char(buf);
            token.slice = string::make_slice(
                buf.data + start_pos,
                buf.idx - start_pos + 1);
            NEXT();
        }

        unexpected_character:
            error::print(stderr, buf, "unexpected character: {}", ch);
            return false;

        unsupported:
            error::print(
                stderr,
                buf,
                "unsupported token mapping: {}",
                character_type_name(char_type));
            return false;
    }

    u0 seek_start(lexer_t& lexer) {
        lexer.token_idx = 1;
    }

    u0 next_token(lexer_t& lexer) {
        lexer.token_idx++;
    }

    b8 has_more_tokens(lexer_t& lexer) {
        return lexer.token_idx < lexer.tokens.size;
    }

    token_t& current_token(lexer_t& lexer) {
        return lexer.tokens[lexer.token_idx];
    }

    const token_t& get_token(lexer_t& lexer, id_t id) {
        return lexer.tokens[id - 1];
    }

    string::slice_t token_type_name(token_type_t type) {
        return s_token_type_names[(u32) type];
    }

    const token_t& peek_token(lexer_t& lexer, u32 count) {
        return lexer.tokens[lexer.token_idx + count];
    }

    string::slice_t character_type_name(character_type_t type) {
        return s_char_type_names[(u32) type];
    }

    lexer_t make(source::buffer_t* buf, memory::allocator_t* allocator) {
        lexer_t lexer{};
        init(lexer, buf, allocator);
        return lexer;
    }

    u0 init(lexer_t& lexer, source::buffer_t* buf, memory::allocator_t* allocator) {
        lexer.buf = buf;
        lexer.token_idx = 1;
        lexer.allocator = allocator;
        array::init(lexer.tokens, allocator);
    }
}