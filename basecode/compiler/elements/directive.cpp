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
#include <boost/filesystem.hpp>
#include "type.h"
#include "intrinsic.h"
#include "attribute.h"
#include "directive.h"
#include "raw_block.h"
#include "assignment.h"
#include "declaration.h"
#include "initializer.h"
#include "string_literal.h"
#include "procedure_type.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    std::unordered_map<std::string, directive::directive_callable> directive::s_execute_handlers = {
        {"run",         std::bind(&directive::on_execute_run,     std::placeholders::_1, std::placeholders::_2)},
        {"type",        std::bind(&directive::on_execute_type,    std::placeholders::_1, std::placeholders::_2)},
        {"foreign",     std::bind(&directive::on_execute_foreign, std::placeholders::_1, std::placeholders::_2)},
        {"assembly",    std::bind(&directive::on_execute_assembly, std::placeholders::_1, std::placeholders::_2)},
        {"intrinsic",   std::bind(&directive::on_execute_intrinsic, std::placeholders::_1, std::placeholders::_2)},
    };

    std::unordered_map<std::string, directive::directive_callable> directive::s_evaluate_handlers = {
        {"run",         std::bind(&directive::on_evaluate_run,     std::placeholders::_1, std::placeholders::_2)},
        {"type",        std::bind(&directive::on_evaluate_type,    std::placeholders::_1, std::placeholders::_2)},
        {"foreign",     std::bind(&directive::on_evaluate_foreign, std::placeholders::_1, std::placeholders::_2)},
        {"assembly",    std::bind(&directive::on_evaluate_assembly, std::placeholders::_1, std::placeholders::_2)},
        {"intrinsic",   std::bind(&directive::on_evaluate_intrinsic, std::placeholders::_1, std::placeholders::_2)},
    };

    ///////////////////////////////////////////////////////////////////////////

    directive::directive(
            compiler::module* module,
            block* parent_scope,
            const std::string& name,
            element* expression) : element(module, parent_scope, element_type_t::directive),
                                   _name(name),
                                   _expression(expression) {
    }

    bool directive::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        if (_instruction_block != nullptr) {
            auto current_block = session.assembler().current_block();
            current_block->comment("*** begin: inline assembly block", 4);
            for (const auto& entry : _instruction_block->entries())
                current_block->add_entry(entry);
            current_block->comment("*** end: inline assembly block", 4);
        }
        return true;
    }

    bool directive::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        if (_expression != nullptr) {
            auto type_ref = dynamic_cast<compiler::type_reference*>(_expression);
            result.inferred_type = type_ref->type();
            result.reference = type_ref;
            return true;
        }
        return false;
    }

    element* directive::expression() {
        return _expression;
    }

    std::string directive::name() const {
        return _name;
    }

    bool directive::on_is_constant() const {
        return _expression != nullptr
            && _expression->element_type() == element_type_t::type_reference;
    }

    bool directive::execute(compiler::session& session) {
        auto it = s_execute_handlers.find(_name);
        if (it == s_execute_handlers.end()) {
            session.error(
                "P044",
                fmt::format("unknown directive: {}", _name),
                location());
            return false;
        }
        return it->second(this, session);
    }

    bool directive::evaluate(compiler::session& session) {
        auto it = s_evaluate_handlers.find(_name);
        if (it == s_evaluate_handlers.end()) {
            session.error(
                "P044",
                fmt::format("unknown directive: {}", _name),
                location());
            return false;
        }
        return it->second(this, session);
    }

    void directive::on_owned_elements(element_list_t& list) {
        if (_expression != nullptr)
            list.emplace_back(_expression);
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // type directive

    bool directive::on_execute_type(compiler::session& session) {
        return true;
    }

    bool directive::on_evaluate_type(compiler::session& session) {
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // assembly directive

    bool directive::on_execute_assembly(compiler::session& session) {
        auto raw_block = dynamic_cast<compiler::raw_block*>(_expression);
        if (raw_block == nullptr) {
            return false;
        }

        common::source_file source_file;
        if (!source_file.load(session.result(), raw_block->value() + "\n"))
            return false;

        auto& assembler = session.assembler();
        auto success = assembler.assemble_from_source(
            session.result(),
            source_file,
            session.stack_frame());
        if (success) {
            // XXX:  this is so evil
            _instruction_block = assembler.blocks().back();
            _instruction_block->should_emit(false);
        }
        return success;
    }

    bool directive::on_evaluate_assembly(compiler::session& session) {
        auto is_valid = _expression != nullptr
            && _expression->element_type() == element_type_t::raw_block;
        if (!is_valid) {
            session.error(
                this,
                "P004",
                "#assembly expects a valid raw block expression.",
                location());
        }
        return is_valid;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // run directive

    bool directive::on_execute_run(compiler::session& session) {
        return true;
    }

    bool directive::on_evaluate_run(compiler::session& session) {
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // foreign directive

    bool directive::on_execute_foreign(compiler::session& session) {
        auto& ffi = session.ffi();

        std::string library_name;
        auto library_attribute = find_attribute("library");
        if (library_attribute != nullptr) {
            if (!library_attribute->as_string(library_name)) {
                session.error(
                    this,
                    "P004",
                    "unable to convert library name.",
                    location());
                return false;
            }
        }

        if (library_name.empty()) {
            session.error(
                this,
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
                    this,
                    "P006",
                    msg->message(),
                    location());
                session.result().remove_code("B062");
            }
            return false;
        }
        library->self_loaded(library_name == COMPILER_LIBRARY_NAME);

        auto assignment = dynamic_cast<compiler::assignment*>(_expression);
        auto ffi_decl = dynamic_cast<compiler::declaration*>(assignment->expressions()[0]);
        std::string symbol_name = ffi_decl->identifier()->symbol()->name();
        auto alias_attribute = attributes().find("alias");
        if (alias_attribute != nullptr) {
            if (!alias_attribute->as_string(symbol_name)) {
                session.error(
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

        auto proc_type = ffi_decl->identifier()->initializer()->procedure_type();
        if (proc_type != nullptr) {
            auto params_list = proc_type->parameters().as_list();
            for (auto param : params_list) {
                // XXX: need to figure out how to best handle this
                if (param->identifier()->type_ref()->is_any_type())
                    continue;
                vm::function_value_t value;
                value.name = param->identifier()->symbol()->name();
                value.type = vm::ffi_types_t::pointer_type;
                signature.arguments.push_back(value);
            }

            if (proc_type->return_type() == nullptr) {
                signature.return_value.type = vm::ffi_types_t::void_type;
            }

            // XXX: this is a hack
            if (proc_type->is_foreign()) {
                signature.calling_mode = vm::ffi_calling_mode_t::c_ellipsis;
            }
        }

        auto result = ffi.register_function(session.result(), signature);
        if (!result) {
            session.error(
                this,
                "P004",
                fmt::format("unable to find foreign function symbol: {}", symbol_name),
                location());
            return false;
        } else {
            if (proc_type != nullptr)
                proc_type->foreign_address(reinterpret_cast<uint64_t>(signature.func_ptr));
        }

        return !session.result().is_failed();
    }

    bool directive::on_evaluate_foreign(compiler::session& session) {
        auto assignment = dynamic_cast<compiler::assignment*>(_expression);
        auto proc_decl = dynamic_cast<compiler::declaration*>(assignment->expressions()[0]);
        if (proc_decl == nullptr)
            return false;

        auto proc_type = proc_decl->identifier()->initializer()->procedure_type();
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

    ///////////////////////////////////////////////////////////////////////////
    //
    // intrinsic directive

    bool directive::on_execute_intrinsic(compiler::session& session) {
        return true;
    }

    bool directive::on_evaluate_intrinsic(compiler::session& session) {
        auto assignment = dynamic_cast<compiler::assignment*>(_expression);
        auto proc_decl = dynamic_cast<compiler::declaration*>(assignment->expressions()[0]);
        if (proc_decl == nullptr)
            return false;

        auto proc_type = proc_decl->identifier()->initializer()->procedure_type();

        std::string intrinsic_name;
        auto attr = find_attribute("intrinsic_name");
        if (attr != nullptr) {
            if (!attr->as_string(intrinsic_name)) {
                session.error(
                    this,
                    "P004",
                    "unable to convert intrinsic name.",
                    location());
                return false;
            }
        }

        if (intrinsic_name.empty()) {
            session.error(
                this,
                "P005",
                "intrinsic_name attribute required for intrinsic directive.",
                location());
            return false;
        }

        if (!compiler::intrinsic::register_intrinsic_procedure_type(
                intrinsic_name,
                proc_type)) {
            return false;
        }

        return true;
    }

};