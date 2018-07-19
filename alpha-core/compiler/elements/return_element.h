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

    class return_element : public element {
    public:
        explicit return_element(block* parent_scope);

        element_list_t& expressions();

    protected:
        bool on_emit(
            common::result& r,
            emit_context_t& context) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        element_list_t _expressions {};
    };

};

