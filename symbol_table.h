#pragma once

#include "result.h"
#include "parser_types.h"

namespace basecode {

    using symbol_dict = std::map<std::string, ast_node_shared_ptr>;

    class symbol_table {
    public:
        symbol_table() = default;

        void put(
            const std::string& name,
            const ast_node_shared_ptr& value);

        void clear();

        bool missing_is_error() const {
            return _missing_is_error;
        }

        void missing_is_error(bool flag) {
            _missing_is_error = flag;
        }

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
        bool _missing_is_error = true;
    };

};