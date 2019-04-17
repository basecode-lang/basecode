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

#pragma once

#include "type.h"
#include "element.h"
#include "identifier.h"

namespace basecode::compiler {

    class block : public element {
    public:
        block(
            compiler::module* module,
            block* parent_scope,
            element_type_t type = element_type_t::block);

        type_map_t& types();

        block_list_t& blocks();

        import_list_t& imports();

        bool has_statements() const;

        defer_stack_t& defer_stack();

        bool has_stack_frame() const;

        reference_map_t& references();

        statement_list_t& statements();

        identifier_map_t& identifiers();

        void has_stack_frame(bool value);

    protected:
        compiler::element* on_clone(
            compiler::session& session,
            compiler::block* new_scope) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        type_map_t _types {};
        block_list_t _blocks {};
        import_list_t _imports {};
        bool _has_stack_frame = false;
        defer_stack_t _defer_stack {};
        reference_map_t _references {};
        statement_list_t _statements {};
        identifier_map_t _identifiers {};
    };

}