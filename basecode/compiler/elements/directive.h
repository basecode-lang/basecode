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
#include <vm/instruction_block.h>
#include <compiler/compiler_types.h>
#include "block.h"

namespace basecode::compiler {

    class directive : public element {
    public:
        using directive_callable = std::function<bool (
            compiler::directive*,
            compiler::session&)>;

        directive(
            compiler::module* module,
            compiler::block* parent_scope,
            const std::string& name,
            compiler::element* lhs,
            compiler::element* rhs,
            compiler::element* body);

        compiler::element* lhs();

        compiler::element* rhs();

        std::string name() const;

        compiler::element* body();

        bool execute(compiler::session& session);

        bool evaluate(compiler::session& session);

    protected:
        bool on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) override;

        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_apply_fold_result(
            compiler::element* e,
            const fold_result_t& fold_result) override;

        bool on_is_constant() const override;

        void on_owned_elements(element_list_t& list) override;

    private:
        // --------------------
        // if/elif/else directive
        // --------------------
        bool on_execute_if(compiler::session& session);

        bool on_evaluate_if(compiler::session& session);

        // --------------------
        // type directive
        // --------------------
        bool on_execute_type(compiler::session& session);

        bool on_evaluate_type(compiler::session& session);

        // --------------------
        // assert directive
        // --------------------
        bool on_execute_assert(compiler::session& session);

        bool on_evaluate_assert(compiler::session& session);

        // --------------------
        // assembly directive
        // --------------------
        bool on_execute_assembly(compiler::session& session);

        bool on_evaluate_assembly(compiler::session& session);

        // --------------------
        // run directive
        // --------------------
        bool on_execute_run(compiler::session& session);

        bool on_evaluate_run(compiler::session& session);

        // --------------------
        // foreign directive
        // --------------------
        bool on_execute_foreign(compiler::session& session);

        bool on_evaluate_foreign(compiler::session& session);

        // --------------------
        // intrinsic directive
        // --------------------
        bool on_execute_intrinsic(compiler::session& session);

        bool on_evaluate_intrinsic(compiler::session& session);

    private:
        static std::unordered_map<std::string, directive_callable> s_execute_handlers;
        static std::unordered_map<std::string, directive_callable> s_evaluate_handlers;

        std::string _name;
        compiler::element* _lhs = nullptr;
        compiler::element* _rhs = nullptr;
        compiler::element* _body = nullptr;
        compiler::element* _true_body = nullptr;
        vm::instruction_block* _instruction_block = nullptr;
    };

};

