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

#include <vm/ffi.h>
#include <configure.h>
#include <compiler/session.h>
#include "attribute.h"
#include "assignment.h"
#include "identifier.h"
#include "declaration.h"
#include "initializer.h"
#include "symbol_element.h"
#include "procedure_type.h"
#include "type_reference.h"
#include "composite_type.h"
#include "foreign_directive.h"

namespace basecode::compiler {

    foreign_directive::foreign_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : directive(module, parent_scope, "foreign"),
                                             _expression(expression) {
    }

    bool foreign_directive::on_evaluate(compiler::session& session) {
        auto& ffi = session.ffi();

        auto assignment = dynamic_cast<compiler::assignment*>(_expression);
        auto proc_decl = dynamic_cast<compiler::declaration*>(assignment->expressions()[0]);
        if (proc_decl == nullptr)
            return false;

        auto proc_type = proc_decl->identifier()->initializer()->procedure_type();
        if (proc_type == nullptr)
            return false;

        auto attrs = proc_type->attributes().as_list();
        for (auto attr : attrs) {
            attributes().add(attr);
            proc_type->attributes().remove(attr->name());
        }
        proc_type->is_foreign(true);

        std::string library_name;
        auto library_attribute = find_attribute("library");
        if (library_attribute != nullptr) {
            if (!library_attribute->as_string(library_name)) {
                session.error(
                    module(),
                    "P004",
                    "unable to convert library name.",
                    location());
                return false;
            }
        }

        if (library_name.empty()) {
            session.error(
                module(),
                "P005",
                "library attribute required for foreign directive.",
                location());
            return false;
        }

        std::stringstream platform_name;
        platform_name
            << SHARED_LIBRARY_PREFIX
            << library_name
            << SHARED_LIBRARY_SUFFIX;
        boost::filesystem::path library_path(platform_name.str());
        auto library = ffi.load_shared_library(session.result(), library_path);
        if (library == nullptr) {
            auto msg = session.result().find_code("B062");
            if (msg != nullptr) {
                session.error(
                    module(),
                    "P006",
                    msg->message(),
                    location());
                session.result().remove_code("B062");
            }
            return false;
        }
        library->self_loaded(library_name == COMPILER_LIBRARY_NAME);

        auto ffi_decl = dynamic_cast<compiler::declaration*>(assignment->expressions()[0]);
        std::string symbol_name = ffi_decl->identifier()->symbol()->name();
        auto alias_attribute = attributes().find("alias");
        if (alias_attribute != nullptr) {
            if (!alias_attribute->as_string(symbol_name)) {
                session.error(
                    module(),
                    "P004",
                    "unable to convert alias attribute's name.",
                    location());
                return false;
            }
        }

        vm::function_signature_t signature {
            .symbol = symbol_name,
            .library = library,
        };

        auto is_variadic = false;

        auto params_list = proc_type->parameters().as_list();
        for (auto param : params_list) {
            vm::function_value_t value;
            value.name = param->identifier()->symbol()->name();

            auto type = param->identifier()->type_ref()->type();
            if (type != nullptr) {
                value.type = type->to_ffi_type();
                if (value.type == vm::ffi_types_t::any_type) {
                    is_variadic = true;
                } else if (value.type == vm::ffi_types_t::struct_type) {
                    auto composite_type = dynamic_cast<compiler::composite_type*>(type);
                    if (composite_type == nullptr)
                        return false;
                    for (auto fld : composite_type->fields().as_list()) {
                        vm::function_value_t fld_value;
                        fld_value.name = fld->identifier()->symbol()->name();
                        fld_value.type = fld->identifier()->type_ref()->type()->to_ffi_type();
                        value.fields.push_back(fld_value);
                    }
                }
            }

            signature.arguments.push_back(value);
        }

        auto return_type_field = proc_type->return_type();
        signature.return_value.type = return_type_field != nullptr ?
                                      return_type_field->identifier()->type_ref()->type()->to_ffi_type() :
                                      vm::ffi_types_t::void_type;
        signature.calling_mode = is_variadic ?
                                 vm::ffi_calling_mode_t::c_ellipsis_varargs :
                                 vm::ffi_calling_mode_t::c_default;

        auto result = ffi.register_function(session.result(), signature);
        if (!result) {
            session.error(
                module(),
                "P004",
                fmt::format("unable to find foreign function symbol: {}", symbol_name),
                location());
            return false;
        } else {
            proc_type->foreign_address(reinterpret_cast<uint64_t>(signature.func_ptr));
        }

        return !session.result().is_failed();
    }

    void foreign_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_expression);
    }

};