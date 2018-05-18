#include <sstream>
#include "lexer.h"

namespace basecode {

    std::multimap<char, lexer::lexer_case_callable> lexer::s_cases = {
        // attribute
        {'@', std::bind(&lexer::attribute, std::placeholders::_1, std::placeholders::_2)},

        // add
        {'+', std::bind(&lexer::plus, std::placeholders::_1, std::placeholders::_2)},

        // minus, negate
        {'-', std::bind(&lexer::minus, std::placeholders::_1, std::placeholders::_2)},

        // line comment, slash
        {'/', std::bind(&lexer::line_comment, std::placeholders::_1, std::placeholders::_2)},
        {'/', std::bind(&lexer::slash, std::placeholders::_1, std::placeholders::_2)},

        // comma
        {',', std::bind(&lexer::comma, std::placeholders::_1, std::placeholders::_2)},

        // caret
        {'^', std::bind(&lexer::caret, std::placeholders::_1, std::placeholders::_2)},

        // not equals, bang
        {'!', std::bind(&lexer::not_equals_operator, std::placeholders::_1, std::placeholders::_2)},
        {'!', std::bind(&lexer::bang, std::placeholders::_1, std::placeholders::_2)},

        // question
        {'?', std::bind(&lexer::question, std::placeholders::_1, std::placeholders::_2)},

        // spread
        {'.', std::bind(&lexer::spread, std::placeholders::_1, std::placeholders::_2)},

        // tilde
        {'~', std::bind(&lexer::tilde, std::placeholders::_1, std::placeholders::_2)},

        // assignment, scope operator, colon
        {':', std::bind(&lexer::assignment, std::placeholders::_1, std::placeholders::_2)},
        {':', std::bind(&lexer::scope_operator, std::placeholders::_1, std::placeholders::_2)},
        {':', std::bind(&lexer::colon, std::placeholders::_1, std::placeholders::_2)},

        // percent/number literal
        {'%', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'%', std::bind(&lexer::percent, std::placeholders::_1, std::placeholders::_2)},

        // asterisk
        {'*', std::bind(&lexer::asterisk, std::placeholders::_1, std::placeholders::_2)},

        // equals
        {'=', std::bind(&lexer::equals_operator, std::placeholders::_1, std::placeholders::_2)},

        // less than equal, less than
        {'<', std::bind(&lexer::less_than_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'<', std::bind(&lexer::less_than_operator, std::placeholders::_1, std::placeholders::_2)},

        // greater than equal, greater than
        {'>', std::bind(&lexer::greater_than_equal_operator, std::placeholders::_1, std::placeholders::_2)},
        {'>', std::bind(&lexer::greater_than_operator, std::placeholders::_1, std::placeholders::_2)},

        // logical and, bitwise and, ampersand
        {'&', std::bind(&lexer::logical_and_operator, std::placeholders::_1, std::placeholders::_2)},
        {'&', std::bind(&lexer::ampersand_literal, std::placeholders::_1, std::placeholders::_2)},

        // logical or, bitwise or, pipe
        {'|', std::bind(&lexer::logical_or_operator, std::placeholders::_1, std::placeholders::_2)},
        {'|', std::bind(&lexer::pipe_literal, std::placeholders::_1, std::placeholders::_2)},

        // braces
        {'{', std::bind(&lexer::left_curly_brace, std::placeholders::_1, std::placeholders::_2)},
        {'}', std::bind(&lexer::right_curly_brace, std::placeholders::_1, std::placeholders::_2)},

        // parens
        {'(', std::bind(&lexer::left_paren, std::placeholders::_1, std::placeholders::_2)},
        {')', std::bind(&lexer::right_paren, std::placeholders::_1, std::placeholders::_2)},

        // square brackets
        {'[', std::bind(&lexer::left_square_bracket, std::placeholders::_1, std::placeholders::_2)},
        {']', std::bind(&lexer::right_square_bracket, std::placeholders::_1, std::placeholders::_2)},

        // line terminator
        {';', std::bind(&lexer::line_terminator, std::placeholders::_1, std::placeholders::_2)},

        // character literal
        {'\'', std::bind(&lexer::character_literal, std::placeholders::_1, std::placeholders::_2)},

        // string literal
        {'"', std::bind(&lexer::string_literal, std::placeholders::_1, std::placeholders::_2)},

        // true/false literals
        {'t', std::bind(&lexer::true_literal, std::placeholders::_1, std::placeholders::_2)},
        {'f', std::bind(&lexer::false_literal, std::placeholders::_1, std::placeholders::_2)},

        // null/none/ns/empty literals
        {'n', std::bind(&lexer::null_literal, std::placeholders::_1, std::placeholders::_2)},
        {'n', std::bind(&lexer::none_literal, std::placeholders::_1, std::placeholders::_2)},
        {'n', std::bind(&lexer::ns_literal, std::placeholders::_1, std::placeholders::_2)},
        {'e', std::bind(&lexer::empty_literal, std::placeholders::_1, std::placeholders::_2)},

        // if/else if/else literals
        {'i', std::bind(&lexer::if_literal, std::placeholders::_1, std::placeholders::_2)},
        {'e', std::bind(&lexer::else_if_literal, std::placeholders::_1, std::placeholders::_2)},
        {'e', std::bind(&lexer::else_literal, std::placeholders::_1, std::placeholders::_2)},

        // fn literal
        {'f', std::bind(&lexer::fn_literal, std::placeholders::_1, std::placeholders::_2)},

        // in literal
        {'i', std::bind(&lexer::in_literal, std::placeholders::_1, std::placeholders::_2)},

        // for literal
        {'f', std::bind(&lexer::for_literal, std::placeholders::_1, std::placeholders::_2)},

        // break literal
        {'b', std::bind(&lexer::break_literal, std::placeholders::_1, std::placeholders::_2)},

        // cast literal
        {'c', std::bind(&lexer::cast_literal, std::placeholders::_1, std::placeholders::_2)},

        // defer literal
        {'d', std::bind(&lexer::defer_literal, std::placeholders::_1, std::placeholders::_2)},

        // continue literal
        {'c', std::bind(&lexer::continue_literal, std::placeholders::_1, std::placeholders::_2)},

        // alias literal
        {'a', std::bind(&lexer::alias_literal, std::placeholders::_1, std::placeholders::_2)},

        // extend literal
        {'e', std::bind(&lexer::extend_literal, std::placeholders::_1, std::placeholders::_2)},

        // read_only literal
        {'r', std::bind(&lexer::read_only_literal, std::placeholders::_1, std::placeholders::_2)},

        // while literal
        {'w', std::bind(&lexer::while_literal, std::placeholders::_1, std::placeholders::_2)},

        // identifier
        {'_', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'a', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'b', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'c', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'d', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'e', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'f', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'g', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'h', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'i', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'j', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'k', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'l', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'m', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'n', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'o', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'p', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'q', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'r', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'s', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'t', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'u', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'v', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'w', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'x', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'y', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},
        {'z', std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2)},

        // number literal
        {'_', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'$', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'%', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'@', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'0', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'1', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'2', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'3', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'4', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'5', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'6', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'7', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'8', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
        {'9', std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2)},
    };

