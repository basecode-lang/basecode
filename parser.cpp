#include <regex>
#include <iomanip>
#include "parser.h"

namespace basecode {

    operator_dict parser::_operators = {
        {
            "~",
            {
                operator_t::op::invert,
                "~",
                12,
                operator_t::op_type::unary,
                operator_t::associativity_type::right,
                operator_t::op_group::arithmetic
            }
        },
        {
            "`",
            {
                operator_t::op::negate,
                "-",
                11,
                operator_t::op_type::unary,
                operator_t::associativity_type::right,
                operator_t::op_group::arithmetic
            }
        },
        {
            "!",
            {
                operator_t::op::logical_not,
                "!",
                11,
                operator_t::op_type::unary,
                operator_t::associativity_type::right,
                operator_t::op_group::logical
            }
        },
        {
            "^",
            {
                operator_t::op::pow,
                "^",
                10,
                operator_t::op_type::binary,
                operator_t::associativity_type::right,
                operator_t::op_group::arithmetic
            }
        },
        {
            "*",
            {
                operator_t::op::multiply,
                "*",
                9,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::arithmetic
            }
        },
        {
            "/",
            {
                operator_t::op::divide,
                "/",
                9,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::arithmetic
            }
        },
        {
            "\\",
            {
                operator_t::op::modulo,
                "\\",
                9,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::arithmetic
            }
        },
        {
            "+",
            {
                operator_t::op::add,
                "+",
                8,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::arithmetic
            }
        },
        {
            "-",
            {
                operator_t::op::subtract,
                "-",
                8,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::arithmetic
            }
        },
        {
            "&",
            {
                operator_t::op::binary_and,
                "&",
                8,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::arithmetic
            }
        },
        {
            "|",
            {
                operator_t::op::binary_or,
                "|",
                8,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::arithmetic
            }
        },
        {
            ">>",
            {
                operator_t::op::shift_right,
                ">>",
                8,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::arithmetic
            }
        },
        {
            "<<",
            {
                operator_t::op::shift_left,
                "<<",
                8,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::arithmetic
            }
        },
        {
            ">",
            {
                operator_t::op::greater_than,
                ">",
                7,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::relational
            }
        },
        {
            ">=",
            {
                operator_t::op::greater_than_equal,
                ">=",
                7,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::relational
            }
        },
        {
            "<",
            {
                operator_t::op::less_than,
                "<",
                7,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::relational
            }
        },
        {
            "<=",
            {
                operator_t::op::less_than_equal,
                "<=",
                7,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::relational
            }
        },
        {
            "==",
            {
                operator_t::op::equal,
                "==",
                6,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::relational
            }
        },
        {
            "!=",
            {
                operator_t::op::not_equal,
                "!=",
                6,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::relational
            }
        },
        {
            "&&",
            {
                operator_t::op::logical_and,
                "&&",
                5,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::logical
            }
        },
        {
            "||",
            {
                operator_t::op::logical_or,
                "||",
                4,
                operator_t::op_type::binary,
                operator_t::associativity_type::left,
                operator_t::op_group::logical
            }
        },
        {
            "(",
            {
                operator_t::op::left_parenthesis,
                "(",
                1,
                operator_t::op_type::no_op,
                operator_t::associativity_type::none,
                operator_t::op_group::grouping
            }
        },
        {
            ")",
            {
                operator_t::op::right_parenthesis,
                ")",
                1,
                operator_t::op_type::no_op,
                operator_t::associativity_type::none,
                operator_t::op_group::grouping
            }
        },
        {
            "[",
            {
                operator_t::op::left_bracket,
                "[",
                1,
                operator_t::op_type::no_op,
                operator_t::associativity_type::none,
                operator_t::op_group::grouping
            }
        },
        {
            "]",
            {
                operator_t::op::right_bracket,
                "]",
                1,
                operator_t::op_type::no_op,
                operator_t::associativity_type::none,
                operator_t::op_group::grouping
            }
        },
    };

    void parser::error(
            const std::string& code,
            const std::string& message) {
        std::stringstream stream;
        stream << "\n";
        auto start_line = std::max<int32_t>(0, static_cast<int32_t>(_line) - 4);
        auto stop_line = std::min<int32_t>(
                static_cast<int32_t>(_input.source_lines.size()),
                _line + 4);
        for (int32_t i = start_line; i < stop_line; i++) {
            if (i == static_cast<int32_t>(_line - 1)) {
                stream << fmt::format("{:04d}: ", i + 1)
                       << _input.source_lines[i] << "\n"
                       << std::setw(_column + 3) << "|\n"
                       << std::setw(_column + 6) << "+--> " << message << "\n";
            } else {
                stream << fmt::format("{:04d}: ", i + 1)
                       << _input.source_lines[i];
            }

            if (i < static_cast<int32_t>(stop_line - 1))
                stream << "\n";
        }

        _result.add_message(code, stream.str(), true);
    }

