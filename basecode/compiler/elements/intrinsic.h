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

#include "element.h"

namespace basecode::compiler {

    class intrinsic  : public element {
    public:
        static intrinsic* intrinsic_for_call(
            compiler::session& session,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            const qualified_symbol_t& symbol,
            const compiler::type_reference_list_t& type_params);

        intrinsic(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::argument_list* args,
            const compiler::type_reference_list_t& type_params);

        virtual bool can_fold() const;

        virtual std::string name() const;

        compiler::argument_list* arguments();

        const compiler::type_reference_list_t& type_parameters() const;

    protected:
        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::argument_list* _arguments = nullptr;
        compiler::type_reference_list_t _type_parameters {};
    };

};

