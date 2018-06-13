// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <common/id_pool.h>
#include <common/result.h>
#include "element_types.h"

namespace basecode::compiler {

    class element {
    public:
        element();

        virtual ~element();

        id_t id() const;

        bool fold(common::result& result);

        bool remove_directive(const std::string& name);

        bool remove_attribute(const std::string& name);

        directive* find_directive(const std::string& name);

        attribute* find_attribute(const std::string& name);

    protected:
        virtual bool on_fold(common::result& result);

    private:
        id_t _id;
        directive_map_t _directives {};
        attribute_map_t _attributes {};
    };

};

