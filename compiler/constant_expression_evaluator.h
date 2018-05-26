// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <parser/ast.h>
#include <common/result.h>
#include "scope.h"

namespace basecode::compiler {

    class constant_expression_evaluator {
    public:
        explicit constant_expression_evaluator(compiler::scope* scope);

        virtual ~constant_expression_evaluator();

        syntax::ast_node_shared_ptr fold_literal_expressions(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

        syntax::ast_node_shared_ptr fold_constant_symbols_and_expressions(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

        syntax::ast_node_shared_ptr fold_constant_functions_and_call_sites(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

    private:
        bool is_subtree_constant(const syntax::ast_node_shared_ptr& node);

    private:
        compiler::scope* _scope;
    };

};

