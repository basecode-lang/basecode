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

#include "type.h"

namespace basecode::compiler {

    class generic_type : public compiler::type {
    public:
        static std::string name_for_generic_type(
            const type_reference_list_t& constraints);

        generic_type(
            compiler::module* module,
            compiler::block* parent_scope,
            const compiler::type_reference_list_t& constraints);

        inline bool is_open() const {
            return _constraints.empty();
        }

        const compiler::type_reference_list_t& constraints() const;

    protected:
        bool on_type_check(compiler::type* other) override;

        bool on_initialize(compiler::session& session) override;

    private:
        compiler::type_reference_list_t _constraints {};
    };

};

