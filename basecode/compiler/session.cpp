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

#include <fstream>
#include <utility>
#include <vm/ffi.h>
#include <vm/terp.h>
#include <vm/assembler.h>
#include <parser/parser.h>
#include <compiler/api/kernel.h>
#include <vm/default_allocator.h>
#include <debugger/environment.h>
#include <vm/register_allocator.h>
#include "session.h"
#include "elements.h"
#include "element_map.h"
#include "ast_evaluator.h"
#include "scope_manager.h"
#include "element_builder.h"
#include "string_intern_map.h"
#include "byte_code_emitter.h"
#include "code_dom_formatter.h"

namespace basecode::compiler {

    session::session(
            const session_options_t& options,
            path_list_t source_files) : _ffi(new vm::ffi(options.ffi_heap_size)),
                                        _terp(new vm::terp(
                                            _ffi,
                                            options.allocator,
                                            options.heap_size,
                                            options.stack_size)),
                                        _source_files(std::move(source_files)),
                                        _options(options),
                                        _elements(new element_map()),
                                        _labels(new vm::label_map()),
                                        _builder(new element_builder(*this)),
                                        _ast_evaluator(new ast_evaluator(*this)),
                                        _ast_builder(new syntax::ast_builder()),
                                        _interned_strings(new string_intern_map()),
                                        _emitter(new compiler::byte_code_emitter(*this)),
                                        _scope_manager(new compiler::scope_manager(*this)),
                                        _register_allocator(new vm::register_allocator()) {
        _assembler = new vm::assembler(_terp, _register_allocator);
    }

    session::~session() {
        delete _register_allocator;
        delete _scope_manager;
        delete _emitter;
        delete _interned_strings;
        delete _ast_builder;
        delete _ast_evaluator;
        delete _assembler;
        delete _builder;
        delete _labels;
        delete _elements;
        delete _terp;
        delete _ffi;
    }

    bool session::run() {
        auto bss_address = _terp->heap_vector(vm::heap_vectors_t::bss_start);
        auto bss_length = _terp->heap_vector(vm::heap_vectors_t::bss_length);

        if (bss_address != 0 && bss_length > 0) {
            for (size_t i = 0; i < bss_length; i++)
                _terp->write(vm::op_sizes::byte, bss_address + i, 0);
        }

        return _terp->run(_result);
    }

    void session::error(
            compiler::module* module,
            const std::string& code,
            const std::string& message,
            const common::source_location& location) {
        if (module != nullptr) {
            auto file = module->source_file();
            if (file != nullptr) {
                file->error(_result, code, message, location);
                return;
            }
        }
        _result.error(code, message, location);
    }

    vm::ffi& session::ffi() {
        return *_ffi;
    }

