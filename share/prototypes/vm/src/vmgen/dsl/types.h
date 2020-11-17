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

#pragma once

#include <basecode/core/types.h>
#include <basecode/core/array/array.h>
#include <basecode/core/string/intern.h>
#include <basecode/core/source/buffer.h>

namespace basecode::dsl {
    struct rpn_t;
    struct scope_t;
    struct lexer_t;
    struct parser_t;
    struct storage_t;

    enum class node_kind_t : u8 {
        none        = 0b00000000,
        field       = 0b00000001,
        header      = 0b00000010,
    };

    enum class node_field_type_t : u8 {
        none        = 0b00000000,
        id          = 0b00000001,
        lhs         = 0b00000010,
        rhs         = 0b00000011,
        arg         = 0b00001001,
        flags       = 0b00000100,
        token       = 0b00000101,
        scope       = 0b00000110,
        radix       = 0b00000111,
        parent      = 0b00001000,
        child       = 0b00001010,
    };

    enum class node_header_type_t : u8 {
        none        = 0b00000000,
        call        = 0b00000001,
        read        = 0b00000010,
        write       = 0b00000011,
        ident       = 0b00000100,
        unary       = 0b00000101,
        scope       = 0b00000110,
        binary      = 0b00000111,
        signal      = 0b00001000,
        comment     = 0b00001001,
        str_lit     = 0b00001010,
        num_lit     = 0b00001011,
        assignment  = 0b00001100,
    };

    enum class value_type_t : u8 {
        reg,
        flag,
    };

    enum class unary_op_type_t : u8 {
        imm,
        neg,
        pos,
        lnot,
        bnot,
        pre_inc,
        pre_dec,
        post_inc,
        post_dec,
    };

    enum class binary_op_type_t : u8 {
        lt,
        gt,
        eq,
        lte,
        gte,
        neq,
        lor,
        shl,
        shr,
        rol,
        ror,
        add,
        sub,
        mul,
        div,
        mod,
        bor,
        dot,
        band,
        bxor,
        land,
        range,
    };

    enum class character_type_t : u8 {
        none,
        ws,
        at,
        eof,
        plus,
        hash,
        bang,
        pipe,
        text,
        colon,
        caret,
        minus,
        tilde,
        comma,
        period,
        equals,
        dollar,
        digits,
        percent,
        newline,
        asterisk,
        question,
        ampersand,
        semicolon,
        less_than,
        left_paren,
        left_brace,
        back_slash,
        underscore,
        right_brace,
        right_paren,
        left_bracket,
        greater_than,
        single_quote,
        double_quote,
        accent_grave,
        right_bracket,
        forward_slash,
    };

    enum class token_type_t : u8 {
        none,
        skip,
        ident,
        comma,
        lt_op,
        gt_op,
        eq_op,
        pos_op,
        neg_op,
        shl_op,
        shr_op,
        rol_op,
        ror_op,
        lte_op,
        gte_op,
        imm_op,
        add_op,
        sub_op,
        mul_op,
        div_op,
        mod_op,
        bor_op,
        lor_op,
        dot_op,
        call_op,
        comment,
        str_lit,
        hex_lit,
        dec_lit,
        bin_lit,
        lnot_op,
        bnot_op,
        bxor_op,
        band_op,
        land_op,
        range_op,
        not_eq_op,
        signal_op,
        scope_end,
        arg_count,
        params_end,
        scope_begin,
        addr_op_end,
        params_begin,
        addr_op_begin,
        assignment_op,
        prefix_dec_op,
        prefix_inc_op,
        postfix_dec_op,
        postfix_inc_op,
        end_of_program,
        statement_terminator,
    };

    struct node_field_t final {
        u32                     kind:3;
        u32                     type:5;
        u32                     value:24;
    };

    struct node_header_t final {
        u32                     kind:3;
        u32                     type:5;
        u32                     size:24;
    };

    struct node_index_t final {
        u8*                     page;
        u16                     offset;
    };

    using node_index_array_t = array::array_t<node_index_t>;

    struct page_header_t final {
        u0*                     prev;
        u0*                     next;
    };

    struct storage_t final {
        u8*                     tail;
        u8*                     head;
        memory::allocator_t*    allocator;
        node_index_array_t      index;
        u32                     id;
        u32                     offset;
    };

    struct node_cursor_t final {
        u8*                     page;
        node_field_t*           field;
        node_header_t*          header;
        storage_t*              storage;
        u16                     offset;
        u16                     end_offset;
        u32                     id;
        b8                      ok;
    };

