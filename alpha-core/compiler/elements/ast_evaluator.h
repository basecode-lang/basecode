// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <functional>
#include <unordered_map>
#include "element_builder.h"

namespace basecode::compiler {

    struct evaluator_context_t {
        evaluator_context_t(
                common::result& r,
                compiler::session& session) : r(r),
                                              session(session) {
        }

        common::result& r;
        compiler::session& session;
        compiler::block* scope = nullptr;
        const syntax::ast_node_t* node = nullptr;
        element_type_t default_block_type = element_type_t::block;
    };

    struct evaluator_result_t {
        compiler::element* element = nullptr;
    };

    class ast_evaluator;

    using node_evaluator_callable = std::function<bool (
        ast_evaluator*,
        evaluator_context_t&,
        evaluator_result_t&)>;

    class ast_evaluator {
    public:
        ast_evaluator(
            element_builder* builder,
            compiler::program* program);

        element* evaluate(
            common::result& r,
            compiler::session& session,
            const syntax::ast_node_t* node,
            element_type_t default_block_type = element_type_t::block);

        element* evaluate_in_scope(
            common::result& r,
            compiler::session& session,
            const syntax::ast_node_t* node,
            compiler::block* scope,
            element_type_t default_block_type = element_type_t::block);

    private:
        void apply_attributes(
            common::result& r,
            compiler::session& session,
            compiler::element* element,
            const syntax::ast_node_t* node);

        void add_procedure_instance(
            common::result& r,
            compiler::session& session,
            compiler::procedure_type* proc_type,
            const syntax::ast_node_t* node);

        void add_expression_to_scope(
            compiler::block* scope,
            compiler::element* expr);

        void add_composite_type_fields(
            common::result& r,
            compiler::session& session,
            compiler::composite_type* type,
            const syntax::ast_node_t* block);

        compiler::block* add_namespaces_to_scope(
            common::result& r,
            compiler::session& session,
            const syntax::ast_node_t* node,
            compiler::symbol_element* symbol,
            compiler::block* parent_scope);

        compiler::identifier* add_identifier_to_scope(
            common::result& r,
            compiler::session& session,
            compiler::symbol_element* symbol,
            type_find_result_t& find_type_result,
            const syntax::ast_node_t* node,
            size_t source_index,
            compiler::block* parent_scope = nullptr);

        compiler::element* resolve_symbol_or_evaluate(
            const evaluator_context_t& context,
            const syntax::ast_node_t* node);

    private:
        bool noop(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool symbol(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool attribute(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool directive(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool module(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool module_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool line_comment(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool block_comment(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool string_literal(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool number_literal(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool boolean_literal(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool namespace_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool argument_list(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool unary_operator(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool binary_operator(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool cast_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool alias_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool return_statement(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool import_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool basic_block(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool proc_call(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool statement(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool enum_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool struct_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool union_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool else_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool if_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool proc_expression(
            evaluator_context_t& context,
            evaluator_result_t& result);

        bool assignment(
            evaluator_context_t& context,
            evaluator_result_t& result);

    private:
        static std::unordered_map<syntax::ast_node_types_t, node_evaluator_callable> s_node_evaluators;

        element_builder* _builder = nullptr;
        compiler::program* _program = nullptr;
    };

};

