// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <parser/ast.h>
#include "block.h"
#include "numeric_type.h"
#include "any_type.h"
#include "string_type.h"

namespace basecode::compiler {

    class program : public block {
    public:
        program();

        ~program() override;

        bool initialize(
            common::result& r,
            const syntax::ast_node_shared_ptr& root);

        element* find_element(id_t id);

    private:
        void initialize_core_types();

        void evaluate(
            common::result& r,
            const syntax::ast_node_shared_ptr& node);

        bool is_subtree_constant(const syntax::ast_node_shared_ptr& node);

    private:
        static inline any_type s_any_type {};
        static inline string_type s_string_type {};
        static inline numeric_type s_bool_type {"bool", 0, 1};
        static inline numeric_type s_address_type {"address", 0, UINTPTR_MAX};

        static inline numeric_type s_u8_type  {"u8",  0, UINT8_MAX};
        static inline numeric_type s_u16_type {"u16", 0, UINT16_MAX};
        static inline numeric_type s_u32_type {"u32", 0, UINT32_MAX};
        static inline numeric_type s_u64_type {"u64", 0, UINT64_MAX};

        static inline numeric_type s_s8_type  {"s8",  INT8_MIN,  INT8_MAX};
        static inline numeric_type s_s16_type {"s16", INT16_MIN, INT16_MAX};
        static inline numeric_type s_s32_type {"s32", INT32_MIN, INT32_MAX};
        static inline numeric_type s_s64_type {"s64", INT64_MIN, INT64_MAX};

        static inline numeric_type s_f32_type {"f32", 0, UINT32_MAX};
        static inline numeric_type s_f64_type {"f64", 0, UINT64_MAX};

        std::unordered_map<id_t, element*> _elements {};
    };

};

