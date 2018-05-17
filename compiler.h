#pragma once

#include <cstdint>
#include "terp.h"
#include "parser.h"
#include "symbol_table.h"

namespace basecode {

    class compiler {
    public:
        explicit compiler(size_t heap_size);

        virtual ~compiler();

        bool initialize(result& r);

        inline uint64_t address() const {
            return _address;
        }

        bool compile(result& r, std::istream& input);

        inline basecode::symbol_table* symbol_table() {
            return &_symbol_table;
        }

        bool compile_stream(result& r, std::istream& input);

    private:
        terp _terp;
        uint64_t _address;
        basecode::symbol_table _symbol_table {};
    };

};

