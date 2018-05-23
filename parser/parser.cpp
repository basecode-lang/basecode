// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include <regex>
#include <iomanip>
#include <sstream>
#include <iostream>
#include "parser.h"

namespace basecode::syntax {

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr union_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto union_node = parser->ast_builder()->union_node(token);
        union_node->rhs = parser->parse_expression(r, 0);
        return union_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr namespace_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto namespace_node = parser->ast_builder()->namespace_node(token);
        namespace_node->rhs = parser->parse_expression(r, 0);
        return namespace_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr struct_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto struct_node = parser->ast_builder()->struct_node(token);
        struct_node->rhs = parser->parse_expression(r, 0);
        return struct_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr enum_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto enum_node = parser->ast_builder()->enum_node(token);
        enum_node->rhs = parser->parse_expression(r, 0);
        return enum_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr for_in_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto for_node = parser->ast_builder()->for_in_node();
        for_node->lhs = parser->parse_expression(r, 0);

        token_t in_token;
        in_token.type = token_types_t::in_literal;
        if (!parser->expect(r, in_token))
            return nullptr;

        for_node->rhs = parser->parse_expression(r, 0);
        for_node->children.push_back(parser->parse_expression(r, 0));

        return for_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr return_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto return_node = parser->ast_builder()->return_node();

        while (true) {
            return_node->rhs->children.push_back(parser->parse_expression(r, 0));
            if (!parser->peek(token_types_t::comma))
                break;
            parser->consume();
        }

        return return_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr if_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto if_node = parser->ast_builder()->if_node();
        if_node->lhs = parser->parse_expression(r, 0);
        if_node->children.push_back(parser->parse_expression(r, 0));

        auto current_branch = if_node;
        while (true) {
            if (!parser->peek(token_types_t::else_if_literal))
                break;
            parser->consume();
            current_branch->rhs = parser->ast_builder()->else_if_node();
            current_branch->rhs->lhs = parser->parse_expression(r, 0);
            current_branch->rhs->children.push_back(parser->parse_expression(r, 0));
            current_branch = current_branch->rhs;
        }

        if (parser->peek(token_types_t::else_literal)) {
            parser->consume();
            current_branch->rhs = parser->ast_builder()->else_node();
            current_branch->rhs->children.push_back(parser->parse_expression(r, 0));
        }

        return if_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr type_identifier_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto is_spread = false;
        auto is_pointer = false;
        ast_node_shared_ptr array_decl_node = nullptr;

        if (parser->peek(token_types_t::asterisk)) {
            parser->consume();
            is_pointer = true;
        }

        if (parser->peek(token_types_t::left_square_bracket)) {
            array_decl_node = parser->parse_expression(
                r,
                static_cast<uint8_t>(precedence_t::variable));
        }

        if (parser->peek(token_types_t::spread_operator)) {
            parser->consume();
            is_spread = true;
        }

        token_t type_identifier;
        type_identifier.type = token_types_t::identifier;

        if (parser->peek(token_types_t::none_literal)) {
            type_identifier.value = "none";
            parser->consume();
        } else {
            if (!parser->expect(r, type_identifier)) {
                parser->error(
                    r,
                    "B027",
                    "type expected.",
                    token.line,
                    token.column);
                return nullptr;
            }
        }

        auto type_node = parser->ast_builder()->type_identifier_node(type_identifier);

        if (is_pointer)
            type_node->flags |= ast_node_t::flags_t::pointer;

        if (array_decl_node != nullptr) {
            type_node->rhs = array_decl_node;
            type_node->flags |= ast_node_t::flags_t::array;
        }

        if (is_spread)
            type_node->flags |= ast_node_t::flags_t::spread;

        return type_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr basic_block_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->parse_scope(r);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr fn_decl_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto fn_decl_node = parser->ast_builder()->fn_decl_node();

        token_t left_paren_token;
        left_paren_token.type = token_types_t::left_paren;
        if (!parser->expect(r, left_paren_token))
            return nullptr;

        if (!parser->peek(token_types_t::right_paren)) {
            while (true) {
                fn_decl_node->rhs->children.push_back(parser->parse_expression(r, 0));
                if (!parser->peek(token_types_t::comma))
                    break;
                parser->consume();
            }
        }

        token_t right_paren_token;
        right_paren_token.type = token_types_t::right_paren;
        if (!parser->expect(r, right_paren_token))
            return nullptr;