    lexer::lexer(std::istream& source) : _source(source) {
        _source.seekg(0, std::istream::seekdir::beg);
    }

    char lexer::peek() {
        while (!_source.eof()) {
            auto ch = static_cast<char>(_source.get());
            if (!isspace(ch))
                return ch;
        }
        return 0;
    }

    void lexer::mark_position() {
        _mark = _source.tellg();
    }

    void lexer::increment_line() {
        auto pos = _source.tellg();
        if (_line_breaks.count(pos) == 0) {
            _line++;
            _column = 0;
            _line_breaks.insert(pos);
        }
    }

    bool lexer::has_next() const {
        return _has_next;
    }

    void lexer::rewind_one_char() {
        _source.seekg(-1, std::istream::cur);
        if (_column > 0)
            _column--;
    }

    void lexer::restore_position() {
        _source.seekg(_mark);
    }

    bool lexer::next(token_t& token) {
        if (_source.eof()) {
            _has_next = false;
            token.line = _line;
            token.column = _column;
            token.type = token_types_t::end_of_file;
            return false;
        }

        const auto ch = static_cast<char>(tolower(read()));
        rewind_one_char();
        mark_position();

        auto case_range = s_cases.equal_range(ch);
        for (auto it = case_range.first; it != case_range.second; ++it) {
            token.radix = 10;
            token.line = _line;
            token.column = _column;
            token.number_type = number_types_t::none;
            if (it->second(this, token))
                return true;
            restore_position();
        }

        token.type = token_types_t::end_of_file;
        token.line = _line;
        token.column = _column;

        _has_next = false;

        return false;
    }