    char* parser::set_token() {
        if (_index > _input.length() - 1) {
            _token = nullptr;
        } else {
            _column++;
            _token = &_input.source[_index];
        }
        return _token;
    }

    bool parser::has_operand() {
        return !_operand_stack.empty();
    }

    bool parser::has_operator() {
        return !_operator_stack.empty();
    }

    void parser::clear_stacks() {
        _operator_stack.clear();

        while (!_position_stack.empty())
            _position_stack.pop();

        while (!_operand_stack.empty())
            _operand_stack.pop();
    }

    void parser::pop_position() {
        if (_position_stack.empty())
            return;

        auto pos = _position_stack.top();
        _line = pos.line;
        _index = pos.index;
        _column = pos.column;
        if (_index < _input.length())
            _token = &_input.source[_index];
        else
            _token = nullptr;
        _position_stack.pop();
    }

    void parser::push_position() {
        _position_stack.push(scanner_pos_t {
                _line,
                std::min(_index, _input.length()),
                _column});
    }

    void parser::increment_line() {
        _column = 1;
        _line++;
    }

    char* parser::current_token() {
        if (_token == nullptr) {
            set_token();
            if (_token == nullptr)
                move_to_next_token();
        }
        return _token;
    }

    void parser::register_operator(
            const std::string& key,
            const operator_t& op) {
        _operators.insert(std::make_pair(key, op));
    }

    char* parser::move_to_next_token() {
        _index++;
        return set_token();
    }

    void parser::consume_white_space() {
        auto token = current_token();
        while (token != nullptr && isspace(*token)) {
            if (*token == '\n')
                increment_line();
            token = move_to_next_token();
            if (token == nullptr)
                return;
        }
    }

    size_t parser::forget_top_position() {
        if (_position_stack.empty())
            return _index;
        auto pos = _position_stack.top();
        _position_stack.pop();
        return pos.index;
    }

    void parser::consume_tokens(int count) {
        for (auto i = 0; i < count; ++i) {
            auto token = move_to_next_token();
            if (token == nullptr)
                break;
        }
    }

    ast_node_shared_ptr parser::pop_operand() {
        if (_operand_stack.empty())
            return nullptr;
        auto top = _operand_stack.top();
        _operand_stack.pop();
        return top;
    }

    const basecode::result& parser::result() const {
        return _result;
    }

    ast_node_shared_ptr parser::parse_number() {
        radix_numeric_literal_t number;

        auto token = current_token();
        if (token == nullptr)
            return nullptr;

        push_position();

        if (*token == '$') {
            std::stringstream stream;
            while (true) {
                token = move_to_next_token();
                if (token == nullptr)
                    break;
                if (!ishexnumber(*token))
                    break;
                stream << *token;
            }
            auto value = stream.str();
            if (!value.empty()) {
                number.radix = 16;
                number.value = value;
            }
        } else if (*token == '@') {
            const std::string valid = "012345678";
            std::stringstream stream;
            while (true) {
                token = move_to_next_token();
                if (token == nullptr)
                    break;
                if (valid.find_first_of(*token) == std::string::npos)
                    break;
                stream << *token;
            }
            auto value = stream.str();
            if (!value.empty()) {
                number.radix = 8;
                number.value = value;
            }
        } else if (*token == '%') {
            std::stringstream stream;
            while (true) {
                token = move_to_next_token();
                if (token == nullptr)
                    break;
                if (*token != '0' && *token != '1')
                    break;
                stream << *token;
            }
            auto value = stream.str();
            if (!value.empty()) {
                number.radix = 2;
                number.value = value;
            }
        } else {
            const std::string valid = "0123456789";
            std::stringstream stream;
            while (valid.find_first_of(*token) != std::string::npos) {
                stream << *token;
                token = move_to_next_token();
                if (token == nullptr)
                    break;
            }
            auto value = stream.str();
            if (!value.empty()) {
                number.radix = 10;
                number.value = value;
            }
        }

        if (number.radix != 0) {
            forget_top_position();

            auto number_literal = create_ast_node(ast_node_t::tokens::number_literal);
            number_literal->value = number;
            return number_literal;
        }

        pop_position();

        return nullptr;
    }

