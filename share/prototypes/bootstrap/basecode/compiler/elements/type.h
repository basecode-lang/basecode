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

#include <string>
#include <unordered_map>
#include <compiler/compiler_types.h>
#include "element.h"

namespace basecode::compiler {

    class type : public element {
    public:
        static std::string make_info_label_name(compiler::type* type);

        static std::string make_literal_label_name(compiler::type* type);

        static std::string make_literal_data_label_name(compiler::type* type);

        type(
            compiler::module* module,
            block* parent_scope,
            element_type_t type,
            compiler::symbol_element* symbol);

        bool packed() const;

        void packed(bool value);

        size_t alignment() const;

        size_t size_in_bytes() const;

        void alignment(size_t value);

        virtual bool is_signed() const;

        virtual bool is_proc_type() const;

        virtual bool is_array_type() const;

        vm::ffi_types_t to_ffi_type() const;

        virtual bool is_unknown_type() const;

        virtual bool is_pointer_type() const;

        bool type_check(compiler::type* other);

        virtual bool is_composite_type() const;

        number_class_t number_class() const;

        compiler::symbol_element* symbol() const;

        bool initialize(compiler::session& session);

        void symbol(compiler::symbol_element* value);

        virtual std::string name(const std::string& alias = "") const;

    protected:
        void size_in_bytes(size_t value);

        virtual bool on_type_check(compiler::type* other);

        virtual number_class_t on_number_class() const;

        void on_owned_elements(element_list_t& list) override;

        virtual bool on_initialize(compiler::session& session);

    private:
        bool _packed = false;
        size_t _alignment = 0;
        size_t _size_in_bytes {};
        compiler::symbol_element* _symbol = nullptr;
    };

};