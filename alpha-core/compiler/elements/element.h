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

#include <cstdint>
#include <common/result.h>
#include <common/id_pool.h>
#include "element_types.h"
#include "compiler/emit_context.h"

namespace basecode::compiler {

    class element {
    public:
        element(
            compiler::module* module,
            block* parent_scope,
            element_type_t type,
            element* parent_element = nullptr);

        virtual ~element();

        bool emit(
            common::result& r,
            emit_context_t& context);

        bool is_type() const;

        block* parent_scope();

        common::id_t id() const;

        template <typename T>
        T* parent_element_as() {
            if (_parent_element == nullptr)
                return nullptr;
            return dynamic_cast<T*>(_parent_element);
        }

        bool is_constant() const;

        element* parent_element();

        compiler::module* module();

        attribute_map_t& attributes();

        bool as_bool(bool& value) const;

        bool as_float(double& value) const;

        element_type_t element_type() const;

        void parent_element(element* value);

        void module(compiler::module* value);

        virtual std::string label_name() const;

        bool as_integer(uint64_t& value) const;

        bool as_string(std::string& value) const;

        element* fold(compiler::session& session);

        void owned_elements(element_list_t& list);

        bool is_parent_element(element_type_t type);

        const common::source_location& location() const;

        attribute* find_attribute(const std::string& name);

        void location(const common::source_location& location);

        compiler::type* infer_type(const compiler::program* program);

    protected:
        virtual bool on_emit(
            common::result& r,
            emit_context_t& context);

        virtual element* on_fold(compiler::session& session);

        element_register_t register_for(
            common::result& r,
            emit_context_t& context,
            element* e);

        virtual bool on_is_constant() const;

        virtual bool on_as_bool(bool& value) const;

        virtual bool on_as_float(double& value) const;

        virtual bool on_as_integer(uint64_t& value) const;

        virtual bool on_as_string(std::string& value) const;

        virtual void on_owned_elements(element_list_t& list);

        virtual compiler::type* on_infer_type(const compiler::program* program);

    private:
        common::id_t _id;
        block* _parent_scope = nullptr;
        attribute_map_t _attributes {};
        element* _parent_element = nullptr;
        compiler::module* _module = nullptr;
        common::source_location _location {};
        element_type_t _element_type = element_type_t::element;
    };

};

