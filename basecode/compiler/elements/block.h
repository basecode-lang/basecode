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

        defer_stack_t& defer_stack();

        statement_list_t& statements();

        identifier_map_t& identifiers();

    protected:
        bool on_emit(compiler::session& session) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        type_map_t _types {};
        block_list_t _blocks {};
        import_list_t _imports {};
        defer_stack_t _defer_stack {};
        statement_list_t _statements {};
        identifier_map_t _identifiers {};
    };

};