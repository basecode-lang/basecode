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

#include <cstdint>
#include <common/result.h>
#include <common/id_pool.h>
#include "element_types.h"

namespace basecode::compiler {

    class element {
    public:
        element(
            element* parent,
            element_type_t type);

        virtual ~element();

        element* parent();

        common::id_t id() const;

        bool is_constant() const;

        bool fold(common::result& r);

        attribute_map_t& attributes();

        bool as_bool(bool& value) const;

        bool as_float(double& value) const;

        element_type_t element_type() const;

        bool as_integer(uint64_t& value) const;

        bool as_string(std::string& value) const;

        compiler::type* infer_type(const compiler::program* program);

    protected:
        virtual bool on_is_constant() const;

        virtual bool on_fold(common::result& r);

        virtual bool on_as_bool(bool& value) const;

        virtual bool on_as_float(double& value) const;

        virtual bool on_as_integer(uint64_t& value) const;

        virtual bool on_as_string(std::string& value) const;

        virtual compiler::type* on_infer_type(const compiler::program* program);

    private:
        common::id_t _id;
        element* _parent = nullptr;
        attribute_map_t _attributes {};
        element_type_t _element_type = element_type_t::element;
    };

};

