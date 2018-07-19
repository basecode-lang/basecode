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

    class module : public element {
    public:
        module(
            compiler::block* parent_scope,
            compiler::block* scope);

        compiler::block* scope();

        std::filesystem::path source_file() const;

        void source_file(const std::filesystem::path& value);

    protected:
        bool on_emit(
            common::result& r,
            emit_context_t& context) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::block* _scope = nullptr;
        std::filesystem::path _source_file;
    };

};

