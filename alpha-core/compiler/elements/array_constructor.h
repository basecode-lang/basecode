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

    class array_constructor : public element {
    public:
        array_constructor(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            const compiler::element_list_t& subscripts);

        compiler::argument_list* args();

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_emit(compiler::session& session) override;

    private:
        compiler::element_list_t _subscripts {};
        compiler::argument_list* _args = nullptr;
    };

};

