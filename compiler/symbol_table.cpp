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

#include "symbol_table.h"

namespace basecode::compiler {

    void symbol_table::put(
            const std::string& name,
            const symbol_table_entry_t& value) {
        _symbols.insert(std::make_pair(name, value));
    }

    void symbol_table::clear() {
        _symbols.clear();
    }

    std::set<std::string> symbol_table::names() const {
        std::set<std::string> names;
        for (const auto& symbol : _symbols)
            names.insert(symbol.first);
        return names;
    }

    void symbol_table::remove(const std::string& name) {
        _symbols.erase(name);
    }

    bool symbol_table::is_defined(const std::string& name) {
        return _symbols.count(name) > 0;
    }

    symbol_lookup_result_t symbol_table::get(const std::string& name) {
        auto range = _symbols.equal_range(name);
        if (range.first == range.second)
            return {};
        symbol_lookup_result_t result;
        result.found = true;
        for (auto it = range.first; it != range.second; ++it) {
            result.entries.push_back(&it->second);
        }
        return result;
    }

}