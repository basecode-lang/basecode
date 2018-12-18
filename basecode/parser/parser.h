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

#pragma once

#include <map>
#include <stack>
#include <string>
#include <fmt/format.h>
#include <unordered_map>
#include <common/result.h>
#include <common/source_file.h>
#include <boost/filesystem.hpp>
#include "ast.h"
#include "lexer.h"

namespace basecode::syntax {

    enum class precedence_t : uint8_t {
        lowest,
        assignment,
        comma,
        key_value,
        logical_or,
        logical_and,
        bitwise_or,
        bitwise_xor,
        bitwise_and,
        equality,
        relational,
        bitwise_shift_or_roll,
        sum,
        product,
        exponent,
        member_access,
        pointer_dereference,
        subscript,
        prefix,
        postfix,
        cast,
        type,
        variable,
        call
    };

    class parser;

    ///////////////////////////////////////////////////////////////////////////

    class infix_parser {
    public:
        virtual ~infix_parser() = default;

        virtual ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) = 0;

        virtual precedence_t precedence() const = 0;
    };

    ///////////////////////////////////////////////////////////////////////////

    class key_value_infix_parser : public infix_parser {
    public:
        key_value_infix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class comma_infix_parser : public infix_parser {
    public:
        comma_infix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class type_declaration_infix_parser : public infix_parser {
    public:
        type_declaration_infix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class pointer_dereference_infix_parser : public infix_parser {
    public:
        pointer_dereference_infix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class proc_call_infix_parser : public infix_parser {
    public:
        proc_call_infix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class array_subscript_infix_parser : public infix_parser {
    public:
        array_subscript_infix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class binary_operator_infix_parser : public infix_parser {
    public:
        binary_operator_infix_parser(
            precedence_t precedence,
            bool is_right_associative,
            bool with_assignment = false) noexcept;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;

    private:
        precedence_t _precedence;
        bool _with_assignment = false;
        bool _is_right_associative = false;
    };

    ///////////////////////////////////////////////////////////////////////////

    class constant_assignment_infix_parser : public infix_parser {
    public:
        constant_assignment_infix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        void precedence(precedence_t value) {
            _precedence = value;
        }

        precedence_t precedence() const override;

    private:
        precedence_t _precedence = precedence_t::assignment;
    };

    ///////////////////////////////////////////////////////////////////////////

    class assignment_infix_parser : public infix_parser {
    public:
        assignment_infix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class prefix_parser {
    public:
        virtual ~prefix_parser() = default;

        virtual ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////

    class subscript_declaration_prefix_parser : public prefix_parser {
    public:
        subscript_declaration_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class pointer_declaration_prefix_parser : public prefix_parser {
    public:
        pointer_declaration_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class with_member_access_prefix_parser : public prefix_parser {
    public:
        with_member_access_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class spread_prefix_parser : public prefix_parser {
    public:
        spread_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class label_prefix_parser : public prefix_parser {
    public:
        label_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class with_prefix_parser : public prefix_parser {
    public:
        with_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class while_prefix_parser : public prefix_parser {
    public:
        while_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class defer_prefix_parser : public prefix_parser {
    public:
        defer_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class union_prefix_parser : public prefix_parser {
    public:
        union_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class namespace_prefix_parser : public prefix_parser {
    public:
        namespace_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class struct_prefix_parser : public prefix_parser {
    public:
        struct_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class enum_prefix_parser : public prefix_parser {
    public:
        enum_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class for_in_prefix_parser : public prefix_parser {
    public:
        for_in_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class return_prefix_parser : public prefix_parser {
    public:
        return_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class if_prefix_parser : public prefix_parser {
    public:
        if_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class basic_block_prefix_parser : public prefix_parser {
    public:
        basic_block_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class group_prefix_parser : public prefix_parser {
    public:
        group_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class proc_expression_prefix_parser : public prefix_parser {
    public:
        proc_expression_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class lambda_expression_prefix_parser : public prefix_parser {
    public:
        lambda_expression_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class unary_operator_prefix_parser : public prefix_parser {
    public:
        explicit unary_operator_prefix_parser(precedence_t precedence) noexcept;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;

    private:
        precedence_t _precedence;
    };

    ///////////////////////////////////////////////////////////////////////////

    class keyword_literal_prefix_parser : public prefix_parser {
    public:
        keyword_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class number_literal_prefix_parser : public prefix_parser {
    public:
        number_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class string_literal_prefix_parser : public prefix_parser {
    public:
        string_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class char_literal_prefix_parser : public prefix_parser {
    public:
        char_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class line_comment_prefix_parser : public prefix_parser {
    public:
        line_comment_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class block_comment_prefix_parser : public prefix_parser {
    public:
        block_comment_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class type_tagged_symbol_prefix_parser : public prefix_parser {
    public:
        type_tagged_symbol_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class symbol_prefix_parser : public prefix_parser {
    public:
        symbol_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class from_prefix_parser : public prefix_parser {
    public:
        from_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class module_prefix_parser : public prefix_parser {
    public:
        module_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class raw_block_prefix_parser : public prefix_parser {
    public:
        raw_block_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class directive_prefix_parser : public prefix_parser {
    public:
        directive_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class attribute_prefix_parser : public prefix_parser {
    public:
        attribute_prefix_parser() = default;

        ast_node_shared_ptr parse(
            common::result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class parser {
    public:
        parser(
            common::source_file* source_file,
            syntax::ast_builder& builder);

        void error(
            common::result& r,
            const std::string& code,
            const std::string& message,
            const common::source_location& location);

        bool consume();

        void write_ast_graph(
            const boost::filesystem::path& path,
            const ast_node_shared_ptr& program_node);

        bool consume(token_t& token);

        bool current(token_t& token);

        bool peek(token_types_t type);

        bool look_ahead(size_t count);

        ast_node_shared_ptr parse_scope(
            common::result& r,
            token_t& token);

        syntax::ast_builder* ast_builder();

        ast_node_shared_ptr parse_expression(
            common::result& r,
            precedence_t precedence = precedence_t::lowest);

        ast_node_shared_ptr expect_expression(
            common::result& r,
            ast_node_types_t expected_type,
            precedence_t precedence = precedence_t::lowest);

        ast_node_shared_ptr parse(common::result& r);

        bool expect(common::result& r, token_t& token);

    protected:
        ast_node_shared_ptr parse_statement(common::result& r);

    private:
        precedence_t current_infix_precedence();

        infix_parser* infix_parser_for(token_types_t type);

        prefix_parser* prefix_parser_for(token_types_t type);

    private:
        static inline if_prefix_parser s_if_prefix_parser {};
        static inline with_prefix_parser s_with_prefix_parser {};
        static inline enum_prefix_parser s_enum_prefix_parser {};
        static inline from_prefix_parser s_from_prefix_parser {};
        static inline defer_prefix_parser s_defer_prefix_parser {};
        static inline union_prefix_parser s_union_prefix_parser {};
        static inline group_prefix_parser s_group_prefix_parser {};
        static inline label_prefix_parser s_label_prefix_parser {};
        static inline module_prefix_parser s_module_prefix_parser {};
        static inline struct_prefix_parser s_struct_prefix_parser {};
        static inline for_in_prefix_parser s_for_in_prefix_parser {};
        static inline return_prefix_parser s_return_prefix_parser {};
        static inline symbol_prefix_parser s_symbol_prefix_parser {};
        static inline spread_prefix_parser s_spread_prefix_parser {};
        static inline raw_block_prefix_parser s_raw_block_prefix_parser {};
        static inline namespace_prefix_parser s_namespace_prefix_parser {};
        static inline attribute_prefix_parser s_attribute_prefix_parser {};
        static inline directive_prefix_parser s_directive_prefix_parser {};
        static inline while_prefix_parser s_while_statement_prefix_parser {};
        static inline basic_block_prefix_parser s_basic_block_prefix_parser {};
        static inline char_literal_prefix_parser s_char_literal_prefix_parser {};
        static inline line_comment_prefix_parser s_line_comment_prefix_parser {};
        static inline block_comment_prefix_parser s_block_comment_prefix_parser {};
        static inline string_literal_prefix_parser s_string_literal_prefix_parser {};
        static inline number_literal_prefix_parser s_number_literal_prefix_parser {};
        static inline keyword_literal_prefix_parser s_keyword_literal_prefix_parser {};
        static inline proc_expression_prefix_parser s_proc_expression_prefix_parser {};
        static inline lambda_expression_prefix_parser s_lambda_expression_prefix_parser {};
        static inline type_tagged_symbol_prefix_parser s_type_tagged_symbol_prefix_parser {};
        static inline with_member_access_prefix_parser s_with_member_access_prefix_parser {};
        static inline unary_operator_prefix_parser s_negate_prefix_parser {precedence_t::sum};
        static inline unary_operator_prefix_parser s_not_prefix_parser {precedence_t::prefix};
        static inline pointer_declaration_prefix_parser s_pointer_declaration_prefix_parser {};
        static inline subscript_declaration_prefix_parser s_subscript_declaration_prefix_parser {};
        static inline unary_operator_prefix_parser s_binary_not_prefix_parser {precedence_t::prefix};

        static inline std::unordered_map<token_types_t, prefix_parser*> s_prefix_parsers = {
            {token_types_t::if_literal,             &s_if_prefix_parser},
            {token_types_t::bang,                   &s_not_prefix_parser},
            {token_types_t::enum_literal,           &s_enum_prefix_parser},
            {token_types_t::with_literal,           &s_with_prefix_parser},
            {token_types_t::from_literal,           &s_from_prefix_parser},
            {token_types_t::label,                  &s_label_prefix_parser},
            {token_types_t::left_paren,             &s_group_prefix_parser},
            {token_types_t::union_literal,          &s_union_prefix_parser},
            {token_types_t::defer_literal,          &s_defer_prefix_parser},
            {token_types_t::struct_literal,         &s_struct_prefix_parser},
            {token_types_t::for_literal,            &s_for_in_prefix_parser},
            {token_types_t::minus,                  &s_negate_prefix_parser},
            {token_types_t::return_literal,         &s_return_prefix_parser},
            {token_types_t::identifier,             &s_symbol_prefix_parser},
            {token_types_t::module_literal,         &s_module_prefix_parser},
            {token_types_t::spread_operator,        &s_spread_prefix_parser},
            {token_types_t::raw_block,              &s_raw_block_prefix_parser},
            {token_types_t::namespace_literal,      &s_namespace_prefix_parser},
            {token_types_t::attribute,              &s_attribute_prefix_parser},
            {token_types_t::directive,              &s_directive_prefix_parser},
            {token_types_t::tilde,                  &s_binary_not_prefix_parser},
            {token_types_t::left_curly_brace,       &s_basic_block_prefix_parser},
            {token_types_t::character_literal,      &s_char_literal_prefix_parser},
            {token_types_t::line_comment,           &s_line_comment_prefix_parser},
            {token_types_t::block_comment,          &s_block_comment_prefix_parser},
            {token_types_t::string_literal,         &s_string_literal_prefix_parser},
            {token_types_t::number_literal,         &s_number_literal_prefix_parser},
            {token_types_t::while_literal,          &s_while_statement_prefix_parser},
            {token_types_t::proc_literal,           &s_proc_expression_prefix_parser},
            {token_types_t::true_literal,           &s_keyword_literal_prefix_parser},
            {token_types_t::nil_literal,            &s_keyword_literal_prefix_parser},
            {token_types_t::false_literal,          &s_keyword_literal_prefix_parser},
            {token_types_t::break_literal,          &s_keyword_literal_prefix_parser},
            {token_types_t::import_literal,         &s_keyword_literal_prefix_parser},
            {token_types_t::continue_literal,       &s_keyword_literal_prefix_parser},
            {token_types_t::switch_literal,         &s_keyword_literal_prefix_parser},
            {token_types_t::case_literal,           &s_keyword_literal_prefix_parser},
            {token_types_t::fallthrough_literal,    &s_keyword_literal_prefix_parser},
            {token_types_t::question,               &s_keyword_literal_prefix_parser},
            {token_types_t::lambda_literal,         &s_lambda_expression_prefix_parser},
            {token_types_t::period,                 &s_with_member_access_prefix_parser},
            {token_types_t::type_tagged_identifier, &s_type_tagged_symbol_prefix_parser},
            {token_types_t::caret,                  &s_pointer_declaration_prefix_parser},
            {token_types_t::left_square_bracket,    &s_subscript_declaration_prefix_parser},
        };

        static inline comma_infix_parser s_comma_infix_parser {};
        static inline key_value_infix_parser s_key_value_infix_parser {};
        static inline proc_call_infix_parser s_proc_call_infix_parser {};
        static inline assignment_infix_parser s_assignment_infix_parser {};
        static inline array_subscript_infix_parser s_array_subscript_infix_parser {};
        static inline type_declaration_infix_parser s_type_declaration_infix_parser {};
        static inline constant_assignment_infix_parser s_constant_assignment_infix_parser {};
        static inline pointer_dereference_infix_parser s_pointer_dereference_infix_parser {};
        static inline binary_operator_infix_parser s_sum_bin_op_parser {
            precedence_t::sum,
            false};
        static inline binary_operator_infix_parser s_product_bin_op_parser {
            precedence_t::product,
            false};
        static inline binary_operator_infix_parser s_exponent_bin_op_parser {
            precedence_t::exponent,
            true};
        static inline binary_operator_infix_parser s_equality_bin_op_parser {
            precedence_t::equality,
            false};
        static inline binary_operator_infix_parser s_relational_bin_op_parser {
            precedence_t::relational,
            false};
        static inline binary_operator_infix_parser s_logical_or_bin_op_parser {
            precedence_t::logical_or,
            false};
        static inline binary_operator_infix_parser s_logical_and_bin_op_parser {
            precedence_t::logical_and,
            false};
        static inline binary_operator_infix_parser s_bitwise_or_bin_op_parser {
            precedence_t::bitwise_or,
            false};
        static inline binary_operator_infix_parser s_bitwise_and_bin_op_parser {
            precedence_t::bitwise_and,
            false};
        static inline binary_operator_infix_parser s_bitwise_xor_bin_op_parser {
            precedence_t::bitwise_xor,
            false};
        static inline binary_operator_infix_parser s_sum_with_assign_bin_op_parser {
            precedence_t::sum,
            false,
            true};
        static inline binary_operator_infix_parser s_member_access_bin_op_parser {
            precedence_t::member_access,
            false};
        static inline binary_operator_infix_parser s_bitwise_with_assign_bin_op_parser {
            precedence_t::bitwise_and,
            false,
            true};
        static inline binary_operator_infix_parser s_product_with_assign_bin_op_parser {
            precedence_t::product,
            false,
            true};
        static inline binary_operator_infix_parser s_bitwise_shift_or_roll_bin_op_parser {
            precedence_t::bitwise_shift_or_roll,
            false};

        static inline std::unordered_map<token_types_t, infix_parser*> s_infix_parsers = {
            {token_types_t::plus,                   &s_sum_bin_op_parser},
            {token_types_t::minus,                  &s_sum_bin_op_parser},
            {token_types_t::comma,                  &s_comma_infix_parser},
            {token_types_t::slash,                  &s_product_bin_op_parser},
            {token_types_t::percent,                &s_product_bin_op_parser},
            {token_types_t::asterisk,               &s_product_bin_op_parser},
            {token_types_t::key_value_operator,     &s_key_value_infix_parser},
            {token_types_t::exponent,               &s_exponent_bin_op_parser},
            {token_types_t::left_paren,             &s_proc_call_infix_parser},
            {token_types_t::equals,                 &s_equality_bin_op_parser},
            {token_types_t::not_equals,             &s_equality_bin_op_parser},
            {token_types_t::assignment,             &s_assignment_infix_parser},
            {token_types_t::less_than,              &s_relational_bin_op_parser},
            {token_types_t::greater_than,           &s_relational_bin_op_parser},
            {token_types_t::less_than_equal,        &s_relational_bin_op_parser},
            {token_types_t::greater_than_equal,     &s_relational_bin_op_parser},
            {token_types_t::logical_or,             &s_logical_or_bin_op_parser},
            {token_types_t::pipe,                   &s_bitwise_or_bin_op_parser},
            {token_types_t::ampersand,              &s_bitwise_and_bin_op_parser},
            {token_types_t::xor_literal,            &s_bitwise_xor_bin_op_parser},
            {token_types_t::logical_and,            &s_logical_and_bin_op_parser},
            {token_types_t::period,                 &s_member_access_bin_op_parser},
            {token_types_t::left_square_bracket,    &s_array_subscript_infix_parser},
            {token_types_t::colon,                  &s_type_declaration_infix_parser},
            {token_types_t::plus_equal_literal,     &s_sum_with_assign_bin_op_parser},
            {token_types_t::minus_equal_literal,    &s_sum_with_assign_bin_op_parser},
            {token_types_t::caret,                  &s_pointer_dereference_infix_parser},
            {token_types_t::constant_assignment,    &s_constant_assignment_infix_parser},
            {token_types_t::pipe,                   &s_bitwise_with_assign_bin_op_parser},
            {token_types_t::tilde,                  &s_bitwise_with_assign_bin_op_parser},
            {token_types_t::ampersand,              &s_bitwise_with_assign_bin_op_parser},
            {token_types_t::divide_equal_literal,   &s_product_with_assign_bin_op_parser},
            {token_types_t::modulus_equal_literal,  &s_product_with_assign_bin_op_parser},
            {token_types_t::multiply_equal_literal, &s_product_with_assign_bin_op_parser},
            {token_types_t::shl_literal,            &s_bitwise_shift_or_roll_bin_op_parser},
            {token_types_t::shr_literal,            &s_bitwise_shift_or_roll_bin_op_parser},
            {token_types_t::rol_literal,            &s_bitwise_shift_or_roll_bin_op_parser},
            {token_types_t::ror_literal,            &s_bitwise_shift_or_roll_bin_op_parser},
        };

        syntax::lexer _lexer;
        std::vector<token_t> _tokens {};
        syntax::ast_builder& _ast_builder;
        common::source_file* _source_file = nullptr;
    };

};