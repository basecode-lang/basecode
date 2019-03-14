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

    class string_literal : public element {
    public:
        static bool escape(
            const std::string& value,
            std::string& result);

        string_literal(
            compiler::module* module,
            block* parent_scope,
            const std::string& value);

        std::string value() const;

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_as_string(std::string& value) const override;

    private:
        std::string _value;
    };

}

