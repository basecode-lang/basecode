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
#include <compiler/compiler_types.h>
#include "block.h"

namespace basecode::compiler {

    class directive : public element {
    public:
        using directive_callable = std::function<bool (
            compiler::directive*,
            common::result&,
            compiler::session&,
            compiler::program*)>;

        directive(
            block* parent_scope,
            const std::string& name,
            element* expression);

        bool evaluate(
            common::result& r,
            compiler::session& session,
            compiler::program* program);

        bool execute(
            common::result& r,
            compiler::session& session,
            compiler::program* program);

        element* expression();

        std::string name() const;

    protected:
        void on_owned_elements(element_list_t& list) override;

    private:
        // --------------------
        // run directive
        // --------------------
        bool on_execute_run(
            common::result& r,
            compiler::session& session,
            compiler::program* program);

        bool on_evaluate_run(
            common::result& r,
            compiler::session& session,
            compiler::program* program);

        // --------------------
        // run directive
        // --------------------
        bool on_execute_foreign(
            common::result& r,
            compiler::session& session,
            compiler::program* program);

        bool on_evaluate_foreign(
            common::result& r,
            compiler::session& session,
            compiler::program* program);

    private:
        static std::unordered_map<std::string, directive_callable> s_execute_handlers;
        static std::unordered_map<std::string, directive_callable> s_evaluate_handlers;

        std::string _name;
        element* _expression = nullptr;
    };

};

