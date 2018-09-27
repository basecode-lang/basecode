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

    class argument_list : public element {
    public:
        argument_list(
            compiler::module* module,
            block* parent_scope);

        size_t size() const;

        void add(element* item);

        void remove(common::id_t id);

        element* find(common::id_t id);

        int32_t find_index(common::id_t id);

        const element_list_t& elements() const;

        element* replace(size_t index, element* item);

    protected:
        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        element_list_t _elements {};
    };

};

