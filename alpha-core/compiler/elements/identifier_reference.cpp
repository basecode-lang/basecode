// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include "identifier_reference.h"

namespace basecode::compiler {

    identifier_reference::identifier_reference(
        block* parent_scope,
        const qualified_symbol_t& symbol,
        compiler::identifier* identifier) : element(parent_scope, element_type_t::identifier_reference),
                                            _symbol(symbol),
                                            _identifier(identifier) {
    }

    bool identifier_reference::resolved() const {
        return _identifier != nullptr;
    }

    compiler::identifier* identifier_reference::identifier() {
        return _identifier;
    }

    const qualified_symbol_t& identifier_reference::symbol() const {
        return _symbol;
    }

    void identifier_reference::identifier(compiler::identifier* value) {
        _identifier = value;
    }

};