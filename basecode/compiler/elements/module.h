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

#include <common/source_file.h>
#include "element.h"

namespace basecode::compiler {

    class module : public element {
    public:
        module(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope);

        bool is_root() const;

        void is_root(bool value);

        compiler::block* scope();

        common::source_file* source_file() const;

        void source_file(common::source_file* source_file);

    protected:
        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        bool _is_root = false;
        compiler::block* _scope = nullptr;
        common::source_file* _source_file = nullptr;
    };

};

