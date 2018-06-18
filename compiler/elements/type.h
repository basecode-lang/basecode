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
#include <unordered_map>
#include "element.h"

namespace basecode::compiler {

    class type : public element {
    public:
        type(
            element* parent,
            const std::string& name);

        inline std::string name() const {
            return _name;
        }

    private:
        std::string _name;
    };

    struct type_map_t {
        void add(
                const std::string& name,
                compiler::type* type) {
            _types.insert(std::make_pair(name, type));
        }

        size_t size() const {
            return _types.size();
        }

        bool remove(const std::string& name) {
            return _types.erase(name) > 0;
        }

        compiler::type* find(const std::string& name) {
            auto it = _types.find(name);
            if (it != _types.end())
                return it->second;
            return nullptr;
        }

    private:
        std::unordered_map<std::string, type*> _types {};
    };
};