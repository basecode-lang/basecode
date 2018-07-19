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
            compiler::block* parent_scope,
            compiler::block* scope) : compiler::element(parent_scope, element_type_t::module),
                                      _scope(scope) {
    }

    bool module::on_emit(
            common::result& r,
            emit_context_t& context) {
        if (_scope == nullptr)
            return true;
        return _scope->emit(r, context);
    }

    compiler::block* module::scope() {
        return _scope;
    }

    std::filesystem::path module::source_file() const {
        return _source_file;
    }

    void module::on_owned_elements(element_list_t& list) {
        if (_scope != nullptr)
            list.emplace_back(_scope);
    }

    void module::source_file(const std::filesystem::path& value) {
        _source_file = value;
    }

};