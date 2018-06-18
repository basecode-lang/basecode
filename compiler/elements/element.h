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
#include <common/result.h>
#include <common/id_pool.h>
#include "element_types.h"

namespace basecode::compiler {

    class element {
    public:
        explicit element(element* parent);

        virtual ~element();

        id_t id() const;

        element* parent();

        attribute_map_t& attributes();

        bool fold(common::result& result);

    protected:
        virtual bool on_fold(common::result& result);

    private:
        id_t _id;
        element* _parent = nullptr;
        attribute_map_t _attributes;
    };

};

