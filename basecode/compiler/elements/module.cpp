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

#include <compiler/session.h>
#include <compiler/element_builder.h>
#include "block.h"
#include "module.h"

namespace basecode::compiler {

    module::module(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope) : compiler::element(module, parent_scope, element_type_t::module),
                                      _scope(scope) {
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

    compiler::element* module::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        auto copy = session.builder().make_module(
            new_scope,
            _scope->clone<compiler::block>(session, new_scope));
        copy->_source_file = _source_file;
        return copy;
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

}