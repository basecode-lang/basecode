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

    class assembly_label : public element {
    public:
        assembly_label(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::identifier_reference* ref,
            compiler::type* type,
            const std::string& name);

        std::string name() const;

        // XXX: this sucks, fix me
        compiler::type* type() const;

        compiler::identifier_reference* reference();

    protected:
        bool on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) override;

        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

    private:
        std::string _name;
        compiler::type* _type = nullptr;
        compiler::identifier_reference* _ref = nullptr;
    };

};

