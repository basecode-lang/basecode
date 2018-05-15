#pragma once

#include <stack>
#include <vector>
#include <string>
#include "result.h"
#include "parser.h"

namespace basecode {

    class alpha_parser : public parser {
    public:
        alpha_parser();

        virtual ~alpha_parser();

        ast_node_shared_ptr parse(const parser_input_t& input) override;

    protected:
        void parse_program();

    private:
        ast_node_shared_ptr parse_statement();

    private:
        ast_node_shared_ptr pop_scope();

        ast_node_shared_ptr current_scope() const;

        ast_node_shared_ptr push_scope(const ast_node_shared_ptr& node);

    private:
        std::stack<ast_node_shared_ptr> _scope_stack {};
    };

};

