#pragma once

#include <string>

namespace basecode {

    enum class token_types_t {

    };

    struct token_t {
        token_types_t type;
        std::string value;
        uint32_t line = 0;
        uint32_t column = 0;
    };
};

