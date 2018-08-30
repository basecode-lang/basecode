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

    class intrinsic  : public element {
    public:
        static intrinsic* intrinsic_for_call(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            const qualified_symbol_t& symbol);

        intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args);

        compiler::argument_list* arguments();

    protected:
        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::argument_list* _arguments = nullptr;
    };

};

