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

#include <map>
#include <string>
#include <memory>
#include <unordered_map>
#include "type.h"
#include "element.h"
#include "initializer.h"

namespace basecode::compiler {

    class field : public element {
    public:
        field(
            compiler::module* module,
            block* parent_scope,
            compiler::identifier* identifier);

        compiler::identifier* identifier();

    protected:
        bool on_infer_type(
            const compiler::session& session,
            type_inference_result_t& result) override;

        void on_owned_elements(element_list_t& list) override;

    private:
        compiler::identifier* _identifier;
    };

};