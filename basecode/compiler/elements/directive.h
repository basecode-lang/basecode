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

#include "element.h"

namespace basecode::compiler {

    class directive : public element {
    public:
        static directive* directive_for_name(
            compiler::module* module,
            compiler::block* parent_scope,
            const std::string& name,
            const element_list_t& params);

        directive(
            compiler::module* module,
            compiler::block* parent_scope);

        virtual directive_type_t type() const;

        bool execute(compiler::session& session);

        bool evaluate(compiler::session& session);

    protected:
        virtual bool on_execute(compiler::session& session);

        virtual bool on_evaluate(compiler::session& session);
    };

}

