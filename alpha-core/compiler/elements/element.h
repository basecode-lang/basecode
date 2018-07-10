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
#include <vm/assembler.h>
#include <common/result.h>
#include <common/id_pool.h>
#include "element_types.h"

namespace basecode::compiler {

    enum class emit_context_type_t {
        empty,
        if_element,
        procedure_type
    };

    enum class emit_access_type_t {
        read,
        write
    };

    struct procedure_type_data_t {
        std::string identifier_name;
    };

    struct if_data_t {
        std::string true_branch_label;
        std::string false_branch_label;
    };

    struct emit_context_t {
        inline static emit_context_t for_if_element(
                const emit_context_t& parent_context,
                const std::string& true_branch_label,
                const std::string& false_branch_label) {
            auto data = new if_data_t();
            data->true_branch_label = true_branch_label;
            data->false_branch_label = false_branch_label;
            return emit_context_t {
                .type = emit_context_type_t::if_element,
                .data = {
                    .if_data = data
                }
            };
        }

        inline static emit_context_t for_procedure_type(
                const emit_context_t& parent_context,
                const std::string& name) {
            auto data = new procedure_type_data_t();
            data->identifier_name = name;
            return emit_context_t {
                .type = emit_context_type_t::procedure_type,
                .data = {
                    .procedure_type = data
                }
            };
        }

        inline static emit_context_t for_read(const emit_context_t& parent_context) {
            return emit_context_t {
                .access_type = emit_access_type_t::read,
                .type = parent_context.type,
            };
        }

        inline static emit_context_t for_write(const emit_context_t& parent_context) {
            return emit_context_t {
                .access_type = emit_access_type_t::write,
                .type = parent_context.type,
            };
        }

        ~emit_context_t() {
            switch (type) {
                case emit_context_type_t::if_element:
                    delete data.if_data;
                    break;
                case emit_context_type_t::procedure_type:
                    delete data.procedure_type;
                    break;
                default:
                    break;
            }
        }

        emit_access_type_t access_type = emit_access_type_t::read;
        emit_context_type_t type = emit_context_type_t::empty;
        union {
            if_data_t* if_data;
            procedure_type_data_t* procedure_type;
        } data;
    };

    class element {
    public:
        element(
            element* parent,
            element_type_t type);

        virtual ~element();

        bool emit(
            common::result& r,
            vm::assembler& assembler,
            const emit_context_t& context);

        element* parent();

        common::id_t id() const;

        bool is_constant() const;

        bool fold(common::result& r);

        attribute_map_t& attributes();

        bool as_bool(bool& value) const;

        bool as_float(double& value) const;

        element_type_t element_type() const;

        virtual std::string label_name() const;

        bool as_integer(uint64_t& value) const;

        bool as_string(std::string& value) const;

        compiler::type* infer_type(const compiler::program* program);

    protected:
        virtual bool on_emit(
            common::result& r,
            vm::assembler& assembler,
            const emit_context_t& context);

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