    char lexer::read(bool skip_whitespace) {
        while (true) {
            auto ch = static_cast<char>(_source.get());

            _column++;

            if (ch == '\n')
                increment_line();

            if (skip_whitespace && isspace(ch))
                continue;

            return ch;
        }
    }

    std::string lexer::read_identifier() {
        auto ch = read();
        if (ch != '_' && !isalpha(ch)) {
            return "";
        }
        _column++;
        std::stringstream stream;
        stream << ch;
        while (true) {
            ch = static_cast<char>(_source.get());
            _column++;
            if (ch == '_' || isalnum(ch)) {
                stream << ch;
            } else {
                return stream.str();
            }
        }
    }

    bool lexer::alias_literal(token_t& token) {
        if (match_literal("alias")) {
            token.type = token_types_t::alias_literal;
            token.value = "alias";
            return true;
        }
        return false;
    }

    bool lexer::break_literal(token_t& token) {
        if (match_literal("break")) {
            token.type = token_types_t::break_literal;
            token.value = "break";
            return true;
        }
        return false;
    }

    bool lexer::while_literal(token_t& token) {
        if (match_literal("while")) {
            token.type = token_types_t::while_literal;
            token.value = "while";
            return true;
        }
        return false;
    }

    bool lexer::extend_literal(token_t& token) {
        if (match_literal("extend")) {
            token.type = token_types_t::extend_literal;
            token.value = "extend";
            return true;
        }
        return false;
    }

    bool lexer::continue_literal(token_t& token) {
        if (match_literal("continue")) {
            token.type = token_types_t::continue_literal;
            token.value = "continue";
            return true;
        }
        return false;
    }

    bool lexer::left_curly_brace(token_t& token) {
        auto ch = read();
        if (ch == '{') {
            token.type = token_types_t::left_curly_brace;
            token.value = "{";
            return true;
        }
        return false;
    }

    bool lexer::right_curly_brace(token_t& token) {
        auto ch = read();
        if (ch == '}') {
            token.type = token_types_t::right_curly_brace;
            token.value = "}";
            return true;
        }
        return false;
    }

    std::string lexer::read_until(char target_ch) {
        std::stringstream stream;
        while (true) {
            auto ch = static_cast<char>(_source.get());
            _column++;
            if (ch == '\n')
                increment_line();
            if (ch == target_ch)
                break;
            stream << ch;
        }
        return stream.str();
    }

    bool lexer::plus(token_t& token) {
        auto ch = read();
        if (ch == '+') {
            token.type = token_types_t::plus;
            token.value = "+";
            return true;
        }
        return false;
    }

    bool lexer::bang(token_t& token) {
        auto ch = read();
        if (ch == '!') {
            token.type = token_types_t::bang;
            token.value = "!";
            return true;
        }
        return false;
    }

    bool lexer::caret(token_t& token) {
        auto ch = read();
        if (ch == '^') {
            token.type = token_types_t::caret;
            token.value = "^";
            return true;
        }
        return false;
    }

