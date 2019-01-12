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

    class character_literal  : public element {
    public:
        character_literal(
            compiler::module* module,
            block* parent_scope,
            common::rune_t rune);

        common::rune_t rune() const;

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_as_integer(uint64_t& value) const override;

        bool on_as_rune(common::rune_t& value) const override;

    private:
        common::rune_t _rune;
    };

};