    struct token_t final {
        u32                     id;
        token_type_t            type;
        string::slice_t         slice;
    };

    struct nary_rule_t final {
        token_type_t            plus;
        token_type_t            minus;
    };

    struct token_rule_t final {
        void*                   tokenizer;
        token_type_t            type;
    };

    struct lexer_t final {
        source::buffer_t*       buf;
        memory::allocator_t*    allocator;
        array::array_t<token_t> tokens;
        u32                     token_idx;
    };

    struct associativity_t final {
        s8                      arity{};
        u8                      precedence{};
    };

    struct operator_precedence_t final {
        b8                      op{};
        associativity_t         left{};
        associativity_t         right{};
    };

    struct operator_rule_t final {
        void*                   handler;
        union {
            unary_op_type_t     unary;
            binary_op_type_t    binary;
        }                       type;
    };

    struct parser_t final {
        lexer_t*                lexer;
        memory::allocator_t*    allocator;
        storage_t               ast;
        intern::pool_t          intern_pool;
    };

    struct rpn_t final {
        scope_t*                scope;
        array::array_t<token_t> tokens;
        s8                      arg_count;
    };

    struct scope_t final {
        array::array_t<scope_t> children;
        array::array_t<rpn_t>   statements;
    };

    struct scope_cursor_t final {
        scope_t*                scope{};
        s32                     stmt_idx{};
        s32                     token_idx = -1;
    };

    namespace ast {
        u0 init(
            storage_t& storage,
            memory::allocator_t* allocator = context::current()->allocator);

        b8 write_field(
            node_cursor_t& cursor,
            node_field_type_t type,
            u32 value);

        u0 free(storage_t& storage);

        node_cursor_t write_header(
            storage_t& storage,
            node_header_type_t type,
            u32 size = 0);

        u8* make_page(storage_t& storage);

        b8 move_next(node_cursor_t& cursor);

        b8 next_field(node_cursor_t& cursor);

        b8 next_header(node_cursor_t& cursor);

        node_cursor_t make_cursor(storage_t& storage);

        node_cursor_t first_header(storage_t& storage);

        string::slice_t node_kind_name(node_kind_t kind);

        string::slice_t unary_op_name(unary_op_type_t type);

        node_cursor_t get_header(storage_t& storage, u32 id);

        node_cursor_t make_cursor(const node_cursor_t& other);

        string::slice_t binary_op_name(binary_op_type_t type);

        string::slice_t node_field_type_name(node_field_type_t type);

        u32 find_field(node_cursor_t& cursor, node_field_type_t type);

        string::slice_t node_header_type_name(node_header_type_t type);

        storage_t make(memory::allocator_t* allocator = context::current()->allocator);
    }

    namespace lexer {
        u0 free(lexer_t& lexer);

        b8 tokenize(lexer_t& lexer);

        u0 seek_start(lexer_t& lexer);

        u0 next_token(lexer_t& lexer);

        b8 has_more_tokens(lexer_t& lexer);

        token_t& current_token(lexer_t& lexer);

        const token_t& get_token(lexer_t& lexer, id_t id);

        string::slice_t token_type_name(token_type_t type);

        const token_t& peek_token(lexer_t& lexer, u32 count = 1);

        string::slice_t character_type_name(character_type_t type);

        lexer_t make(source::buffer_t* buf, memory::allocator_t* allocator = context::current()->allocator);

        u0 init(lexer_t& lexer, source::buffer_t* buf, memory::allocator_t* allocator = context::current()->allocator);
    }

    namespace parser {
        u0 init_rpn(
            rpn_t& expr,
            memory::allocator_t* allocator = context::current()->allocator);

        u0 init_scope(
            scope_t& scope,
            memory::allocator_t* allocator = context::current()->allocator);

        u0 free_rpn(rpn_t& expr);

        u0 reset_rpn(rpn_t& expr);

        u0 free(parser_t& parser);

        b8 parse(parser_t& parser);

        u0 free_scope(scope_t& scope);

        scope_t& make_child_scope(scope_t& scope, memory::allocator_t* allocator);

        rpn_t make_rpn(memory::allocator_t* allocator = context::current()->allocator);

        scope_t make_scope(memory::allocator_t* allocator = context::current()->allocator);

        parser_t make(lexer_t* lexer, memory::allocator_t* allocator = context::current()->allocator);

        u0 init(parser_t& parser, lexer_t* lexer, memory::allocator_t* allocator = context::current()->allocator);
    }
}