#pragma once

#include <map>
#include <stack>
#include <string>
#include <fmt/format.h>
#include "ast.h"
#include "result.h"

namespace basecode {

    class symbol_table;

    class parser {
    public:
        parser() = default;

        const basecode::result& result() const {
            return _result;
        }

    protected:
        basecode::result _result;
    };

};