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
        call
    };

    class parser;

    ///////////////////////////////////////////////////////////////////////////

    class prefix_parser {
    public:
        virtual ~prefix_parser() = default;

        virtual ast_node_shared_ptr parse(
            parser* parser,
            token_t& token) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////

    class infix_parser {
    public:
        virtual ~infix_parser() = default;

        virtual ast_node_shared_ptr parse(
            parser* parser,
            const ast_node_shared_ptr& lhs,
            token_t& token) = 0;

        virtual precedence_t precedence() const = 0;
    };

    ///////////////////////////////////////////////////////////////////////////

    class comment_prefix_parser : public prefix_parser {
    public:
        comment_prefix_parser() = default;

        ast_node_shared_ptr parse(
            parser* parse,
            token_t& token) override;
    };

    ///////////////////////////////////////////////////////////////////////////

    class parser {
    public:
        explicit parser(std::istream& source);

        virtual ~parser();

        ast_node_shared_ptr parse(result& r);

    protected:
        bool consume(token_t& token);

        bool look_ahead(size_t count);

    private:
        basecode::lexer _lexer;
        basecode::result _result;
        ast_builder _ast_builder;
        std::vector<token_t> _tokens {};
    };

};