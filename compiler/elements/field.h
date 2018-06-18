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

#include <map>
#include <string>
#include <memory>
#include <unordered_map>
#include "type.h"
#include "element.h"
#include "initializer.h"

namespace basecode::compiler {

    class field : public element {
    public:
        field(
            const std::string& name,
            compiler::type* type,
            compiler::initializer* initializer);

        ~field() override;

        compiler::type* type();

        std::string name() const;

        bool inferred_type() const;

        void type(compiler::type* t);

        compiler::initializer* initializer();

        void initializer(compiler::initializer* v);

    private:
        std::string _name;
        bool _inferred_type = false;
        compiler::type* _type = nullptr;
        compiler::initializer* _initializer = nullptr;
    };

    struct field_map_t {
        ~field_map_t() {
            for (auto field : _fields)
                delete field.second;
            _fields.clear();
        }

        void add(
                const std::string& name,
                compiler::type* type,
                compiler::initializer* initializer) {
            auto field = new compiler::field(name, type, initializer);
            field->type(type);
            _fields.insert(std::make_pair(name, field));
        }

        inline size_t size() const {
            return _fields.size();
        }

        bool remove(const std::string& name) {
            return _fields.erase(name) > 0;
        }

        compiler::field* find(const std::string& name) {
            auto it = _fields.find(name);
            if (it != _fields.end())
                return it->second;
            return nullptr;
        }

    private:
        std::unordered_map<std::string, field*> _fields {};
    };

};