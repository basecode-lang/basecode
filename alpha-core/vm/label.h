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

#include <string>
#include <common/id_pool.h>

namespace basecode::vm {

    class label;

    struct label_ref_t {
        common::id_t id;
        std::string name;
        label* resolved = nullptr;
    };

    class label {
    public:
        explicit label(const std::string& name);

        uint64_t address() const;

        std::string name() const;

        void address(uint64_t value);

    private:
        std::string _name;
        uint64_t _address = 0;
    };

};

