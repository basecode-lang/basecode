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
            element* parent,
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

};