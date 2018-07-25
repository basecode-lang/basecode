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

    class symbol_element : public element {
    public:
        symbol_element(
            compiler::block* parent_scope,
            const std::string& name,
            const string_list_t& namespaces);

        std::string name() const;

        bool is_qualified() const;

        void constant(bool value);

        void cache_fully_qualified_name();

        std::string fully_qualified_name();

        const string_list_t& namespaces() const;

        qualified_symbol_t qualified_symbol() const;

        bool operator== (const symbol_element& other) const;

        bool operator== (const qualified_symbol_t& other) const;

    protected:
        bool on_is_constant() const override;

    private:
        std::string _name {};
        bool _is_constant = false;
        string_list_t _namespaces {};
        std::string _fully_qualified_name {};
    };

};

