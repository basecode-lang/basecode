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

#include "field.h"

namespace basecode::compiler {

    field::field(
        element* parent,
        const std::string& name,
        compiler::type* type,
        compiler::initializer* initializer): element(parent),
                                             _name(name),
                                             _type(type),
                                             _initializer(initializer) {
    }

    field::~field() {
    }

    compiler::type* field::type() {
        return _type;
    }

    std::string field::name() const {
        return _name;
    }

    bool field::inferred_type() const {
        return _inferred_type;
    }

    void field::type(compiler::type* t) {
        _type = t;
    }

    compiler::initializer* field::initializer() {
        return _initializer;
    }

    void field::initializer(compiler::initializer* v) {
        _initializer = v;
    }

};