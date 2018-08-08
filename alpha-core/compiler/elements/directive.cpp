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

#include <vm/terp.h>
#include <configure.h>
#include <compiler/session.h>
#include <boost/filesystem.hpp>
#include "attribute.h"
#include "directive.h"
#include "initializer.h"
#include "string_literal.h"
#include "procedure_type.h"
#include "symbol_element.h"

namespace basecode::compiler {

    std::unordered_map<std::string, directive::directive_callable> directive::s_execute_handlers = {
        {"run",     std::bind(&directive::on_execute_run,     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)},
        {"foreign", std::bind(&directive::on_execute_foreign, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)},
    };

    std::unordered_map<std::string, directive::directive_callable> directive::s_evaluate_handlers = {
        {"run",     std::bind(&directive::on_evaluate_run,     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)},
        {"foreign", std::bind(&directive::on_evaluate_foreign, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)},
    };

    ///////////////////////////////////////////////////////////////////////////

    directive::directive(
            block* parent_scope,
            const std::string& name,
            element* expression) : element(parent_scope, element_type_t::directive),
                                   _name(name),
                                   _expression(expression) {
    }

    bool directive::evaluate(
            common::result& r,
            compiler::session& session,
            compiler::program* program) {
        auto it = s_evaluate_handlers.find(_name);
        if (it == s_evaluate_handlers.end())
            return true;
        return it->second(this, r, session, program);
    }

    bool directive::execute(
            common::result& r,
            compiler::session& session,
            compiler::program* program) {
        auto it = s_execute_handlers.find(_name);
        if (it == s_execute_handlers.end())
            return true;
        return it->second(this, r, session, program);
    }

    element* directive::expression() {
        return _expression;
    }

    std::string directive::name() const {
        return _name;
    }

    void directive::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // run directive

    bool directive::on_execute_run(
            common::result& r,
            compiler::session& session,
            compiler::program* program) {
        return true;
    }

    bool directive::on_evaluate_run(
            common::result& r,
            compiler::session& session,
            compiler::program* program) {
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // foreign directive

    bool directive::on_execute_foreign(
            common::result& r,
            compiler::session& session,
            compiler::program* program) {
        auto terp = program->terp();

        std::string library_name;
        auto library_attribute = attributes().find("library");
        if (library_attribute != nullptr) {
            if (!library_attribute->as_string(library_name)) {
                program->error(
                    r,
                    this,
                    "P004",
                    "unable to convert library name.",
                    location());
                return false;
            }
        }

        if (library_name.empty()) {
            program->error(
                r,
                this,
                "P005",
                "library attribute required for foreign directive.",
                location());
            return false;
        }

        vm::shared_library_t* library = nullptr;
        std::stringstream platform_name;
        platform_name
            << SHARED_LIBRARY_PREFIX
            << library_name
            << SHARED_LIBRARY_SUFFIX;
        boost::filesystem::path library_path(platform_name.str());
        library = terp->load_shared_library(r, library_path);
#if defined(__FreeBSD__)
        if (library == nullptr && library_name != COMPILER_LIBRARY_NAME) {
#else
        if (library == nullptr) {
#endif
            // XXX: revisit this at some point
            auto msg = r.find_code("B062");
            if (msg != nullptr) {
                program->error(
                    r,
                    this,
                    "P006",
                    msg->message(),
                    location());
                r.remove_code("B062");
            }
            return false;
        }

        auto ffi_identifier = dynamic_cast<compiler::identifier*>(_expression);
        std::string symbol_name = ffi_identifier->symbol()->name();
        auto alias_attribute = attributes().find("alias");
        if (alias_attribute != nullptr) {
            if (!alias_attribute->as_string(symbol_name)) {
                program->error(
                    r,
                    this,
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

        auto proc_identifier = dynamic_cast<compiler::identifier*>(_expression);
        auto proc_type = proc_identifier->initializer()->procedure_type();
        if (proc_type != nullptr) {
            for (auto param : proc_type->parameters().as_list()) {
                // XXX: need to figure out how to best handle this
                if (param->identifier()->type()->element_type() == element_type_t::any_type)
                    continue;
                vm::function_value_t value;
                value.name = param->identifier()->symbol()->name();
                value.type = vm::ffi_types_t::pointer_type;
                signature.arguments.push_back(value);
            }

            if (proc_type->returns().size() == 0) {
                signature.return_value.type = vm::ffi_types_t::void_type;
            }

            // XXX: this is a hack
            if (proc_type->is_foreign()) {
                signature.calling_mode = vm::ffi_calling_mode_t::c_ellipsis;
            }
        }

        auto result = terp->register_foreign_function(r, signature);
        if (!result) {
            program->error(
                r,
                this,
                "P004",
                fmt::format("unable to find foreign function symbol: {}", symbol_name),
                location());
            return false;
        } else {
            if (proc_type != nullptr)
                proc_type->foreign_address(reinterpret_cast<uint64_t>(signature.func_ptr));
        }

        return !r.is_failed();
    }

    bool directive::on_evaluate_foreign(
            common::result& r,
            compiler::session& session,
            compiler::program* program) {
        auto proc_identifier = dynamic_cast<compiler::identifier*>(_expression);
        auto proc_type = proc_identifier->initializer()->procedure_type();
        if (proc_type != nullptr) {
            auto attrs = proc_type->attributes().as_list();
            for (auto attr : attrs) {
                attributes().add(attr);
                proc_type->attributes().remove(attr->name());
            }
            proc_type->is_foreign(true);
            return true;
        }
        return false;
    }

};