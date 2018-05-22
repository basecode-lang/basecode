#pragma once

#include <string>
#include <cstdint>
#include <filesystem>
#include "terp.h"
#include "scope.h"
#include "parser.h"

namespace basecode {

    class compiler {
    public:
        compiler(
            size_t heap_size,
            size_t stack_size);

        virtual ~compiler();

        bool initialize(result& r);

        bool compile(result& r, std::istream& input);

        bool compile_stream(result& r, std::istream& input);

        bool compile_file(result& r, const std::filesystem::path& path);

    private:
        void build_scope_tree(
            result& r,
            basecode::scope* scope,
            const ast_node_shared_ptr& node);

    private:
        terp _terp;
        scope _global_scope;
    };

};

