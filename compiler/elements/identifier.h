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

#include "type.h"
#include "element.h"
#include "initializer.h"

namespace basecode::compiler {

    class identifier : public element {
    public:
        identifier(
            const std::string& name,
            const compiler::initializer& initializer);

        ~identifier() override;

        compiler::type* type();

        std::string name() const;

        bool is_constant() const;

        bool inferred_type() const;

        void type(compiler::type* t);

        const compiler::initializer& initializer() const;

    private:
        std::string _name;
        bool _constant = false;
        bool _inferred_type = false;
        compiler::type* _type = nullptr;
        compiler::initializer _initializer;
    };

    struct identifier_map_t {
        void add(
                const std::string& name,
                compiler::identifier* identifier) {
            _identifiers.insert(std::make_pair(name, identifier));
        }

        size_t size() const {
            return _identifiers.size();
        }

        bool remove(const std::string& name) {
            return _identifiers.erase(name) > 0;
        }

        identifier* find(const std::string& name) {
            auto it = _identifiers.find(name);
            if (it != _identifiers.end())
                return it->second;
            return nullptr;
        }

        // XXX: add ability to get range of identifiers for overloads

    private:
        std::unordered_multimap<std::string, identifier*> _identifiers {};
    };
};