    bool lexer::tilde(token_t& token) {
        auto ch = read();
        if (ch == '~') {
            token.type = token_types_t::tilde;
            token.value = "~";
            return true;
        }
        return false;
    }

    bool lexer::colon(token_t& token) {
        auto ch = read();
        if (ch == ':') {
            token.type = token_types_t::colon;
            token.value = ":";
            return true;
        }
        return false;
    }

    bool lexer::minus(token_t& token) {
        auto ch = read();
        if (ch == '-') {
            token.type = token_types_t::minus;
            token.value = "-";
            return true;
        }
        return false;
    }

    bool lexer::comma(token_t& token) {
        auto ch = read();
        if (ch == ',') {
            token.type = token_types_t::comma;
            token.value = ",";
            return true;
        }
        return false;
    }

    bool lexer::slash(token_t& token) {
        auto ch = read();
        if (ch == '/') {
            token.type = token_types_t::slash;
            token.value = "/";
            return true;
        }
        return false;
    }

    bool lexer::spread(token_t& token) {
        if (match_literal("...")) {
            token.type = token_types_t::spread_operator;
            token.value = "...";
            return true;
        }
        return false;
    }

    bool lexer::percent(token_t& token) {
        auto ch = read();
        if (ch == '%') {
            token.type = token_types_t::percent;
            token.value = "%";
            return true;
        }
        return false;
    }

    bool lexer::question(token_t& token) {
        auto ch = read();
        if (ch == '?') {
            token.type = token_types_t::question;
            token.value = "?";
            return true;
        }
        return false;
    }

    bool lexer::asterisk(token_t& token) {
        auto ch = read();
        if (ch == '*') {
            token.type = token_types_t::asterisk;
            token.value = "*";
            return true;
        }
        return false;
    }

    bool lexer::attribute(token_t& token) {
        auto ch = read();
        if (ch == '@') {
            token.value = read_identifier();
            if (token.value.empty())
                return false;
            token.type = token_types_t::attribute;
            return true;
        }
        return false;
    }

    bool lexer::identifier(token_t& token) {
        auto name = read_identifier();
        if (name.empty())
            return false;
        token.type = token_types_t::identifier;
        token.value = name;
        rewind_one_char();
        return true;
    }

    bool lexer::assignment(token_t& token) {
        auto ch = read();
        if (ch == ':') {
            ch = read();
            if (ch == '=') {
                token.type = token_types_t::assignment;
                token.value = ":=";
                return true;
            }
        }
        return false;
    }

    bool lexer::left_paren(token_t& token) {
        auto ch = read();
        if (ch == '(') {
            token.type = token_types_t::left_paren;
            token.value = "(";
            return true;
        }
        return false;
    }

    bool lexer::in_literal(token_t& token) {
        auto ch = read();
        if (ch == 'i') {
            ch = read();
            if (ch == 'n') {
                token.type = token_types_t::in_literal;
                token.value = "in";
                return true;
            }
        }
        return false;
    }

    bool lexer::right_paren(token_t& token) {
        auto ch = read();
        if (ch == ')') {
            token.type = token_types_t::right_paren;
            token.value = ")";
            return true;
        }
        return false;
    }

    bool lexer::fn_literal(token_t& token) {
        auto ch = read();
        if (ch == 'f') {
            ch = read();
            if (ch == 'n') {
                token.type = token_types_t::fn_literal;
                token.value = "fn";
                return true;
            }
        }
        return false;
    }

    bool lexer::ns_literal(token_t& token) {
        auto ch = read();
        if (ch == 'n') {
            ch = read();
            if (ch == 's') {
                token.type = token_types_t::namespace_literal;
                token.value = "ns";
                return true;
            }
        }
        return false;
    }

    bool lexer::if_literal(token_t& token) {
        auto ch = read();
        if (ch == 'i') {
            ch = read();
            if (ch == 'f') {
                token.type = token_types_t::if_literal;
                token.value = "if";
                return true;
            }
        }
        return false;
    }