        fn_decl_node->lhs = parser->parse_expression(r, 0);

        if (!parser->peek(token_types_t::semi_colon))
            fn_decl_node->children.push_back(parser->parse_expression(r, 0));

        return fn_decl_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr group_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto expression_node = parser->ast_builder()->expression_node();
        expression_node->lhs = parser->parse_expression(r, 0);
        token_t right_paren_token;
        right_paren_token.type = token_types_t::right_paren;
        if (!parser->expect(r, right_paren_token))
            return nullptr;
        return expression_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    unary_operator_prefix_parser::unary_operator_prefix_parser(
        precedence_t precedence) : _precedence(precedence) {

    }

    ast_node_shared_ptr unary_operator_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto unary_operator_node = parser->ast_builder()->unary_operator_node(token);
        unary_operator_node->rhs = parser->parse_expression(
            r,
            static_cast<uint8_t>(_precedence));
        return unary_operator_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr keyword_literal_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        switch (token.type) {
            case token_types_t::break_literal:
                return parser->ast_builder()->break_node(token);
            case token_types_t::continue_literal:
                return parser->ast_builder()->continue_node(token);
            case token_types_t::none_literal:
                return parser->ast_builder()->none_literal_node(token);
            case token_types_t::empty_literal:
                return parser->ast_builder()->empty_literal_node(token);
            case token_types_t::null_literal:
                return parser->ast_builder()->null_literal_node(token);
            case token_types_t::true_literal:
            case token_types_t::false_literal:
                return parser->ast_builder()->boolean_literal_node(token);
            default:
                return nullptr;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr number_literal_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->number_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr string_literal_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->string_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr char_literal_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->character_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr line_comment_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->line_comment_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr block_comment_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->block_comment_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr symbol_reference_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto symbol_reference_node = parser->ast_builder()->qualified_symbol_reference_node();

        while (true) {
            auto symbol_node = parser->ast_builder()->symbol_reference_node(token);
            symbol_reference_node->lhs->children.push_back(symbol_node);
            if (!parser->peek(token_types_t::scope_operator)
            &&  !parser->peek(token_types_t::period))
                break;
            parser->consume();
            if (!parser->expect(r, token))
                return nullptr;
        }

        return symbol_reference_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr fn_call_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto fn_call_node = parser->ast_builder()->fn_call_node();
        fn_call_node->lhs = lhs;

        if (!parser->peek(token_types_t::right_paren)) {
            while (true) {
                fn_call_node->rhs->children.push_back(parser->parse_expression(r, 0));
                if (!parser->peek(token_types_t::comma))
                    break;
                parser->consume();
            }
        }

        token_t right_paren_token;
        right_paren_token.type = token_types_t::right_paren;
        if (!parser->expect(r, right_paren_token))
            return nullptr;

        return fn_call_node;
    }

    precedence_t fn_call_infix_parser::precedence() const {
        return precedence_t::call;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr symbol_reference_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto symbol_reference_node = parser->ast_builder()->qualified_symbol_reference_node();

        while (true) {
            auto symbol_node = parser->ast_builder()->symbol_reference_node(token);
            symbol_reference_node->lhs->children.push_back(symbol_node);
            if (!parser->peek(token_types_t::scope_operator)
            &&  !parser->peek(token_types_t::period))
                break;
            parser->consume();
            if (!parser->expect(r, token))
                return nullptr;
        }

        lhs->rhs = symbol_reference_node;

        return lhs;
    }

    precedence_t symbol_reference_infix_parser::precedence() const {
        return precedence_t::variable;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr type_identifier_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto is_spread = false;
        auto is_pointer = false;
        ast_node_shared_ptr array_decl_node = nullptr;

        if (parser->peek(token_types_t::asterisk)) {
            parser->consume();
            is_pointer = true;
        }

        if (parser->peek(token_types_t::left_square_bracket)) {
            array_decl_node = parser->parse_expression(
                r,
                static_cast<uint8_t>(precedence_t::variable));
        }

        if (parser->peek(token_types_t::spread_operator)) {
            parser->consume();
            is_spread = true;
        }

        token_t type_identifier;
        type_identifier.type = token_types_t::identifier;

        if (parser->peek(token_types_t::none_literal)) {
            type_identifier.value = "none";
            parser->consume();
        } else {
            if (!parser->expect(r, type_identifier)) {
                parser->error(
                    r,
                    "B027",
                    "type name expected for variable declaration.",
                    token.line,
                    token.column);
                return nullptr;
            }
        }

        lhs->rhs = parser->ast_builder()->type_identifier_node(type_identifier);

        if (is_pointer)
            lhs->rhs->flags |= ast_node_t::flags_t::pointer;

        if (array_decl_node != nullptr) {
            lhs->rhs->rhs = array_decl_node;
            lhs->rhs->flags |= ast_node_t::flags_t::array;
        }

        if (is_spread)
            lhs->rhs->flags |= ast_node_t::flags_t::spread;

        return lhs;
    }

