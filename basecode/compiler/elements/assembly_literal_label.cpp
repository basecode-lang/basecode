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
#include "assembly_literal_label.h"

namespace basecode::compiler {

    assembly_literal_label::assembly_literal_label(
        compiler::module* module,
        compiler::block* parent_scope,
        compiler::type* type,
        const std::string_view& name) : element(module, parent_scope, element_type_t::assembly_literal_label),
                                        _name(name),
                                        _type(type) {
    }

    bool assembly_literal_label::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        result.types.emplace_back(_type);
        return true;
    }

    compiler::element* assembly_literal_label::on_clone(
            compiler::session& session,
            compiler::block* new_scope) {
        return session.builder().make_assembly_literal_label(
            new_scope,
            _type,
            _name);
    }

    bool assembly_literal_label::on_is_constant() const {
        return true;
    }

    compiler::type* assembly_literal_label::type() const {
        return _type;
    }

    std::string_view assembly_literal_label::name() const {
        return _name;
    }

}