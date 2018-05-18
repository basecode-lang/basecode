#pragma once

#include <map>
#include <stack>
#include <string>
#include <fmt/format.h>
#include "ast.h"
#include "lexer.h"
#include "result.h"

namespace basecode {

    enum class precedence_t : uint8_t {
        assignment = 1,
        conditional,
        sum,
        product,
        bitwise,
        exponent,
        prefix,
        postfix,
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
            result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) = 0;

        virtual precedence_t precedence() const = 0;
    };

    ///////////////////////////////////////////////////////////////////////////

    class type_identifier_infix_parser : public infix_parser {
    public:
        type_identifier_infix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class variable_reference_infix_parser : public infix_parser {
    public:
        variable_reference_infix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class fn_call_infix_parser : public infix_parser {
    public:
        fn_call_infix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
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
            bool is_right_associative);

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) override;

        precedence_t precedence() const override;

    private:
        precedence_t _precedence;
        bool _is_right_associative = false;
    };

    ///////////////////////////////////////////////////////////////////////////

    class assignment_infix_parser : public infix_parser {
    public:
        assignment_infix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
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
            result& r,
            parser* parser,
            token_t& token) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////

    class group_prefix_parser : public prefix_parser {
    public:
        group_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class unary_operator_prefix_parser : public prefix_parser {
    public:
        explicit unary_operator_prefix_parser(precedence_t precedence);

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;

    private:
        precedence_t _precedence;
    };

    ///////////////////////////////////////////////////////////////////////////

    class null_literal_prefix_parser : public prefix_parser {
    public:
        null_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class number_literal_prefix_parser : public prefix_parser {
    public:
        number_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class string_literal_prefix_parser : public prefix_parser {
    public:
        string_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class char_literal_prefix_parser : public prefix_parser {
    public:
        char_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class boolean_literal_prefix_parser : public prefix_parser {
    public:
        boolean_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class line_comment_prefix_parser : public prefix_parser {
    public:
        line_comment_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class block_comment_prefix_parser : public prefix_parser {
    public:
        block_comment_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class variable_declaration_prefix_parser : public prefix_parser {
    public:
        variable_declaration_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class attribute_prefix_parser : public prefix_parser {
    public:
        attribute_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class array_subscript_prefix_parser : public prefix_parser {
    public:
        array_subscript_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class parser {
    public:
        explicit parser(std::istream& source);

        virtual ~parser();

        void error(
            result& r,
            const std::string& code,
            const std::string& message,
            uint32_t line,
            uint32_t column);

        bool consume();

        bool consume(token_t& token);

        bool peek(token_types_t type);

        bool look_ahead(size_t count);

        basecode::ast_builder* ast_builder();

        ast_node_shared_ptr parse(result& r);

        bool expect(result& r, token_t& token);

        ast_node_shared_ptr parse_expression(result& r, uint8_t precedence);

    protected:
        ast_node_shared_ptr parse_scope(result& r);

        ast_node_shared_ptr parse_statement(result& r);

    private:
        uint8_t current_infix_precedence();

        infix_parser* infix_parser_for(token_types_t type);

        prefix_parser* prefix_parser_for(token_types_t type);

    private:
        static inline group_prefix_parser s_group_prefix_parser {};
        static inline attribute_prefix_parser s_attribute_prefix_parser {};
        static inline null_literal_prefix_parser s_null_literal_prefix_parser {};
        static inline char_literal_prefix_parser s_char_literal_prefix_parser {};
        static inline line_comment_prefix_parser s_line_comment_prefix_parser {};
        static inline block_comment_prefix_parser s_block_comment_prefix_parser {};
        static inline string_literal_prefix_parser s_string_literal_prefix_parser {};
        static inline number_literal_prefix_parser s_number_literal_prefix_parser {};
        static inline array_subscript_prefix_parser s_array_subscript_prefix_parser {};
        static inline boolean_literal_prefix_parser s_boolean_literal_prefix_parser {};
        static inline unary_operator_prefix_parser s_negate_prefix_parser {precedence_t::sum};
        static inline unary_operator_prefix_parser s_not_prefix_parser {precedence_t::prefix};
        static inline variable_declaration_prefix_parser s_variable_declaration_prefix_parser {};
        static inline unary_operator_prefix_parser s_binary_not_prefix_parser {precedence_t::prefix};

        static inline std::map<token_types_t, prefix_parser*> s_prefix_parsers = {
            {token_types_t::bang,                &s_not_prefix_parser},
            {token_types_t::left_paren,          &s_group_prefix_parser},
            {token_types_t::minus,               &s_negate_prefix_parser},
            {token_types_t::attribute,           &s_attribute_prefix_parser},
            {token_types_t::tilde,               &s_binary_not_prefix_parser},
            {token_types_t::null_literal,        &s_null_literal_prefix_parser},
            {token_types_t::character_literal,   &s_char_literal_prefix_parser},
            {token_types_t::line_comment,        &s_line_comment_prefix_parser},
            {token_types_t::block_comment,       &s_block_comment_prefix_parser},
            {token_types_t::string_literal,      &s_string_literal_prefix_parser},
            {token_types_t::number_literal,      &s_number_literal_prefix_parser},
            {token_types_t::left_square_bracket, &s_array_subscript_prefix_parser},
            {token_types_t::true_literal,        &s_boolean_literal_prefix_parser},
            {token_types_t::false_literal,       &s_boolean_literal_prefix_parser},
            {token_types_t::identifier,          &s_variable_declaration_prefix_parser},
        };

        static inline fn_call_infix_parser s_fn_call_infix_parser {};
        static inline assignment_infix_parser s_assignment_infix_parser {};
        static inline type_identifier_infix_parser s_type_identifier_infix_parser {};
        static inline variable_reference_infix_parser s_variable_reference_infix_parser {};
        static inline binary_operator_infix_parser s_sum_binary_op_parser {precedence_t::sum, false};
        static inline binary_operator_infix_parser s_product_binary_op_parser {precedence_t::product, false};
        static inline binary_operator_infix_parser s_bitwise_binary_op_parser {precedence_t::bitwise, false};
        static inline binary_operator_infix_parser s_exponent_binary_op_parser {precedence_t::exponent, true};

        static inline std::map<token_types_t, infix_parser*> s_infix_parsers = {
            {token_types_t::left_paren,    &s_fn_call_infix_parser},
            {token_types_t::minus,         &s_sum_binary_op_parser},
            {token_types_t::plus,          &s_sum_binary_op_parser},
            {token_types_t::assignment,    &s_assignment_infix_parser},
            {token_types_t::slash,         &s_product_binary_op_parser},
            {token_types_t::asterisk,      &s_product_binary_op_parser},
            {token_types_t::percent,       &s_product_binary_op_parser},
            {token_types_t::ampersand,     &s_bitwise_binary_op_parser},
            {token_types_t::pipe,          &s_bitwise_binary_op_parser},
            {token_types_t::caret,         &s_exponent_binary_op_parser},
            {token_types_t::colon,         &s_type_identifier_infix_parser},
            {token_types_t::identifier,    &s_variable_reference_infix_parser},
        };

        std::istream& _source;
        basecode::lexer _lexer;
        std::vector<token_t> _tokens {};
        basecode::ast_builder _ast_builder;
    };

};