    void parser::reset(const parser_input_t& input) {
        _line = 1;
        _index = 0;
        _column = 1;
        _result = {};
        _input = input;
        _token = nullptr;
        clear_stacks();
    }

    operator_t* parser::pop_operator() {
        if (_operator_stack.empty())
            return nullptr;
        auto top = _operator_stack.front();
        _operator_stack.erase(_operator_stack.begin());
        return top;
    }

    operator_t* parser::peek_operator() {
        if (_operator_stack.empty())
            return nullptr;
        return _operator_stack.front();
    }

    operator_t* parser::parse_operator() {
        auto token = current_token();
        if (token == nullptr)
            return nullptr;

        push_position();

        std::vector<operator_t*> candidates;
        for (auto it = _operators.begin(); it != _operators.end(); ++it)
            candidates.push_back(&it->second);

        size_t index = 0;
        operator_t* op = nullptr;
        std::vector<operator_t*> narrowed;
        while (true) {
            if (token == nullptr) {
                error("P008", "unexpected end of operator");
                pop_position();
                return nullptr;
            }

            narrowed = find_matching_operators(candidates, *token, index);
            if (narrowed.empty())
                break;

            token = move_to_next_token();

            if (narrowed.size() == 1 && (index == narrowed.front()->symbol.length() - 1)) {
                op = narrowed.front();
                break;
            } else {
                for (auto x : narrowed) {
                    if (x->symbol.length() - 1 == index)
                        op = x;
                }
            }

            candidates = narrowed;
            index++;
        }

        if (!candidates.empty()
        &&   candidates.size() < _operators.size()) {
            auto top_op = peek_operator();
            auto top_operand = peek_operand();
            auto top_is_binary_op = top_op != nullptr && (top_op->type & operator_t::op_type::binary) != 0;

            for (auto candidate : candidates) {
                if (candidate->symbol == "-") {
                    if (top_operand == nullptr || top_is_binary_op)
                        op = &_operators["`"];
                    else
                        op = &_operators["-"];
                    break;
                }
            }
        }

        if (op != nullptr)
            forget_top_position();
        else
            pop_position();

        return op;
    }

    ast_node_shared_ptr parser::peek_operand() {
        if (_operand_stack.empty())
            return nullptr;

        return _operand_stack.top();
    }

    ast_node_shared_ptr parser::parse_comment() {
        auto token = current_token();

        if (token == nullptr)
            return nullptr;

        push_position();

        if (*token == '/') {
            token = move_to_next_token();
            if (token != nullptr && *token == '/') {
                forget_top_position();

                token = move_to_next_token();
                std::stringstream stream;
                while (token != nullptr && *token != '\n') {
                    stream << *token;
                    token = move_to_next_token();
                }
                auto comment = create_ast_node(ast_node_t::tokens::comment);
                comment->value = comment_t{stream.str()};
                return comment;
            }
        }

        pop_position();

        return nullptr;
    }

    ast_node_shared_ptr parser::parse_identifier() {
        auto token = current_token();
        if (token == nullptr)
            return nullptr;

        push_position();

        std::string s;
        s = *token;
        if (std::regex_match(s, std::regex("[_a-zA-Z]"))) {
            std::stringstream stream;
            stream << *token;
            while (true) {
                token = move_to_next_token();
                if (token == nullptr)
                    break;
                s = *token;
                if (std::regex_match(s, std::regex("[_a-zA-Z0-9]")))
                    stream << *token;
                else
                    break;
            }

            if (token != nullptr && *token == ':') {
                move_to_next_token();
                auto identifier_node = create_ast_node(ast_node_t::tokens::label);
                identifier_node->value = label_t{stream.str()};
                return identifier_node;
            } else {
                auto identifier_node = create_ast_node(ast_node_t::tokens::identifier);
                identifier_node->value = identifier_t{stream.str()};
                return identifier_node;
            }
        } else {
            if (*token == ':') {
                error("P008", "Unexpected colon without valid identifier");
            }
        }

        pop_position();

        return nullptr;
    }

    void parser::push_operator(operator_t* op) {
        _operator_stack.insert(_operator_stack.begin(), op);
    }

