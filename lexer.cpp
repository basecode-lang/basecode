#include <sstream>
#include <regex>
#include "lexer.h"

namespace basecode {

    std::vector<lexer::lexer_case_callable> lexer::s_cases = {
        std::bind(&lexer::attribute, std::placeholders::_1, std::placeholders::_2),
        std::bind(&lexer::line_terminator, std::placeholders::_1, std::placeholders::_2),
        std::bind(&lexer::assignment, std::placeholders::_1, std::placeholders::_2),
        std::bind(&lexer::true_literal, std::placeholders::_1, std::placeholders::_2),
        std::bind(&lexer::false_literal, std::placeholders::_1, std::placeholders::_2),
        std::bind(&lexer::null_literal, std::placeholders::_1, std::placeholders::_2),
        std::bind(&lexer::line_comment, std::placeholders::_1, std::placeholders::_2),
        std::bind(&lexer::identifier, std::placeholders::_1, std::placeholders::_2),
        std::bind(&lexer::number_literal, std::placeholders::_1, std::placeholders::_2),
    };

    lexer::lexer(std::istream& source) : _source(source) {
        _source.seekg(0, std::istream::seekdir::beg);
    }

    char lexer::read() {
        while (true) {
            auto ch = static_cast<char>(_source.get());
            if (!isspace(ch))
                return ch;
            _column++;
            if (ch == '\n') {
                _line++;
                _column = 0;
            }
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

    std::string lexer::read_until(char target_ch) {
        std::stringstream stream;
        while (true) {
            auto ch = static_cast<char>(_source.get());
            _column++;
            if (ch == target_ch) {
                if (ch == '\n') {
                    _line++;
                    _column = 0;
                }
                break;
            }
            stream << ch;
        }
        return stream.str();
    }

    token_t lexer::next() {
        if (_source.eof()) {
            _has_next = false;
            return token_t {
                .type = token_types_t::end_of_file,
                .line = _line,
                .column = _column,
            };
        }

        token_t token = {
            .type = token_types_t::unknown,
            .line = _line,
            .value = "",
            .radix = 10,
            .column = _column,
        };

        mark_position();
        for (const auto& lexer_case : s_cases) {
            if (lexer_case(this, token)) {
                return token;
            }
            restore_position();
        }

        _has_next = false;

        return token;
    }

    void lexer::mark_position() {
        _mark = _source.tellg();
    }

    bool lexer::has_next() const {
        return _has_next;
    }

    void lexer::rewind_one_char() {
        _source.seekg(-1, std::istream::cur);
        _column--;
    }

    void lexer::restore_position() {
        _source.seekg(_mark);
    }

    bool lexer::plus(token_t& token) {
        return false;
    }

    bool lexer::minus(token_t& token) {
        return false;
    }

    bool lexer::asterisk(token_t& token) {
        return false;
    }

    bool lexer::attribute(token_t& token) {
        token.line = _line;
        token.column = _column;
        auto ch = read();
        if (ch == '@') {
            token.type = token_types_t::attribute;
            token.value = read_identifier();
            return true;
        }
        return false;
    }

    bool lexer::identifier(token_t& token) {
        token.line = _line;
        token.column = _column;
        auto name = read_identifier();
        if (name.empty())
            return false;
        token.type = token_types_t::identifier;
        token.value = name;
        rewind_one_char();
        return true;
    }

    bool lexer::assignment(token_t& token) {
        token.line = _line;
        token.column = _column;
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

    bool lexer::line_comment(token_t& token) {
        token.line = _line;
        token.column = _column;
        auto ch = read();
        if (ch == '/') {
            ch = read();
            if (ch == '/') {
                token.type = token_types_t::line_comment;
                token.value = read_until('\n');
                return true;
            }
        }
        return false;
    }

    bool lexer::null_literal(token_t& token) {
        token.line = _line;
        token.column = _column;
        if (match_literal("null")) {
            token.type = token_types_t::null_literal;
            token.value = "null";
            return true;
        }
        return false;
    }

    bool lexer::true_literal(token_t& token) {
        token.line = _line;
        token.column = _column;
        if (match_literal("true")) {
            token.type = token_types_t::true_literal;
            token.value = "true";
            return true;
        }
        return false;
    }

    bool lexer::false_literal(token_t& token) {
        token.line = _line;
        token.column = _column;
        if (match_literal("false")) {
            token.type = token_types_t::false_literal;
            token.value = "false";
            return true;
        }
        return false;
    }

    bool lexer::number_literal(token_t& token) {
        return false;
    }

    bool lexer::line_terminator(token_t& token) {
        token.line = _line;
        token.column = _column;
        auto ch = read();
        if (ch == ';') {
            token.type = token_types_t::semi_colon;
            token.value = ";";
            return true;
        }
        return false;
    }

    bool lexer::match_literal(const std::string& literal) {
        auto ch = read();
        for (size_t i = 0; i < literal.length(); ++i) {
            auto target_ch = literal[i];
            if (target_ch != ch)
                return false;
            ch = read();
        }
        return true;
    }

};