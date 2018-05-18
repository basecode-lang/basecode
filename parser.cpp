#include <regex>
#include <iomanip>
#include <sstream>
#include "parser.h"

namespace basecode {

    ast_node_shared_ptr null_literal_prefix_parser::parse(
            result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->null_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr number_literal_prefix_parser::parse(
            result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->number_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr string_literal_prefix_parser::parse(
            result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->string_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr char_literal_prefix_parser::parse(
            result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->character_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr boolean_literal_prefix_parser::parse(
            result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->boolean_literal_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr line_comment_prefix_parser::parse(
            result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->line_comment_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr block_comment_prefix_parser::parse(
            result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->block_comment_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr variable_declaration_prefix_parser::parse(
            result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->variable_declaration_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr variable_reference_infix_parser::parse(
            result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        lhs->rhs = parser->ast_builder()->variable_reference_node(token);
        return lhs;
    }

    precedence_t variable_reference_infix_parser::precedence() const {
        return precedence_t::variable;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr type_identifier_infix_parser::parse(
            result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto is_pointer = false;
        if (parser->peek(token_types_t::asterisk)) {
            parser->consume();
            is_pointer = true;
        }

        token_t type_identifier;
        type_identifier.type = token_types_t::identifier;
        if (!parser->expect(r, type_identifier)) {
            r.add_message(
                "B027",
                "type name expected for variable declaration.",
                true);
            return nullptr;
        }

        lhs->rhs = parser->ast_builder()->type_identifier_node(type_identifier);

        auto is_array = false;
        if (parser->peek(token_types_t::left_square_bracket)) {
            is_array = true;
            auto array_subscript_expression = parser->parse_expression(r, 0);
            lhs->rhs->rhs = array_subscript_expression;
        }

        if (is_pointer)
            lhs->rhs->flags |= ast_node_t::flags_t::pointer;
        else if (is_array)
            lhs->rhs->flags |= ast_node_t::flags_t::array;

        return lhs;
    }

    precedence_t type_identifier_infix_parser::precedence() const {
        return precedence_t::type;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr assignment_infix_parser::parse(
            result& r,
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) {
        auto assignment_node = parser->ast_builder()->assignment_node();
        assignment_node->lhs = lhs;
        assignment_node->rhs = parser->parse_expression(r, 0);
        return assignment_node;
    }

    precedence_t assignment_infix_parser::precedence() const {
        return precedence_t::assignment;
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr attribute_prefix_parser::parse(
            result& r,
            parser* parser,
            token_t& token) {
        return parser->ast_builder()->attribute_node(token);
    }

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr array_subscript_prefix_parser::parse(
            result& r,
            parser* parser,
            token_t& token) {
        auto expression = parser->parse_expression(r, 0);
        if (expression != nullptr) {
            token_t right_bracket_token;
            right_bracket_token.type = token_types_t::right_square_bracket;
            if (!parser->expect(r, right_bracket_token))
                return nullptr;
        }
        return expression;
    }

    ///////////////////////////////////////////////////////////////////////////

    parser::parser(std::istream& source) : _lexer(source) {
    }

    parser::~parser() {
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

        return true;
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

    basecode::ast_builder* parser::ast_builder() {
        return &_ast_builder;
    }

    ast_node_shared_ptr parser::parse(result& r) {
        return parse_scope(r);
    }

    bool parser::expect(result& r, token_t& token) {
        if (!look_ahead(0))
            return false;

        std::string expected_name = token.name();
        auto expected_type = token.type;
        token = _tokens.front();
        if (token.type != expected_type) {
            r.add_message(
                "B016",
                fmt::format("expected token '{}' but found '{}'.", expected_name, token.name()),
                true);
            return false;
        }

        _tokens.erase(_tokens.begin());

        return true;
    }

    ast_node_shared_ptr parser::parse_scope(result& r) {
        auto scope = _ast_builder.begin_scope();

        while (true) {
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

    ast_node_shared_ptr parser::parse_statement(result& r) {
        auto expression = parse_expression(r, 0);
        if (expression == nullptr) {
            r.add_message(
                "B031",
                "unexpected end of input",
                true);
            return nullptr;
        }
        if (expression->type == ast_node_types_t::line_comment
        ||  expression->type == ast_node_types_t::block_comment) {
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

    ast_node_shared_ptr parser::parse_expression(result& r, uint8_t precedence) {
        token_t token;
        if (!consume(token))
            return nullptr;

        auto prefix_parser = prefix_parser_for(token.type);
        if (prefix_parser == nullptr) {
            r.add_message(
                "B021",
                fmt::format("prefix parser for token '{}' not found.", token.name()),
                true);
            return nullptr;
        }

        auto lhs = prefix_parser->parse(r, this, token);
        if (token.is_comment())
            return lhs;

        while (precedence < current_infix_precedence()) {
            if (!consume(token)) {
                // XXX: this bad ***mmmmkay***
                break;
            }

            auto infix_parser = infix_parser_for(token.type);
            if (infix_parser == nullptr) {
                r.add_message(
                    "B021",
                    fmt::format("infix parser for token '{}' not found.", token.name()),
                    true);
                break;
            }
            lhs = infix_parser->parse(r, this, lhs, token);
            if (lhs == nullptr || r.is_failed())
                break;
        }

        return lhs;
    }

}