    ast_node_shared_ptr parser::parse_expression() {
        while (true) {
        main:
            consume_white_space();

            auto token = current_token();
            if (token == nullptr) {
                break;
            }

            push_position();

            auto op = parse_operator();
            if (_result.is_failed())
                return nullptr;

            if (op != nullptr) {
                if (op->custom_parser != nullptr) {
                    switch (op->type) {
                        case operator_t::op_type::binary: {
                            auto bin_op_node = create_ast_node(ast_node_t::tokens::binary_op);
                            bin_op_node->value = *op;
                            bin_op_node->lhs = pop_operand();
                            bin_op_node->rhs = op->custom_parser();
                            return bin_op_node;
                        }
                        default: {
                            error("P009", "unexpected operator type.");
                            break;
                        }
                    }
                } else if (op->symbol == "(") {
                    auto top = peek_operator();
                    if (top == nullptr && peek_operand() != nullptr) {
                        pop_position();
                        break;
                    }
                    push_operator(op);
                } else if (op->symbol == ")") {
                    while (has_operator()) {
                        op = pop_operator();
                        if (op == &_operators["("]) {
                            auto subexpression = create_ast_node(ast_node_t::tokens::expression);
                            subexpression->children.push_back(pop_operand());
                            push_operand(subexpression);
                            goto main;
                        }
                        auto bin_op_node = create_ast_node(ast_node_t::tokens::binary_op);
                        bin_op_node->value = *op;
                        bin_op_node->rhs = pop_operand();
                        bin_op_node->lhs = pop_operand();
                        push_operand(bin_op_node);
                    }
                    error("P008", "unbalanced right parentheses");
                } else {
                    while (has_operator()) {
                        auto top = peek_operator();
                        if ((op->not_right_associative() && op->compare_precedence(*top) == 0)
                            || (op->compare_precedence(*top) < 0)) {
                            pop_operator();
                            if ((top->type & operator_t::op_type::unary) != 0) {
                                auto unary_op_node = create_ast_node(ast_node_t::tokens::unary_op);
                                unary_op_node->value = *top;
                                unary_op_node->rhs = pop_operand();
                                push_operand(unary_op_node);
                            } else {
                                auto bin_op_node = create_ast_node(ast_node_t::tokens::binary_op);
                                bin_op_node->value = *top;
                                bin_op_node->rhs = pop_operand();
                                bin_op_node->lhs = pop_operand();
                                push_operand(bin_op_node);
                            }
                        } else
                            break;
                    }
                    push_operator(op);
                }
            } else {
                auto top = peek_operator();
                if (top == nullptr && peek_operand() != nullptr) {
                    break;
                }
                if (top != nullptr) {
                    if (((top->type & operator_t::op_type::unary) != 0 && _operand_stack.size() >= 1)
                    ||  ((top->type & operator_t::op_type::binary) != 0 && _operand_stack.size() >= 2)) {
                        if (!operator_stack_has(&_operators["("]))
                            break;
                    }
                }

                for (const auto& terminal : _terminals) {
                    auto node = terminal();
                    if (node != nullptr) {
                        push_operand(node);
                        break;
                    }
                }
            }

            auto start_pos = forget_top_position();
            if (start_pos == _index)
                return nullptr;
        }

        while (has_operator()) {
            auto op = pop_operator();

            if (op == &_operators["("]) {
                error("P008", "unbalanced left parentheses");
            } else if (op == &_operators[")"]) {
                error("P008", "unbalanced right parentheses");
            }

            if ((op->type & operator_t::op_type::unary) != 0) {
                auto unary_op_node = create_ast_node(ast_node_t::tokens::unary_op);
                unary_op_node->value = *op;
                unary_op_node->rhs = pop_operand();
                push_operand(unary_op_node);
            }
            if ((op->type & operator_t::op_type::binary) != 0) {
                auto bin_op_node = create_ast_node(ast_node_t::tokens::binary_op);
                bin_op_node->value = *op;
                bin_op_node->rhs = pop_operand();
                bin_op_node->lhs = pop_operand();
                push_operand(bin_op_node);
            }
        }

        forget_top_position();

        return pop_operand();
    }

    basecode::symbol_table* parser::symbol_table() {
        return _symbol_table;
    }

    ast_node_shared_ptr parser::parse_null_literal() {
        push_position();

        auto token = current_token();
        if (token == nullptr)
            return nullptr;

        if (match_literal("null")) {
            forget_top_position();
            return create_ast_node(ast_node_t::tokens::null_literal);
        }

        pop_position();

        return nullptr;
    }

    ast_node_shared_ptr parser::parse_string_literal() {
        auto token = current_token();
        if (token == nullptr)
            return nullptr;

        std::stringstream stream;
        if (*token == '\"') {
            while (true) {
                token = move_to_next_token();
                if (token == nullptr) {
                    error("P008", "unexpected end of string literal");
                    break;
                }
                if (*token == '\"') {
                    move_to_next_token();
                    break;
                } else if (*token =='\\') {
                    token = move_to_next_token();
                }
                stream << *token;
            }
            auto value = stream.str();
            if (!value.empty()) {
                auto string_literal = create_ast_node(ast_node_t::tokens::string_literal);
                string_literal->value = string_literal_t{value};
                return string_literal;
            }
        }
        return nullptr;
    }