    bool session::compile() {
        auto& listing = _assembler->listing();

        time_task(
            session_task_category_t::assembler,
            "preparation",
            [&](session_task_t* task) {
                auto listing_name = _source_files
                    .front()
                    .filename()
                    .replace_extension(".basm");
                listing.add_source_file(listing_name.string());
                listing.select_source_file(listing_name.string());
                return true;
            });

        time_task(
            session_task_category_t::compiler,
            "preparation",
            [&](session_task_t* task) {
                _program->block(_scope_manager->push_new_block());
                _program->block()->parent_element(_program);
                return true;
            });

        time_task(
            session_task_category_t::compiler,
            "core types",
            [&](session_task_t* task) {
                initialize_core_types();
                return true;
            });

        auto success = time_task(
            session_task_category_t::compiler,
            "generate model",
            [&](session_task_t* task) {
                for (const auto& file : source_files()) {
                    auto module = compile_module(add_source_file(file));
                    if (module == nullptr)
                        return false;
                }
                return true;
            });
        if (!success)
            return false;

        success = time_task(
            session_task_category_t::compiler,
            "resolve unknown types (phase 1)",
            [&](session_task_t* task) { return resolve_unknown_types(false); });
        if (!success)
            return false;

        success = time_task(
            session_task_category_t::compiler,
            "resolve unknown identifiers",
            [&](session_task_t* task) {
                return resolve_unknown_identifiers();
            });
        if (!success)
            return false;

        success = time_task(
            session_task_category_t::compiler,
            "resolve unknown types (phase 2)",
            [&](session_task_t* task) { return resolve_unknown_types(false); });
        if (!success)
            return false;

        success = time_task(
            session_task_category_t::compiler,
            "constant expression folding",
            [&](session_task_t* task) { return fold_constant_expressions(); });
        if (!success)
            return false;

        success = time_task(
            session_task_category_t::compiler,
            "type check",
            [&](session_task_t* task) { return type_check(); });
        if (!success)
            return false;

        if (!_result.is_failed()) {
            time_task(
                session_task_category_t::byte_code_emitter,
                "generate byte-code",
                [&](session_task_t* task) {
                    _emitter->emit();
                    return true;
                });

            success = time_task(
                session_task_category_t::assembler,
                "encode byte-code",
                [&](session_task_t* task) {
                    return _assembler->assemble(_result, *_labels);
                });

            if (_options.verbose) {
                time_task(
                    session_task_category_t::assembler,
                    "listing file",
                    [&](session_task_t* task) {
                        disassemble(stdout);
                        fmt::print("\n");
                        return true;
                    });
            }

            if (success) {
                success = time_task(
                    session_task_category_t::compiler,
                    "execute directives",
                    [&](session_task_t* task) { return execute_directives(); });
                if (success) {
                    if (_options.debugger) {
#if DEBUGGER_ENABLED
                        disassemble(nullptr);
                        debugger::environment env(*this);
                        if (!env.initialize(_result))
                            return false;
                        env.run(_result);
                        env.shutdown(_result);
#else
                        fmt::print("\nNOTE: Debugger not enabled.\n");
                        fmt::print("      Ensure you have ncurses installed and rebuild the compiler.\n");
#endif
                    } else {
                        if (_run) {
                            success = time_task(
                                session_task_category_t::virtual_machine,
                                "execute byte-code",
                                [&](session_task_t* task) { return run(); });
                            if (!success)
                                return false;
                        }
                    }
                }
            }
        }

        return !_result.is_failed();
    }

    bool session::time_task(
            session_task_category_t category,
            const std::string& name,
            const session_task_callable_t& callable) {
        auto parent_task = current_task();

        session_task_t* task = nullptr;
        if (parent_task != nullptr) {
            task = &parent_task->subtasks.emplace_back(name, category);
        } else {
            task = &_tasks.emplace_back(name, category);
        }

        _task_stack.push(task);
        defer(_task_stack.pop());

        auto start = std::chrono::high_resolution_clock::now();
        auto success = callable(task);
        auto end = std::chrono::high_resolution_clock::now();

        task->elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        return success;
    }

    vm::terp& session::terp() {
        return *_terp;
    }

    void session::raise_phase(
            session_compile_phase_t phase,
            session_module_type_t module_type,
            const boost::filesystem::path& source_file) {
        if (_options.compile_callback == nullptr)
            return;
        _options.compile_callback(phase, module_type, source_file);
    }

    void session::finalize() {
        try {
            if (!_options.dom_graph_file.empty()) {
                time_task(
                    session_task_category_t::compiler,
                    "write code dom graph file",
                    [&](session_task_t* task) {
                        write_code_dom_graph(_options.dom_graph_file);
                        return true;
                    });
            }

            api::g_session = nullptr;
        } catch (const fmt::format_error& e) {
            fmt::print("fmt::format_error caught: {}\n", e.what());
        }
    }

