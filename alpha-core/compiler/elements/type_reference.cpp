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

#include "type.h"
#include "type_reference.h"

namespace basecode::compiler {

    type_reference::type_reference(
            compiler::module* module,
            block* parent_scope,
            const qualified_symbol_t& symbol,
            compiler::type* type) : element(module, parent_scope, element_type_t::type_reference),
                                    _symbol(symbol),
                                    _type(type) {
    }

    bool type_reference::on_infer_type(
            const compiler::session& session,
            infer_type_result_t& result) {
        result.inferred_type = _type;
        result.reference = this;
        return true;
    }

    bool type_reference::resolved() const {
        return _type != nullptr;
    }

    compiler::type* type_reference::type() {
        return _type;
    }

    std::string type_reference::name() const {
        return _type->name(_symbol.name);
    }

    bool type_reference::is_any_type() const {
        return _type != nullptr
               && _type->element_type() == element_type_t::any_type;
    }

    bool type_reference::is_proc_type() const {
        return _type != nullptr && _type->is_proc_type();
    }

    bool type_reference::is_array_type() const {
        return _type != nullptr
               && _type->element_type() == element_type_t::array_type;
    }

    bool type_reference::on_is_constant() const {
        return true;
    }

    bool type_reference::is_pointer_type() const {
        return _type != nullptr && _type->is_pointer_type();
    }

    bool type_reference::is_unknown_type() const {
        return _type != nullptr
               && _type->element_type() == element_type_t::unknown_type;
    }

    bool type_reference::is_composite_type() const {
        return _type != nullptr && _type->is_composite_type();
    }

    void type_reference::type(compiler::type* value) {
        _type = value;
    }

    const qualified_symbol_t& type_reference::symbol() const {
        return _symbol;
    }

};