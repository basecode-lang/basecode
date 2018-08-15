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

#include "block.h"
#include "module.h"

namespace basecode::compiler {

    module::module(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope) : compiler::element(module, parent_scope, element_type_t::module),
                                      _scope(scope) {
    }

    bool module::on_emit(
            common::result& r,
            emit_context_t& context) {
        if (_scope == nullptr)
            return true;
        return _scope->emit(r, context);
    }

    bool module::is_root() const {
        return _is_root;
    }

    void module::is_root(bool value) {
        _is_root = value;
    }

    compiler::block* module::scope() {
        return _scope;
    }

    common::source_file* module::source_file() const {
        return _source_file;
    }

    void module::on_owned_elements(element_list_t& list) {
        if (_scope != nullptr)
            list.emplace_back(_scope);
    }

    void module::source_file(common::source_file* source_file) {
        _source_file = source_file;
    }

};