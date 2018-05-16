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

    private:
        std::stack<ast_node_shared_ptr> _scope_stack {};
    };

};

