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

#include "program.h"
#include "attribute.h"
#include "directive.h"
#include "initializer.h"
#include "string_literal.h"
#include "procedure_type.h"

namespace basecode::compiler {

    directive::directive(
            block* parent,
            const std::string& name,
            element* expression) : element(parent, element_type_t::directive),
                                   _name(name),
                                   _expression(expression) {
    }

    bool directive::evaluate(
            common::result& r,
            compiler::program* program) {
        auto it = s_evaluate_handlers.find(_name);
        if (it == s_evaluate_handlers.end())
            return true;
        return it->second(this, r, program);
    }

    bool directive::execute(
            common::result& r,
            compiler::program* program) {
        auto it = s_execute_handlers.find(_name);
        if (it == s_execute_handlers.end())
            return true;
        return it->second(this, r, program);
    }

    element* directive::expression() {
        return _expression;
    }

    std::string directive::name() const {
        return _name;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // run directive

    bool directive::on_execute_run(
            common::result& r,
            compiler::program* program) {
        return true;
    }

    bool directive::on_evaluate_run(
            common::result& r,
            compiler::program* program) {
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // load directive

    bool directive::on_execute_load(
            common::result& r,
            compiler::program* program) {
        return true;
    }

    bool directive::on_evaluate_load(
            common::result& r,
            compiler::program* program) {
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // foreign directive

    bool directive::on_execute_foreign(
            common::result& r,
            compiler::program* program) {
        auto terp = program->terp();

        // XXX: this should move to a cmake generated header file
        std::string library_name = "libalpha-core.dylib";
        // XXX: -------------------------------------------------

        auto library_attribute = attributes().find("library");
        if (library_attribute != nullptr)
            library_name = library_attribute->as_string();

        std::filesystem::path library_path(library_name);
        auto library = terp->load_shared_library(r, library_path);
        if (library == nullptr) {
            return false;
        }

        auto ffi_identifier = dynamic_cast<compiler::identifier*>(_expression);
        std::string symbol_name = ffi_identifier->name();
        auto alias_attribute = attributes().find("alias");
        if (alias_attribute != nullptr) {
            symbol_name = alias_attribute->as_string();
        }

        vm::function_signature_t signature {
            .symbol = symbol_name,
            .library = library,
        };

        auto result = terp->register_foreign_function(r, signature);
        if (!result) {
            r.add_message(
                "P004",
                fmt::format("unable to find foreign function symbol: {}", symbol_name),
                false);
        }

        return !r.is_failed();
    }

    bool directive::on_evaluate_foreign(
            common::result& r,
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