    bool session::type_check() {
        bool success;

        success = time_task(
            session_task_category_t::compiler,
            "intrinsic call sites",
            [&](session_task_t* task) {
                auto intrinsics = _elements->find_by_type<compiler::intrinsic>(element_type_t::intrinsic);
                for (auto intrinsic : intrinsics) {
                    auto args = intrinsic->arguments();
                    prepare_call_site_result_t result {};
                    if (!intrinsic->procedure_type()->prepare_call_site(
                            *this,
                            intrinsic->uniform_function_call(),
                            args,
                            result)) {
                        for (const auto& msg : result.messages.messages())
                            error(intrinsic->module(), msg.code(), msg.message(), msg.location());
                        return false;
                    }
                    args->elements(result.arguments);
                    args->argument_index(result.index);
                }
                return true;
            });
        if (!success)
            return false;

        success = time_task(
            session_task_category_t::compiler,
            "procedure call sites",
            [&](session_task_t* task) {
                auto proc_calls = _elements->find_by_type<compiler::procedure_call>(element_type_t::proc_call);
                for (auto proc_call : proc_calls) {
                    if (!proc_call->resolve_overloads(*this)) {
                        error(
                            proc_call->module(),
                            "X000",
                            "unable to prepare procedure call site.",
                            proc_call->location());
                        return false;
                    }
                }
                return true;
            });
        if (!success)
            return false;

        success = time_task(
            session_task_category_t::compiler,
            "resolve unknown types (phase 3)",
            [&](session_task_t* task) { return resolve_unknown_types(true); });
        if (!success) {
            error(
                nullptr,
                "X000",
                "unable to resolve unknown types (phase 3).",
                _program->location());
            return false;
        }

        type_check_options_t type_check_options {};
        type_check_options.strict = false;

        auto identifiers = _elements->find_by_type<compiler::identifier>(element_type_t::identifier);
        for (auto var : identifiers) {
            auto init = var->initializer();
            if (init == nullptr)
                continue;

            auto expr = init->expression();
            if (expr != nullptr
            &&  expr->element_type() == element_type_t::uninitialized_literal) {
                continue;
            }

            infer_type_result_t type_result {};
            if (!init->infer_type(*this, type_result)) {
                // XXX: error
                return false;
            }

            const auto& inferred = type_result.types.back();

            auto var_type = var->type_ref()->type();
            if (!var_type->type_check(inferred.type, type_check_options)) {
                error(
                    init->module(),
                    "C051",
                    fmt::format(
                        "type mismatch: cannot assign {} to {}.",
                        inferred.type_name(),
                        var->type_ref()->symbol()->name()),
                    var->location());
            }
        }

        // XXX: fix type checking for multiple assignment from multiple return values
        auto binary_ops = _elements->find_by_type<compiler::binary_operator>(element_type_t::binary_operator);
        for (auto binary_op : binary_ops) {
            if (binary_op->operator_type() != operator_type_t::assignment)
                continue;

            infer_type_result_t lhs_type_result {};
            if (!binary_op->lhs()->infer_type(*this, lhs_type_result)) {
                error(
                    binary_op->lhs()->module(),
                    "P052",
                    "unable to infer type.",
                    binary_op->lhs()->location());
                return false;
            }

            infer_type_result_t rhs_type_result {};
            if (!binary_op->rhs()->infer_type(*this, rhs_type_result)) {
                error(
                    binary_op->rhs()->module(),
                    "P052",
                    "unable to infer type.",
                    binary_op->rhs()->location());
                return false;
            }

            const auto& lhs_inferred = lhs_type_result.types.back();
            const auto& rhs_inferred = rhs_type_result.types.back();

            type_check_options.strict =
                !(binary_op->rhs()->element_type() == element_type_t::integer_literal
                    || binary_op->rhs()->element_type() == element_type_t::float_literal);
            if (!lhs_inferred.type->type_check(rhs_inferred.type, type_check_options)) {
                error(
                    binary_op->module(),
                    "C051",
                    fmt::format(
                        "type mismatch: cannot assign {} to {}.",
                        rhs_inferred.type_name(),
                        lhs_inferred.type_name()),
                    binary_op->rhs()->location());
            }
        }

        return !_result.is_failed();
    }

    bool session::initialize() {
        api::g_session = this;

        _tasks.reserve(256);

        _program = _builder->make_program(nullptr, nullptr);

        _ffi->initialize(_result);
        _terp->initialize(_result);

        _assembler->resolver(std::bind(&session::resolve_assembly_symbol,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4));
        _assembler->initialize(_result);

        _terp->register_trap(trap_putc, [](vm::terp* terp) {
            vm::register_value_alias_t fp_alias {};
            fp_alias.qw = terp->pop();

            vm::register_value_alias_t ch_alias {};
            ch_alias.qw = terp->pop();

            fputc(ch_alias.dw, stdout);
        });

        _terp->register_trap(trap_getc, [](vm::terp* terp) {
            vm::register_value_alias_t fp_alias {};
            fp_alias.qw = terp->pop();

            auto ch = fgetc(stdin);
            terp->push(ch);
        });

        return !_result.is_failed();
    }

