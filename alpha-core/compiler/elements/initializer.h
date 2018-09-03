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

    class initializer : public element {
    public:
        initializer(
            compiler::module* module,
            block* parent_scope,
            element* expr);

        element* expression();

        void expression(element* value);

        compiler::procedure_type* procedure_type();

    protected:
        bool on_infer_type(
            const compiler::session& program,
            infer_type_result_t& result) override;

        bool on_as_bool(bool& value) const override;

        bool on_as_float(double& value) const override;

        bool on_emit(compiler::session& session) override;

        bool on_as_integer(uint64_t& value) const override;

        bool on_as_string(std::string& value) const override;

        void on_owned_elements(element_list_t& list) override;

    private:
        element* _expr = nullptr;
    };

};

