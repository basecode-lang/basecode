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

#include "element.h"

namespace basecode::compiler {

    class argument_list : public element {
    public:
        argument_list(element* parent);

        void add(element* item);

        void remove(common::id_t id);

        element* find(common::id_t id);

        const element_list_t& elements() const;

    protected:
        bool on_emit(
            common::result& r,
            emit_context_t& context) override;

    private:
        element_list_t _elements {};
    };

};