    void session::enable_run() {
        _run = true;
    }

    element_map& session::elements() {
        return *_elements;
    }

    vm::label_map& session::labels() {
        return *_labels;
    }

    string_set_t& session::strings() {
        return _strings;
    }

    common::result& session::result() {
        return _result;
    }

    type_list_t session::used_types() {
        type_list_t list {};
        for (auto type : _used_types)
            list.emplace_back(type);
        return list;
    }

    bool session::execute_directives() {
        auto directives = _elements->find_by_type<compiler::directive>(element_type_t::directive);
        for (auto directive_element : directives) {
            if (directive_element->is_parent_type_one_of({element_type_t::directive}))
                continue;

            if (!directive_element->execute(*this)) {
                error(
                    directive_element->module(),
                    "P044",
                    fmt::format(
                        "directive failed to execute: {}",
                        directive_type_to_name(directive_element->type())),
                    directive_element->location());
                return false;
            }
        }
        return true;
    }

    vm::assembler& session::assembler() {
        return *_assembler;
    }

    element_builder& session::builder() {
        return *_builder;
    }

    vm::allocator* session::allocator() {
        return _options.allocator;
    }

    bool session::resolve_assembly_symbol(
            vm::assembly_symbol_type_t type,
            void* data,
            const std::string& symbol,
            vm::assembly_symbol_result_t& result) {
        auto scope = reinterpret_cast<compiler::block*>(data);
        switch (type) {
            case vm::assembly_symbol_type_t::module: {
                auto vars = _scope_manager->find_identifier(
                    make_qualified_symbol(symbol),
                    scope);
                compiler::identifier* var = vars.empty() ? nullptr : vars.front();
                if (var != nullptr) {
                    if (var->is_constant()) {
                        switch (var->type_ref()->type()->element_type()) {
                            case element_type_t::bool_type: {
                                bool value;
                                if (var->as_bool(value)) {
                                    result.data(vm::compiler_module_data_t(
                                        static_cast<uint64_t>(value ? 1 : 0)));
                                }
                                break;
                            }
                            case element_type_t::numeric_type: {
                                auto size = var->type_ref()->type()->size_in_bytes();
                                auto number_class = var->type_ref()->type()->number_class();

                                if (number_class == number_class_t::integer) {
                                    uint64_t value;
                                    if (var->as_integer(value)) {
                                        result.data(vm::compiler_module_data_t(value));
                                    }
                                } else {
                                    double value;
                                    if (var->as_float(value)) {
                                        if (size == 4)
                                            result.data(vm::compiler_module_data_t((float)value));
                                        else
                                            result.data(vm::compiler_module_data_t(value));
                                    }
                                }
                                break;
                            }
                            default:
                                break;
                        }
                    }

                    if (!result.is_set()) {
                        result.data(vm::compiler_module_data_t(var->label_name()));
                    }

                    return true;
                }
                break;
            }
            case vm::assembly_symbol_type_t::assembler: {
                auto vars = _scope_manager->find_identifier(
                    make_qualified_symbol(symbol),
                    scope);
                compiler::identifier* var = vars.empty() ? nullptr : vars.front();
                if (var != nullptr) {
                    result.data(vm::compiler_module_data_t(var->label_name()));
                    return true;
                }
                break;
            }
            default: {
                break;
            }
        }
        return false;
    }

    compiler::program& session::program() {
        return *_program;
    }

