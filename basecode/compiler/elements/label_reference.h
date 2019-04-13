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

    class label_reference : public element {
    public:
        label_reference(
            compiler::module* module,
            compiler::block* parent_scope,
            const std::string_view& label);

        std::string_view label() const;

    protected:
        bool on_fold(
            compiler::session& session,
            fold_result_t& result) override;

        bool on_is_constant() const override;

    private:
        std::string_view _label;
    };

}