    precedence_t type_identifier_infix_parser::precedence() const {
        return precedence_t::type;
    }

    ///////////////////////////////////////////////////////////////////////////

    binary_operator_infix_parser::binary_operator_infix_parser(
            precedence_t precedence,
            bool is_right_associative) : _precedence(precedence),
                                         _is_right_associative(is_right_associative) {
    }

    ast_node_shared_ptr binary_operator_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto associative_precedence = static_cast<uint8_t>(
            static_cast<uint8_t>(_precedence) - (_is_right_associative ? 1 : 0));
        return parser->ast_builder()->binary_operator_node(
            lhs,
            token,
            parser->parse_expression(r, associative_precedence));
    }

    precedence_t binary_operator_infix_parser::precedence() const {
        return _precedence;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr assignment_infix_parser::parse(
            common::result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto assignment_node = parser->ast_builder()->assignment_node();
        assignment_node->lhs = lhs;
        assignment_node->rhs = parser->parse_expression(
            r,
            static_cast<uint8_t>(precedence_t::assignment) - 1);
        return assignment_node;
    }

    precedence_t assignment_infix_parser::precedence() const {
        return precedence_t::assignment;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr directive_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto directive_node = parser->ast_builder()->directive_node(token);
        directive_node->lhs = parser->parse_expression(r, 0);
        return directive_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr attribute_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        auto attribute_node = parser->ast_builder()->attribute_node(token);
        attribute_node->lhs = parser->parse_expression(r, 0);
        return attribute_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr array_subscript_prefix_parser::parse(
            common::result& r,
            parser* parser,
            token_t& token) {
        ast_node_shared_ptr subscript_node = parser->ast_builder()->subscript_node();
        if (!parser->peek(token_types_t::right_square_bracket)) {
            subscript_node->lhs = parser->parse_expression(r, 0);
        }

        token_t right_bracket_token;
        right_bracket_token.type = token_types_t::right_square_bracket;
        if (!parser->expect(r, right_bracket_token))
            return nullptr;

        return subscript_node;
    }

    ///////////////////////////////////////////////////////////////////////////

    parser::parser(std::istream& source) : _source(source),
                                           _lexer(source) {
    }

    parser::~parser() {
    }

    void parser::error(
            common::result& r,
            const std::string& code,
            const std::string& message,
            uint32_t line,
            uint32_t column) {
        _source.seekg(0, std::ios_base::beg);

        std::vector<std::string> source_lines {};
        std::string source_line;
        while (std::getline(_source, source_line)) {
            source_lines.push_back(source_line);
        }

        std::stringstream stream;
        stream << "\n";
        auto start_line = std::max<int32_t>(0, static_cast<int32_t>(line) - 4);
        auto stop_line = std::min<int32_t>(
            static_cast<int32_t>(source_lines.size()),
            line + 4);
        auto message_indicator = "^ " + message;
        for (int32_t i = start_line; i < stop_line; i++) {
            if (i == static_cast<int32_t>(line)) {
                stream << fmt::format("{:04d}: ", i + 1)
                       << source_lines[i] << "\n"
                       << fmt::format("{}{}",
                                      std::string(column + 6, ' '),
                                      message_indicator);
            } else {
                stream << fmt::format("{:04d}: ", i + 1)
                       << source_lines[i];
            }

            if (i < static_cast<int32_t>(stop_line - 1))
                stream << "\n";
        }

        r.add_message(
            code,
            fmt::format("{} @ {}:{}", message, line, column),
            stream.str(),
            true);
    }

    bool parser::consume() {
        token_t token;
        return consume(token);
    }

    bool parser::consume(token_t& token) {
        if (!look_ahead(0))
            return false;

        token = _tokens.front();
        _tokens.erase(_tokens.begin());

        return token.type != token_types_t::end_of_file;
    }

    bool parser::peek(token_types_t type) {
        if (!look_ahead(0))
            return false;
        auto& token = _tokens.front();
        return token.type == type;
    }

    bool parser::look_ahead(size_t count) {
        while (count >= _tokens.size() && _lexer.has_next()) {
            token_t token;
            if (_lexer.next(token))
                _tokens.push_back(token);
        }
        return !_tokens.empty();
    }

    uint8_t parser::current_infix_precedence() {
        if (!look_ahead(0))
            return 0;

        auto& token = _tokens.front();
        auto infix_parser = infix_parser_for(token.type);
        if (infix_parser != nullptr)
            return static_cast<uint8_t>(infix_parser->precedence());

        return 0;
    }

    syntax::ast_builder* parser::ast_builder() {
        return &_ast_builder;
    }

    ast_node_shared_ptr parser::parse(common::result& r) {
        return parse_scope(r);
    }

    bool parser::expect(common::result& r, token_t& token) {
        if (!look_ahead(0))
            return false;

        std::string expected_name = token.name();
        auto expected_type = token.type;
        token = _tokens.front();
        if (token.type != expected_type) {
            error(
                r,
                "B016",
                fmt::format("expected token '{}' but found '{}'.", expected_name, token.name()),
                token.line,
                token.column);
            return false;
        }

        _tokens.erase(_tokens.begin());

        return true;
    }

    ast_node_shared_ptr parser::parse_scope(common::result& r) {
        auto scope = _ast_builder.begin_scope();

        while (true) {
            if (peek(token_types_t::right_curly_brace)) {
                consume();
                break;
            }

            auto node = parse_statement(r);
            if (node == nullptr)
                break;
            scope->children.push_back(node);

            if (node->type == ast_node_types_t::statement) {
                token_t line_terminator_token;
                line_terminator_token.type = token_types_t::semi_colon;
                if (!expect(r, line_terminator_token))
                    break;
            }
        }

        return _ast_builder.end_scope();
    }

    ast_node_shared_ptr parser::parse_statement(common::result& r) {
        auto expression = parse_expression(r, 0);
        if (expression == nullptr)
            return nullptr;

        if (expression->type == ast_node_types_t::line_comment
        ||  expression->type == ast_node_types_t::block_comment
        ||  expression->type == ast_node_types_t::basic_block) {
            return expression;
        }

        auto statement_node = _ast_builder.statement_node();
        // XXX:  lhs should be used or any labels
        statement_node->rhs = expression;

        return statement_node;
    }

    infix_parser* parser::infix_parser_for(token_types_t type) {
        auto it = s_infix_parsers.find(type);
        if (it == s_infix_parsers.end())
            return nullptr;
        return it->second;
    }

    prefix_parser* parser::prefix_parser_for(token_types_t type) {
        auto it = s_prefix_parsers.find(type);
        if (it == s_prefix_parsers.end())
            return nullptr;
        return it->second;
    }

    ast_node_shared_ptr parser::parse_expression(common::result& r, uint8_t precedence) {
        token_t token;
        if (!consume(token))
            return nullptr;

        auto prefix_parser = prefix_parser_for(token.type);
        if (prefix_parser == nullptr) {
            error(
                r,
                "B021",
                fmt::format("prefix parser for token '{}' not found.", token.name()),
                token.line,
                token.column);
            return nullptr;
        }

        auto lhs = prefix_parser->parse(r, this, token);
        if (lhs == nullptr) {
            error(
                r,
                "B021",
                "unexpected empty ast node.",
                token.line,
                token.column);
            return nullptr;
        }

        if (token.is_comment())
            return lhs;

        while (precedence < current_infix_precedence()) {
            if (!consume(token)) {
                break;
            }

            auto infix_parser = infix_parser_for(token.type);
            if (infix_parser == nullptr) {
                error(
                    r,
                    "B021",
                    fmt::format("infix parser for token '{}' not found.", token.name()),
                    token.line,
                    token.column);
                break;
            }
            lhs = infix_parser->parse(r, this, lhs, token);
            if (lhs == nullptr || r.is_failed())
                break;
        }

        return lhs;
    }

}