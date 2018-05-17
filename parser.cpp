#include <regex>
#include <iomanip>
#include <sstream>
#include "parser.h"

namespace basecode {

    ///////////////////////////////////////////////////////////////////////////

    ast_node_shared_ptr comment_prefix_parser::parse(parser* parse, token_t& token) {
        return nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////

    parser::parser(std::istream& source) : _lexer(source) {
    }

    parser::~parser() {
    }

    ast_node_shared_ptr parser::parse(result& r) {
        auto program_node = _ast_builder.program_node();

        return program_node;
    }

    bool parser::consume(token_t& token) {
        if (!look_ahead(0))
            return false;

        token = _tokens.front();
        _tokens.erase(_tokens.begin());

        return true;
    }

    bool parser::look_ahead(size_t count) {
        while (count >= _tokens.size() && _lexer.has_next()) {
            token_t token;
            if (_lexer.next(token))
                _tokens.push_back(token);
        }
        return !_tokens.empty();
    }

}