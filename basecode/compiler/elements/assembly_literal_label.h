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

    class assembly_literal_label  : public element {
    public:
        assembly_literal_label(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type* type,
            const std::string& name);

        std::string name() const;

        compiler::type* type() const;

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

    private:
        std::string _name;
        compiler::type* _type = nullptr;
    };

}