    void session::initialize_core_types() {
        auto parent_scope = _scope_manager->current_scope();

        compiler::numeric_type::make_types(*this, parent_scope);
        for (const auto& type_name : compiler::numeric_type::type_names()) {
            auto base_type = _scope_manager->find_type(qualified_symbol_t(type_name));
            auto it = _strings.insert(compiler::pointer_type::name_for_pointer(base_type));
            _builder->make_pointer_type(
                parent_scope,
                qualified_symbol_t(*it.first),
                base_type);
        }
        _scope_manager->add_type_to_scope(_builder->make_module_type(
            parent_scope,
            _builder->make_block(parent_scope->module(), parent_scope)));
        _scope_manager->add_type_to_scope(_builder->make_namespace_type(parent_scope));
        _scope_manager->add_type_to_scope(_builder->make_bool_type(parent_scope));
        _scope_manager->add_type_to_scope(_builder->make_rune_type(parent_scope));
        _scope_manager->add_type_to_scope(_builder->make_generic_type(
            parent_scope,
            {}));
    }

    bool session::resolve_unknown_types(bool final) {
        auto& identifiers = _scope_manager->identifiers_with_unknown_types();
        auto remaining = identifiers.size();

        std::set<common::id_t> to_remove {};

        auto it = identifiers.begin();
        while (it != identifiers.end()) {
            auto var = *it;

            if (var->type_ref() != nullptr
            && !var->type_ref()->is_unknown_type()) {
                it = identifiers.erase(it);
                remaining--;
                continue;
            }

            compiler::pointer_type* pointer = nullptr;
            compiler::unknown_type* unknown_type = nullptr;
            auto is_pointer = var->type_ref()->is_pointer_type();

            if (is_pointer) {
                pointer = dynamic_cast<compiler::pointer_type*>(var->type_ref()->type());
                unknown_type = dynamic_cast<compiler::unknown_type*>(pointer->base_type_ref()->type());
            } else {
                unknown_type = dynamic_cast<compiler::unknown_type*>(var->type_ref()->type());
            }

            infer_type_result_t type_result {};

            if (var->is_parent_type_one_of({element_type_t::binary_operator})) {
                auto binary_operator = dynamic_cast<compiler::binary_operator*>(var->parent_element());
                switch (binary_operator->operator_type()) {
                    case operator_type_t::assignment: {
                        if (!binary_operator->rhs()->infer_type(*this, type_result))
                            return false;
                        const auto& inferred = type_result.types.back();
                        var->type_ref(inferred.ref);
                        break;
                    }
                    default: {
                        break;
                    }
                }
            } else {
                compiler::element* expr = nullptr;
                auto init = var->initializer();
                if (init != nullptr)
                    expr = init->expression();
                else
                    expr = unknown_type->expression();

                if (expr == nullptr || is_pointer) {
                    auto type = _scope_manager->find_type(
                        unknown_type->symbol()->qualified_symbol(),
                        var->parent_scope());
                    if (type != nullptr) {
                        auto type_ref = _builder->make_type_reference(
                            type->parent_scope(),
                            qualified_symbol_t{},
                            type,
                            true);
                        if (is_pointer) {
                            pointer->base_type_ref(type_ref);
                        } else {
                            var->type_ref(type_ref);
                        }
                        to_remove.insert(unknown_type->id());
                    }
                } else {
                    if (!expr->infer_type(*this, type_result))
                        return false;
                    const auto& inferred = type_result.types.back();
                    if (inferred.ref != nullptr)
                        var->type_ref(inferred.ref);
                    else {
                        if (!final)
                            remaining--;
                    }
                }
            }

            auto type_ref = var->type_ref();
            if (type_ref != nullptr && !type_ref->is_unknown_type()) {
                var->inferred_type(true);
                it = identifiers.erase(it);
                remaining--;
            } else {
                ++it;
                if (final || unknown_type->expression() == nullptr) {
                    error(
                        var->module(),
                        "P004",
                        fmt::format(
                            "unable to resolve type for identifier: {}",
                            var->symbol()->name()),
                        var->symbol()->location());
                }
            }
        }

        for (auto id : to_remove)
            _elements->remove(id);

        return remaining == 0;
    }

    ast_evaluator& session::evaluator() {
        return *_ast_evaluator;
    }

    void session::disassemble(FILE* file) {
        _assembler->disassemble();
        if (file != nullptr) {
            fmt::print(file, "\n");
            _assembler->listing().write(file);
        }
    }