    bool lexer::else_literal(token_t& token) {
        if (match_literal("else")) {
            token.type = token_types_t::else_literal;
            token.value = "else";
            return true;
        }
        return false;
    }

    bool lexer::line_comment(token_t& token) {
        auto ch = read();
        if (ch == '/') {
            ch = read();
            if (ch == '/') {
                token.type = token_types_t::line_comment;
                token.value = read_until('\n');
                rewind_one_char();
                return true;
            }
        }
        return false;
    }

    bool lexer::for_literal(token_t& token) {
        if (match_literal("for")) {
            token.type = token_types_t::for_literal;
            token.value = "for";
            return true;
        }
        return false;
    }

    bool lexer::null_literal(token_t& token) {
        if (match_literal("null")) {
            token.type = token_types_t::null_literal;
            token.value = "null";
            return true;
        }
        return false;
    }

    bool lexer::none_literal(token_t& token) {
        if (match_literal("none")) {
            token.type = token_types_t::none_literal;
            token.value = "none";
            return true;
        }
        return false;
    }

    bool lexer::cast_literal(token_t& token) {
        if (match_literal("cast")) {
            token.type = token_types_t::cast_literal;
            token.value = "cast";
            return true;
        }
        return false;
    }

    bool lexer::true_literal(token_t& token) {
        if (match_literal("true")) {
            token.type = token_types_t::true_literal;
            token.value = "true";
            return true;
        }
        return false;
    }

    bool lexer::pipe_literal(token_t& token) {
        auto ch = read();
        if (ch == '|') {
            token.type = token_types_t::pipe;
            token.value = "|";
            return true;
        }
        return false;
    }

    bool lexer::string_literal(token_t& token) {
        auto ch = read();
        if (ch == '\"') {
            token.type = token_types_t::string_literal;
            token.value = read_until('"');
            return true;
        }
        return false;
    }

    bool lexer::false_literal(token_t& token) {
        if (match_literal("false")) {
            token.type = token_types_t::false_literal;
            token.value = "false";
            return true;
        }
        return false;
    }

    bool lexer::defer_literal(token_t& token) {
        if (match_literal("defer")) {
            token.type = token_types_t::defer_literal;
            token.value = "defer";
            return true;
        }
        return false;
    }

    bool lexer::empty_literal(token_t& token) {
        if (match_literal("empty")) {
            token.type = token_types_t::empty_literal;
            token.value = "empty";
            return true;
        }
        return false;
    }

    bool lexer::number_literal(token_t& token) {
        std::stringstream stream;
        token.type = token_types_t::number_literal;
        token.number_type = number_types_t::integer;

        auto ch = read();
        if (ch == '$') {
            token.radix = 16;
            while (true) {
                ch = read();
                if (ch == '_')
                    continue;
                if (!ishexnumber(ch))
                    break;
                stream << ch;
            }
        } else if (ch == '@') {
            const std::string valid = "012345678";
            token.radix = 8;
            while (true) {
                ch = read();
                if (ch == '_')
                    continue;
                if (valid.find_first_of(ch) == std::string::npos)
                    break;
                stream << ch;
            }
        } else if (ch == '%') {
            token.radix = 2;
            while (true) {
                ch = read();
                if (ch == '_')
                    continue;
                if (ch != '0' && ch != '1')
                    break;
                stream << ch;
            }
        } else {
            const std::string valid = "0123456789_.";
            while (valid.find_first_of(ch) != std::string::npos) {
                if (ch != '_') {
                    if (ch == '.')
                        token.number_type = number_types_t::floating_point;
                    stream << ch;
                }
                ch = read();
            }
        }

        token.value = stream.str();
        if (token.value.empty())
            return false;

        rewind_one_char();

        return true;
    }

    bool lexer::scope_operator(token_t& token) {
        auto ch = read();
        if (ch == ':') {
            ch = read();
            if (ch == ':') {
                token.type = token_types_t::scope_operator;
                token.value = "::";
                return true;
            }
        }
        return false;
    }

