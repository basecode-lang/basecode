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

    class null_literal_prefix_parser : public prefix_parser {
    public:
        null_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    class number_literal_prefix_parser : public prefix_parser {
    public:
        number_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    class string_literal_prefix_parser : public prefix_parser {
    public:
        string_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    class char_literal_prefix_parser : public prefix_parser {
    public:
        char_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    class boolean_literal_prefix_parser : public prefix_parser {
    public:
        boolean_literal_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    class line_comment_prefix_parser : public prefix_parser {
    public:
        line_comment_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    class block_comment_prefix_parser : public prefix_parser {
    public:
        block_comment_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    class variable_declaration_prefix_parser : public prefix_parser {
    public:
        variable_declaration_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

    class attribute_prefix_parser : public prefix_parser {
    public:
        attribute_prefix_parser() = default;

        ast_node_shared_ptr parse(
            result& r,
            parser* parser,
            token_t& token) override;
    };

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
        static inline null_literal_prefix_parser s_null_literal_prefix_parser {};
        static inline char_literal_prefix_parser s_char_literal_prefix_parser {};
        static inline string_literal_prefix_parser s_string_literal_prefix_parser {};
        static inline number_literal_prefix_parser s_number_literal_prefix_parser {};
        static inline attribute_prefix_parser s_attribute_prefix_parser {};
        static inline line_comment_prefix_parser s_line_comment_prefix_parser {};
        static inline block_comment_prefix_parser s_block_comment_prefix_parser {};
        static inline array_subscript_prefix_parser s_array_subscript_prefix_parser {};
        static inline boolean_literal_prefix_parser s_boolean_literal_prefix_parser {};
        static inline variable_declaration_prefix_parser s_variable_declaration_prefix_parser {};

        static inline std::map<token_types_t, prefix_parser*> s_prefix_parsers = {
            {token_types_t::attribute,           &s_attribute_prefix_parser},
            {token_types_t::line_comment,        &s_line_comment_prefix_parser},
            {token_types_t::block_comment,       &s_block_comment_prefix_parser},
            {token_types_t::identifier,          &s_variable_declaration_prefix_parser},
            {token_types_t::left_square_bracket, &s_array_subscript_prefix_parser},
            {token_types_t::true_literal,        &s_boolean_literal_prefix_parser},
            {token_types_t::false_literal,       &s_boolean_literal_prefix_parser},
            {token_types_t::null_literal,        &s_null_literal_prefix_parser},
            {token_types_t::character_literal,   &s_char_literal_prefix_parser},
            {token_types_t::string_literal,      &s_string_literal_prefix_parser},
            {token_types_t::number_literal,      &s_number_literal_prefix_parser}
        };

        static inline assignment_infix_parser s_assignment_infix_parser {};
        static inline type_identifier_infix_parser s_type_identifier_infix_parser {};
        static inline variable_reference_infix_parser s_variable_reference_infix_parser {};

        static inline std::map<token_types_t, infix_parser*> s_infix_parsers = {
            {token_types_t::identifier,    &s_variable_reference_infix_parser},
            {token_types_t::colon,         &s_type_identifier_infix_parser},
            {token_types_t::assignment,    &s_assignment_infix_parser},
        };

        std::istream& _source;
        basecode::lexer _lexer;
        std::vector<token_t> _tokens {};
        basecode::ast_builder _ast_builder;
    };

};