    session_task_t* session::current_task() {
        if (_task_stack.empty())
            return nullptr;
        return _task_stack.top();
    }

    bool session::fold_constant_expressions() {
        return fold_elements_of_type(element_type_t::intrinsic)
            && fold_elements_of_type(element_type_t::identifier_reference)
            && fold_elements_of_type(element_type_t::unary_operator)
            && fold_elements_of_type(element_type_t::binary_operator)
            && fold_elements_of_type(element_type_t::label_reference);
    }

    bool session::resolve_unknown_identifiers() {
        auto& unresolved = _scope_manager->unresolved_identifier_references();
        auto it = unresolved.begin();
        while (it != unresolved.end()) {
            auto unresolved_reference = *it;
            if (unresolved_reference->resolved()) {
                it = unresolved.erase(it);
                continue;
            }

            compiler::block* type_scope = unresolved_reference->parent_scope();
            if (unresolved_reference->is_parent_type_one_of({element_type_t::binary_operator})) {
                auto bin_op = dynamic_cast<compiler::binary_operator*>(unresolved_reference->parent_element());
                if (bin_op->operator_type() == operator_type_t::member_access) {
                    infer_type_result_t type_result;
                    if (!bin_op->lhs()->infer_type(*this, type_result)) {
                        error(
                            unresolved_reference->module(),
                            "X000",
                            "unable to infer lhs of member access operator.",
                            unresolved_reference->symbol().location);
                        return false;
                    }

                    const auto& inferred = type_result.types.back();
                    if (!inferred.type->is_composite_type()) {
                        error(
                            unresolved_reference->module(),
                            "X000",
                            "member access requires lhs composite type.",
                            unresolved_reference->symbol().location);
                        return false;
                    }
                    auto composite_type = dynamic_cast<compiler::composite_type*>(inferred.type);
                    type_scope = composite_type->scope();
                }
            }

            compiler::identifier* identifier = nullptr;

            auto vars = _scope_manager->find_identifier(
                unresolved_reference->symbol(),
                type_scope);
            if (vars.size() > 1) {
                if (unresolved_reference->is_parent_type_one_of({element_type_t::proc_call})) {
                    auto proc_call = dynamic_cast<compiler::procedure_call*>(unresolved_reference->parent_element());
                    unresolved_reference->identifier(vars[0]);

                    auto new_refs = proc_call->references();
                    for (size_t i = 1; i < vars.size(); i++) {
                        new_refs.emplace_back(_builder->make_identifier_reference(
                            unresolved_reference->parent_scope(),
                            vars[i]->symbol()->qualified_symbol(),
                            vars[i]));
                    }
                    proc_call->references(new_refs);
                }
            } else {
                identifier = vars.empty() ? nullptr : vars.front();
                if (identifier == nullptr) {
                    if (unresolved_reference->symbol().is_qualified()) {
                        vars = _scope_manager->find_identifier(
                            unresolved_reference->symbol(),
                            type_scope);
                        identifier = vars.empty() ? nullptr : vars.front();
                    }
                }

                if (identifier != nullptr) {
                    unresolved_reference->identifier(identifier);
                } else {
                    ++it;
                    error(
                        unresolved_reference->module(),
                        "P004",
                        fmt::format(
                            "unable to resolve identifier: {}",
                            unresolved_reference->symbol().name),
                        unresolved_reference->symbol().location);
                    continue;
                }
            }

            it = unresolved.erase(it);
        }

        return unresolved.empty();
    }

    syntax::ast_builder& session::ast_builder() {
        return *_ast_builder;
    }

    const element_map& session::elements() const {
        return *_elements;
    }

