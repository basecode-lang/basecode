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

    class assignment_target : public compiler::element {
    public:
        assignment_target(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::identifier_reference_list_t refs);

        const compiler::identifier_reference_list_t& refs() const;

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

    private:
        compiler::identifier_reference_list_t _refs{};
    };

}

