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

    bool directive::execute(
            common::result& r,
            compiler::program* program) {
        auto it = s_directive_handlers.find(_name);
        if (it == s_directive_handlers.end())
            return true;
        return it->second(this, r, program);
    }

    element* directive::expression() {
        return _expression;
    }

    std::string directive::name() const {
        return _name;
    }

    bool directive::on_run(common::result& r, compiler::program* program) {
        return true;
    }

    bool directive::on_load(common::result& r, compiler::program* program) {
        return true;
    }

    bool directive::on_foreign(common::result& r, compiler::program* program) {
        auto terp = program->terp();

        // XXX: this sucks; it must be fixed!
//            "/Users/jeff/src/basecode-lang/bootstrap/build/debug/bin/bootstrap");
        auto library = terp->load_shared_library(r, "bootstrap");
        if (library == nullptr) {
            return false;
        }
        terp->dump_shared_libraries();

        auto ffi_identifier = dynamic_cast<compiler::identifier*>(_expression);
        auto proc_type = dynamic_cast<compiler::procedure_type*>(ffi_identifier->initializer()->expression());

        std::string symbol_name = ffi_identifier->name();
        auto alias_attribute = proc_type->attributes().find("alias");
        if (alias_attribute != nullptr) {
            symbol_name = dynamic_cast<compiler::string_literal*>(alias_attribute->expression())->value();
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
                true);
        }

        return result;
    }

};