#include "symbol_table.h"

namespace basecode {

    void symbol_table::put(
        const std::string& name,
        const ast_node_shared_ptr& value) {
        _symbols[name] = value;
    }

    void symbol_table::clear() {
        _symbols.clear();
    }

    void symbol_table::remove(const std::string& name) {
        _symbols.erase(name);
    }

    ast_node_shared_ptr symbol_table::get(const std::string& name) const {
        auto it = _symbols.find(name);
        if (it == _symbols.end())
            return nullptr;
        return it->second;
    }

}