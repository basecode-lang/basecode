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

#include "identifier.h"

namespace basecode::compiler {

    identifier::identifier(
            element* parent,
            const std::string& name,
            const compiler::initializer& initializer) : element(parent),
                                                        _name(name),
                                                        _initializer(initializer) {
    }

    identifier::~identifier() {
    }

    compiler::type* identifier::type() {
        return _type;
    }

    std::string identifier::name() const {
        return _name;
    }

    bool identifier::is_constant() const {
        return _constant;
    }

    const compiler::initializer& identifier::initializer() const {
        return _initializer;
    }

    bool identifier::inferred_type() const {
        return _inferred_type;
    }

    void identifier::type(compiler::type* t) {
        _type = t;
    }

};