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

#include <unordered_map>
#include <common/result.h>
#include "token.h"

namespace basecode::syntax {

    class token_pool {
    public:
        static token_pool* instance();

        token_pool(const token_pool&) = delete;

        token_t* find(common::id_t id);

        token_t* add(token_type_t type);

        token_t* add(token_type_t type, const std::string_view& value);

    private:
        token_pool() = default;

    private:
        std::unordered_map<common::id_t, token_t> _tokens {};
    };

}

