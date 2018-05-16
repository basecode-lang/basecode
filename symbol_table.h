#pragma once

#include "result.h"
#include "ast.h"

namespace basecode {

    using symbol_dict = std::map<std::string, ast_node_shared_ptr>;

    class symbol_table {
    public:
        symbol_table() = default;

        void put(
            const std::string& name,
            const ast_node_shared_ptr& value);

        void clear();

        void remove(const std::string& name);

        std::vector<std::string> identifiers() {
            std::vector<std::string> identifiers;
            for (auto& symbol : _symbols) {
                identifiers.push_back(symbol.first);
            }
            return identifiers;
        }

        ast_node_shared_ptr get(const std::string& name) const;

    private:
        symbol_dict _symbols;
    };

};