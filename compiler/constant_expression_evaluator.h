#pragma once

#include <parser/ast.h>
#include <common/result.h>
#include "scope.h"

namespace basecode::compiler {

    class constant_expression_evaluator {
    public:
        explicit constant_expression_evaluator(compiler::scope* scope);

        virtual ~constant_expression_evaluator();

        syntax::ast_node_shared_ptr evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

    private:
        compiler::scope* _scope;
    };

};