    ast_node_shared_ptr parser::parse_uninitialized() {
        push_position();

        auto token = current_token();
        if (token == nullptr)
            return nullptr;

        if (*token == '?') {
            forget_top_position();
            move_to_next_token();
            return create_ast_node(ast_node_t::tokens::uninitialized_literal);
        }

        pop_position();

        return nullptr;
    }

    ast_node_shared_ptr parser::parse_boolean_literal() {
        push_position();

        auto token = current_token();
        if (token == nullptr)
            return nullptr;

        if (match_literal("true")) {
            forget_top_position();
            auto identifier_node = create_ast_node(ast_node_t::tokens::boolean_literal);
            identifier_node->value = boolean_literal_t {true};
            return identifier_node;
        } else if (match_literal("false")) {
            forget_top_position();
            auto identifier_node = create_ast_node(ast_node_t::tokens::boolean_literal);
            identifier_node->value = boolean_literal_t {false};
            return identifier_node;
        }

        pop_position();

        return nullptr;
    }

    ast_node_shared_ptr parser::parse_assignment() {
        consume_white_space();

        auto token = current_token();
        if (token != nullptr && *token == ':') {
            token = move_to_next_token();
            if (token != nullptr && *token == '=') {
                token = move_to_next_token();
                auto expression_node = parse_expression();
                if (expression_node != nullptr) {
                    auto assignment_node = create_ast_node(ast_node_t::tokens::assignment);
                    assignment_node->rhs = expression_node;
                    return assignment_node;
                } else {

                }
            }
        }

        return nullptr;
    }

    bool parser::operator_stack_has(operator_t* op) {
        return std::find(
                _operator_stack.begin(),
                _operator_stack.end(),
                op) != _operator_stack.end();
    }

    ast_node_shared_ptr parser::parse_semicolon_literal() {
        consume_white_space();

        auto token = current_token();
        if (token == nullptr || *token != ';')
            return nullptr;

        move_to_next_token();

        auto character_literal = create_ast_node(ast_node_t::tokens::character_literal);
        character_literal->value = char_literal_t {';'};
        return character_literal;
    }

    ast_node_shared_ptr parser::parse_character_literal() {
        auto token = current_token();
        if (token != nullptr && *token == '\'') {
            char value;
            token = move_to_next_token();
            if (token != nullptr) {
                value = *token;
                token = move_to_next_token();
                if (token != nullptr && *token == '\'') {
                    move_to_next_token();
                    auto character_literal = create_ast_node(ast_node_t::tokens::character_literal);
                    character_literal->value = char_literal_t {static_cast<unsigned char>(value)};
                    return character_literal;
                } else {
                    error("P008", "unbalanced single quote of character literal");
                }
            } else {
                error("P008", "unexpected end of input");
            }
        }
        return nullptr;
    }

    bool parser::match_literal(const std::string& literal) {
        auto token = current_token();
        for (size_t i = 0; i < literal.length(); ++i) {
            auto c = literal[i];
            if (token == nullptr) {
                error("P008", "Unexpected end of input");
                return false;
            }
            if (c != *token && toupper(c) != *token) {
                return false;
            }
            token = move_to_next_token();
        }
        return true;
    }

    std::vector<operator_t*> parser::find_matching_operators(
            std::vector<operator_t*> candidates,
            char token,
            size_t index) {
        std::vector<operator_t*> matches;
        for (auto it = candidates.begin(); it != candidates.end(); ++it) {
            auto op = *it;
            if (op->precedence == 0)
                continue;
            if (token == op->symbol[index]) {
                matches.push_back(op);
            }
        }
        return matches;
    }

    void parser::symbol_table(basecode::symbol_table* value) {
        _symbol_table = value;
    }

    void parser::push_operand(const ast_node_shared_ptr& node) {
        _operand_stack.push(node);
    }

    ast_node_shared_ptr parser::create_ast_node(ast_node_t::tokens type) {
        auto node = std::make_shared<ast_node_t>();
        node->token = type;
        node->line = _line;
        node->column = _column;
        return node;
    }

    ast_node_shared_ptr parser::parse_expression(const parser_input_t& input) {
        reset(input);
        return parse_expression();
    }

}