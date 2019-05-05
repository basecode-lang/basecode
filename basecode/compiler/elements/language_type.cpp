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
#include <compiler/type_name_builder.h>
#include "language_type.h"

namespace basecode::compiler {

    std::string language_type::name_for_language() {
        type_name_builder builder {};
        builder
            .add_part("language")
            .add_part(common::id_pool::instance()->allocate());
        return builder.format();
    }

    ///////////////////////////////////////////////////////////////////////////

    language_type::language_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::raw_block* grammar,
            compiler::raw_block* translator,
            compiler::symbol_element* symbol) : compiler::composite_type(
                                                    module,
                                                    parent_scope,
                                                    composite_types_t::struct_type,
                                                    scope,
                                                    symbol,
                                                    element_type_t::language_type),
                                                _grammar(grammar),
                                                _translator(translator) {
    }

    bool language_type::on_type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        return other != nullptr && other->id() == id();
    }

    compiler::raw_block* language_type::grammar() {
        return _grammar;
    }

    compiler::raw_block* language_type::translator() {
        return _translator;
    }

    bool language_type::on_initialize(compiler::session& session) {
        return composite_type::on_initialize(session);
    }

}