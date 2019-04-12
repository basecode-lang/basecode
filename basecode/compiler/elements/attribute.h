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

#include <string>
#include <unordered_map>
#include "element.h"

namespace basecode::compiler {

    class attribute : public element {
    public:
        attribute(
            compiler::module* module,
            block* parent_scope,
            const std::string_view& name,
            element* expr);

        element* expression();

        std::string_view name() const;

    protected:
        bool on_as_bool(bool& value) const override;

        bool on_as_float(double& value) const override;

        bool on_as_integer(uint64_t& value) const override;

        bool on_as_string(std::string& value) const override;

        void on_owned_elements(element_list_t& list) override;

    private:
        std::string_view _name;
        element* _expr = nullptr;
    };

}

