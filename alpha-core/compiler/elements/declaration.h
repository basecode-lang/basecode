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

    class declaration : public element {
    public:
        declaration(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::identifier* identifier,
            compiler::binary_operator* assignment);

        compiler::identifier* identifier();

        compiler::binary_operator* assignment();

    protected:
        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::identifier* _identifier = nullptr;
        compiler::binary_operator* _assignment = nullptr;
    };

};