    bool lexer::line_terminator(token_t& token) {
        auto ch = read();
        if (ch == ';') {
            token.type = token_types_t::semi_colon;
            token.value = ";";
            return true;
        }
        return false;
    }

    bool lexer::equals_operator(token_t& token) {
        auto ch = read();
        if (ch == '=') {
            ch = read();
            if (ch == '=') {
                token.type = token_types_t::equals;
                token.value = "==";
                return true;
            }
        }
        return false;
    }

    bool lexer::else_if_literal(token_t& token) {
        if (match_literal("else if")) {
            token.type = token_types_t::else_if_literal;
            token.value = "else if";
            return true;
        }
        return false;
    }

    bool lexer::character_literal(token_t& token) {
        auto ch = read();
        if (ch == '\'') {
            auto value = read();
            ch = read();
            if (ch == '\'') {
                token.type = token_types_t::character_literal;
                token.value = value;
                return true;
            }
        }
        return false;
    }

    bool lexer::read_only_literal(token_t& token) {
        if (match_literal("read_only")) {
            token.type = token_types_t::read_only_literal;
            token.value = "read_only";
            return true;
        }
        return false;
    }

    bool lexer::ampersand_literal(token_t& token) {
        auto ch = read();
        if (ch == '&') {
            token.type = token_types_t::ampersand;
            token.value = "&";
            return true;
        }
        return false;
    }

    bool lexer::less_than_operator(token_t& token) {
        auto ch = read();
        if (ch == '<') {
            token.type = token_types_t::less_than;
            token.value = "<";
            return true;
        }
        return false;
    }

    bool lexer::logical_or_operator(token_t& token) {
        auto ch = read();
        if (ch == '|') {
            ch = read();
            if (ch == '|') {
                token.type = token_types_t::logical_or;
                token.value = "||";
                return true;
            }
        }
        return false;
    }

    bool lexer::logical_and_operator(token_t& token) {
        auto ch = read();
        if (ch == '&') {
            ch = read();
            if (ch == '&') {
                token.type = token_types_t::logical_and;
                token.value = "&&";
                return true;
            }
        }
        return false;
    }

    bool lexer::not_equals_operator(token_t& token) {
        auto ch = read();
        if (ch == '!') {
            ch = read();
            if (ch == '=') {
                token.type = token_types_t::not_equals;
                token.value = "!=";
                return true;
            }
        }
        return false;
    }

    bool lexer::left_square_bracket(token_t& token) {
        auto ch = read();
        if (ch == '[') {
            token.type = token_types_t::left_square_bracket;
            token.value = "[";
            return true;
        }
        return false;
    }

    bool lexer::right_square_bracket(token_t& token) {
        auto ch = read();
        if (ch == ']') {
            token.type = token_types_t::right_square_bracket;
            token.value = "]";
            return true;
        }
        return false;
    }

    bool lexer::greater_than_operator(token_t& token) {
        auto ch = read();
        if (ch == '>') {
            token.type = token_types_t::greater_than;
            token.value = ">";
            return true;
        }
        return false;
    }

    bool lexer::less_than_equal_operator(token_t& token) {
        auto ch = read();
        if (ch == '<') {
            ch = read();
            if (ch == '=') {
                token.type = token_types_t::less_than_equal;
                token.value = "<=";
                return true;
            }
        }
        return false;
    }

    bool lexer::match_literal(const std::string& literal) {
        auto ch = read();
        for (size_t i = 0; i < literal.length(); ++i) {
            const auto& target_ch = literal[i];
            if (target_ch != ch)
                return false;
            ch = read(false);
        }
        rewind_one_char();
        return true;
    }

    bool lexer::greater_than_equal_operator(token_t& token) {
        auto ch = read();
        if (ch == '>') {
            ch = read();
            if (ch == '=') {
                token.type = token_types_t::greater_than_equal;
                token.value = ">=";
                return true;
            }
        }
        return false;
    }

};