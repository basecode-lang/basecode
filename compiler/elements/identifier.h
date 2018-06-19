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
            element* parent,
            const std::string& name,
            compiler::initializer* initializer);

        ~identifier() override;

        bool constant() const;

        compiler::type* type();

        std::string name() const;

        void constant(bool value);

        bool inferred_type() const;

        void type(compiler::type* t);

        void inferred_type(bool value);

        compiler::initializer* initializer();

    private:
        std::string _name;
        bool _constant = false;
        bool _inferred_type = false;
        compiler::type* _type = nullptr;
        compiler::initializer* _initializer;
    };

};

