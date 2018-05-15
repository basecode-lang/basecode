#pragma once

#include "parser.h"
#include "symbol_table.h"

namespace basecode {

    class alpha_compiler;

    class evaluator {
    public:
        explicit evaluator(alpha_compiler* compiler);

        bool evaluate_program(
            result& result,
            const ast_node_shared_ptr& program_node);

        variant_t evaluate(
            result& result,
            const ast_node_shared_ptr& node);

        basecode::symbol_table* symbol_table();

        void symbol_table(basecode::symbol_table* value);

    protected:
        void error(
            result& result,
            const std::string& code,
            const std::string& message);

    private:
        bool transform_program(
            result& result,
            const ast_node_shared_ptr& node);

        bool transform_identifier_node(
            result& result,
            const ast_node_shared_ptr& node);

    private:
        alpha_compiler* _compiler = nullptr;
        basecode::symbol_table* _symbol_table = nullptr;
    };

};