    common::source_file* session::pop_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        auto source_file = _source_file_stack.top();
        _source_file_stack.pop();
        return source_file;
    }

    const path_list_t& session::source_files() const {
        return _source_files;
    }

    void session::track_used_type(compiler::type* type) {
        const element_type_set_t invalid_types = {
            element_type_t::unknown_type,
            element_type_t::generic_type
        };

        if (type == nullptr
        ||  type->is_type_one_of(invalid_types)) {
            return;
        }

        compiler::type* base_type = nullptr;
        switch (type->element_type()) {
            case element_type_t::pointer_type: {
                auto pointer_type = dynamic_cast<compiler::pointer_type*>(type);
                base_type = pointer_type->base_type_ref()->type();
                break;
            }
            case element_type_t::array_type: {
                auto array_type = dynamic_cast<compiler::array_type*>(type);
                base_type = array_type->base_type_ref()->type();
                break;
            }
            default: {
                break;
            }
        }

        if (base_type != nullptr) {
            if (base_type->is_type_one_of(invalid_types))
                return;
        }

        _used_types.insert(type);
    }

    vm::register_allocator* session::register_allocator() {
        return _register_allocator;
    }

    bool session::fold_elements_of_type(element_type_t type) {
        std::vector<common::id_t> to_remove {};

        auto elements = _elements->find_by_type<compiler::element>(type);
        for (auto e : elements) {
            if (e->element_type() == element_type_t::intrinsic) {
                auto intrinsic = dynamic_cast<compiler::intrinsic*>(e);
                if (!intrinsic->can_fold())
                    continue;
            }

            fold_result_t fold_result {};
            if (!e->fold(*this, fold_result))
                continue;

            if (fold_result.element == nullptr)
                continue;

            auto parent = e->parent_element();
            if (parent != nullptr) {
                switch (e->element_type()) {
                    case element_type_t::intrinsic: {
                        auto intrinsic = dynamic_cast<compiler::intrinsic*>(e);
                        fold_result.element->attributes().add(_builder->make_attribute(
                            _scope_manager->current_scope(),
                            "intrinsic_substitution"sv,
                            _builder->make_string(
                                _scope_manager->current_scope(),
                                intrinsic_type_to_name(intrinsic->type()))));
                        break;
                    }
                    default:
                        break;
                }

                fold_result.element->parent_element(parent);
                if (!parent->apply_fold_result(e, fold_result)) {
                    error(
                        e->module(),
                        "X000",
                        fmt::format(
                            "element does not implement apply_fold_result: {}",
                            element_type_name(e->element_type())),
                        e->location());
                    return false;
                }

                to_remove.push_back(e->id());
            }
        }

        for (auto id : to_remove)
            _elements->remove(id);

        return true;
    }

    compiler::scope_manager& session::scope_manager() {
        return *_scope_manager;
    }

    const session_options_t& session::options() const {
        return _options;
    }

    const compiler::program& session::program() const {
        return *_program;
    }

    const session_task_list_t& session::tasks() const {
        return _tasks;
    }

    common::source_file* session::current_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        return _source_file_stack.top();
    }

    compiler::byte_code_emitter& session::byte_code_emitter() {
        return *_emitter;
    }

    const string_intern_map& session::interned_strings() const {
        return *_interned_strings;
    }

    common::source_file* session::source_file(common::id_t id) {
        auto it = _source_file_registry.find(id);
        if (it == _source_file_registry.end())
            return nullptr;
        return &it->second;
    }

    const compiler::scope_manager& session::scope_manager() const {
        return *_scope_manager;
    }

    bool session::should_read_variable(compiler::element* element) {
        if (element == nullptr)
            return false;

        switch (element->element_type()) {
            case element_type_t::identifier_reference: return false;
            default: break;
        }

        return true;
    }

    void session::push_source_file(common::source_file* source_file) {
        _source_file_stack.push(source_file);
    }

    common::id_t session::intern_string(compiler::string_literal* literal) {
        return _interned_strings->intern(literal);
    }

    void session::write_code_dom_graph(const boost::filesystem::path& path) {
        FILE* output_file = nullptr;
        if (!path.empty()) {
            output_file = fopen(path.string().c_str(), "wt");
        }
        defer({
            if (output_file != nullptr)
                fclose(output_file);
        });

        compiler::code_dom_formatter formatter(*this, output_file);
        formatter.format(fmt::format("Code DOM Graph: {}", path.string()));
    }

    compiler::module* session::compile_module(common::source_file* source_file) {
        auto path_string = source_file->path().string();
        auto it = _modules.find(path_string);
        if (it != _modules.end())
            return it->second;

        auto is_root = current_source_file() == nullptr;
        auto module_type = is_root ?
            session_module_type_t::program :
            session_module_type_t::module;

        push_source_file(source_file);
        raise_phase(
            session_compile_phase_t::start,
            module_type,
            source_file->path());

        defer({
            pop_source_file();
            raise_phase(
                _result.is_failed() ?
                    session_compile_phase_t::failed :
                    session_compile_phase_t::success,
                module_type,
                source_file->path());
        });

        compiler::module* module = nullptr;
        auto success = time_task(
            session_task_category_t::compiler,
            source_file->path().filename().string(),
            [&](session_task_t* module_task) {
                syntax::ast_node_t* module_node = nullptr;

                auto parse_success = time_task(
                    session_task_category_t::parser,
                    "source -> ast",
                    [&](session_task_t* parse_task) {
                        module_node = parse(source_file);
                        return module_node != nullptr;
                    });
                if (!parse_success)
                    return false;

                return time_task(
                    session_task_category_t::evaluator,
                    "ast -> code dom",
                    [&](session_task_t* compile_task) {
                        module = dynamic_cast<compiler::module*>(_ast_evaluator->evaluate(module_node));
                        if (module != nullptr) {
                            module->source_file(source_file);
                            auto current_module = _scope_manager->current_module();
                            if (current_module == nullptr) {
                                module->is_root(true);
                                _program->module(module);
                                module->parent_element(_program);
                            } else {
                                module->parent_element(current_module);
                            }
                            if (!_ast_evaluator->compile_module(module_node, module))
                                return false;
                        }
                        return true;
                    });
            });

        if (!success)
            return nullptr;

        _modules.insert(std::make_pair(path_string, module));

        return module;
    }

    syntax::ast_node_t* session::parse(common::source_file* source_file) {
        if (source_file->empty()) {
            if (!source_file->load(_result))
                return nullptr;
        }

        _ast_builder->reset();
        syntax::parser* alpha_parser = nullptr;
        const auto& source_path = source_file->path().string();
        auto it = _parsers.find(source_path);
        if (it == std::end(_parsers)) {
            auto result = _parsers.insert(std::make_pair(
                source_path,
                syntax::parser(source_file, *_ast_builder)));
            alpha_parser = &result.first->second;
        } else {
            alpha_parser = &it->second;
        }

        auto module_node = alpha_parser->parse(_result);
        if (module_node != nullptr && !_result.is_failed()) {
            if (_options.output_ast_graphs) {
                boost::filesystem::path ast_file_path(source_file->path().parent_path());
                auto filename = source_file->path()
                    .filename()
                    .replace_extension({})
                    .string();
                filename += "-ast";
                ast_file_path.append(filename);
                ast_file_path.replace_extension(".dot");
                alpha_parser->write_ast_graph(
                    ast_file_path,
                    module_node);
            }
        }

        return module_node;
    }

    syntax::ast_node_t* session::parse(const boost::filesystem::path& path) {
        auto path_string = path.string();
        auto it = _asts.find(path_string);
        if (it != _asts.end())
            return it->second;
        auto root = parse(add_source_file(path));
        if (root != nullptr) {
            _asts.insert(std::make_pair(path_string, root));
            return root;
        }
        return nullptr;
    }

    common::source_file* session::source_file(const boost::filesystem::path& path) {
        auto it = _source_file_paths.find(path.string());
        if (it == _source_file_paths.end())
            return nullptr;
        return it->second;
    }

    common::source_file* session::add_source_file(const boost::filesystem::path& path) {
        auto adjusted_path = path;
        if (adjusted_path.is_relative()) {
            adjusted_path = boost::filesystem::absolute(path);
        }

        auto file = source_file(path);
        if (file != nullptr)
            return file;

        auto id = common::id_pool::instance()->allocate();
        auto it = _source_file_registry.insert(std::make_pair(
            id,
            common::source_file(id, adjusted_path)));

        if (!it.second)
            return nullptr;

        _source_file_paths.insert(std::make_pair(
            adjusted_path.string(),
            &it.first->second));

        return &it.first->second;
    }

}