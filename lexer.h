#pragma once

#include <istream>
#include <vector>
#include "token.h"

namespace basecode {

    class lexer {
    public:
        using lexer_case_callable = std::function<bool (lexer*, token_t&)>;

        explicit lexer(std::istream& source);

        token_t next();

        bool has_next() const;

    private:
        bool plus(token_t& token);

        bool minus(token_t& token);

        bool asterisk(token_t& token);

        bool attribute(token_t& token);

        bool identifier(token_t& token);

        bool assignment(token_t& token);

        bool line_comment(token_t& token);

        bool null_literal(token_t& token);

        bool true_literal(token_t& token);

        bool false_literal(token_t& token);

        bool number_literal(token_t& token);

        bool line_terminator(token_t& token);

    private:
        char read();

        void mark_position();

        void rewind_one_char();

        void restore_position();

        std::string read_identifier();

        std::string read_until(char target_ch);

    private:
        static std::vector<lexer_case_callable> s_cases;

        uint32_t _line = 0;
        uint32_t _column = 0;
        bool _has_next = true;
        std::istream& _source;
        std::istream::pos_type _mark;

        bool match_literal(const std::string& literal);
    };

};

