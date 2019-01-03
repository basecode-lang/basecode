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
#include <debugger/environment.h>
#include "session.h"
#include "elements.h"
#include "code_dom_formatter.h"

namespace basecode::compiler {

    session::session(
            const session_options_t& options,
            const path_list_t& source_files) : _ffi(options.ffi_heap_size),
                                               _terp(&_ffi, options.allocator, options.heap_size, options.stack_size),
                                               _builder(*this),
                                               _assembler(&_terp),
                                               _ast_evaluator(*this),
                                               _options(options),
                                               _scope_manager(*this) {
        for (const auto& path : source_files) {
            if (path.is_relative()) {
                add_source_file(boost::filesystem::absolute(path));
            } else {
                add_source_file(path);
            }
        }
    }

    session::~session() {
    }

    bool session::run() {
        return _terp.run(_result);
    }

    void session::error(
            compiler::module* module,
            const std::string& code,
            const std::string& message,
            const common::source_location& location) {
        auto file = module->source_file();
        if (file != nullptr)
            file->error(_result, code, message, location);
        else
            _result.error(code, message, location);
    }

    bool session::variable(
            compiler::element* element,
            variable_handle_t& handle,
            bool activate) {
        compiler::element* target_element = element;
        auto id = target_element->id();

        switch (element->element_type()) {
            case element_type_t::binary_operator: {
                auto child_bin_op = dynamic_cast<compiler::binary_operator*>(element);
                if (child_bin_op->operator_type() != operator_type_t::member_access)
                    break;

                std::vector<variable_handle_t> vars {};
                std::stack<binary_operator*> member_accesses {};

                auto current = element;
                while (current->element_type() == element_type_t::binary_operator) {
                    auto bin_op = dynamic_cast<compiler::binary_operator*>(current);
                    if (bin_op->operator_type() == operator_type_t::member_access) {
                        member_accesses.push(bin_op);
                    }
                    current = bin_op->lhs();
                }

                auto bin_op = member_accesses.top();
                auto lhs_read = should_read_variable(bin_op->lhs());
                vars.push_back({});
                if (variable(bin_op->lhs(), vars.back(), lhs_read)) {
                    if (lhs_read) {
                        auto& var = vars.back();
                        var->read();
                    }
                }

                while (!member_accesses.empty()) {
                    bin_op = member_accesses.top();
                    member_accesses.pop();

                    auto& previous_var = vars.back();
                    vars.push_back({});

                    auto rhs_read = should_read_variable(bin_op->rhs());
                    previous_var->field(bin_op->rhs(), vars.back(), rhs_read);
                    if (rhs_read) {
                        auto& var = vars.back();
                        if (!var.is_valid())
                            return false;
                        var->read();
                    }
                }

                vars.back().skip_deactivate();
                handle.set(vars.back().get(), activate);
                return true;
            }
            case element_type_t::identifier_reference: {
                // N.B. need to leave this case alone because we need the id from
                //      the reference; not the identifier itself
                auto ref = dynamic_cast<compiler::identifier_reference*>(element);
                target_element = ref->identifier();
                id = ref->id();
                break;
            }
            default: {
                break;
            }
        }

        auto it = _variables.find(id);
        if (it == _variables.end()) {
            compiler::variable var(*this, target_element);
            if (!var.initialize())
                return false;

            auto new_it = _variables.insert(std::make_pair(id, var));
            handle.set(&new_it.first->second, activate);
        } else {
            handle.set(&it->second, activate);
        }

        return true;
    }

    vm::ffi& session::ffi() {
        return _ffi;
    }

    bool session::compile() {
        auto& top_level_stack = _scope_manager.top_level_stack();
        auto& listing = _assembler.listing();
        const auto& first = (*_source_files.begin()).second;

        time_task(
            "assembler: preparation",
            [&]() {
                auto listing_name = first
                    .path()
                    .filename()
                    .replace_extension(".basm");
                listing.add_source_file(listing_name.string());
                listing.select_source_file(listing_name.string());
                return true;
            });

        time_task(
            "compiler: preparation",
            [&]() {
                _program.block(_scope_manager.push_new_block());
                _program.block()->parent_element(&_program);
                top_level_stack.push(_program.block());
                return true;
            });
        defer(top_level_stack.pop());

        time_task(
            "compiler: core types",
            [&]() {
                initialize_core_types();
                return true;
            });

        time_task(
            "compiler: built-in procedures",
            [&]() {
                initialize_built_in_procedures();
                return true;
            });

        auto success = time_task(
            "compiler: generate model",
            [&]() {
                for (auto source_file : source_files()) {
                    auto module = compile_module(source_file);
                    if (module == nullptr)
                        return false;
                }
                return true;
            });
        if (!success)
            return false;

        success = time_task(
            "compiler: resolve unknown types (phase 1)",
            [&]() { return resolve_unknown_types(false); });
        if (!success)
            return false;

        success = time_task(
            "compiler: resolve unknown identifiers",
            [&]() {
                return resolve_unknown_identifiers();
            });
        if (!success)
            return false;

        success = time_task(
            "compiler: resolve unknown types (phase 2)",
            [&]() { return resolve_unknown_types(false); });
        if (!success)
            return false;

        success = time_task(
            "compiler: constant expression folding",
            [&]() { return fold_constant_expressions(); });
        if (!success)
            return false;

        success = time_task(
            "compiler: type check",
            [&]() { return type_check(); });
        if (!success)
            return false;

        if (!_result.is_failed()) {
            time_task(
                "compiler: generate byte-code",
                [&]() {
                    emit_context_t context {};
                    emit_result_t result(_assembler);
                    _program.emit(*this, context, result);
                   return true;
                });

            success = time_task(
                "assembler: encode byte-code",
                [&]() {
                    _assembler.apply_addresses(_result);
                    _assembler.resolve_labels(_result);
                    return _assembler.assemble(_result);
                });

            if (_options.verbose) {
                time_task(
                    "assembler: listing file",
                    [&]() {
                        disassemble(stdout);
                        fmt::print("\n");
                        return true;
                    });
            }

            if (success) {
                success = time_task(
                    "compiler: execute directives",
                    [&]() { return execute_directives(); });
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
                                "compiler: execute byte-code",
                                [&]() { return run(); });
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
            const std::string& name,
            const session_task_callable_t& callable,
            bool include_in_total) {
        auto insert_at = _tasks.size();

        session_task_t task;
        task.name = name;
        task.include_in_total = include_in_total;

        auto start = std::chrono::high_resolution_clock::now();
        auto success = callable();
        auto end = std::chrono::high_resolution_clock::now();

        task.elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        _tasks.insert(_tasks.begin() + insert_at, task);

        return success;
    }

    vm::terp& session::terp() {
        return _terp;
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
                    "compiler: write code DOM",
                    [&]() {
                        write_code_dom_graph(_options.dom_graph_file);
                        return true;
                    });
            }
        } catch (const fmt::format_error& e) {
            fmt::print("fmt::format_error caught: {}\n", e.what());
        }
    }

    bool session::allocate_reg(
            vm::register_t& reg,
            compiler::element* element) {
        if (!_assembler.allocate_reg(reg)) {
            error(
                element->module(),
                "P052",
                "assembler registers exhausted.",
                element->location());
            return false;
        }
        return true;
    }

    bool session::type_check() {
        bool success;

        success = time_task(
            " - instrinsic callsites",
            [&]() {
                auto intrinsics = _elements.find_by_type<compiler::intrinsic>(element_type_t::intrinsic);
                for (auto intrinsic : intrinsics) {
                    auto args = intrinsic->arguments();
                    prepare_call_site_result_t result {};
                    if (!intrinsic->procedure_type()->prepare_call_site(
                        *this,
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
            },
            false);
        if (!success)
            return false;

        success = time_task(
            " - procedure callsites",
            [&]() {
                auto proc_calls = _elements.find_by_type<compiler::procedure_call>(element_type_t::proc_call);
                for (auto proc_call : proc_calls) {
                    if (!proc_call->resolve_overloads(*this))
                        return false;
                }
                return true;
            },
            false);
        if (!success)
            return false;

        success = time_task(
            "compiler: resolve unknown types (phase 3)",
            [&]() { return resolve_unknown_types(true); });
        if (!success)
            return false;

        auto identifiers = _elements.find_by_type<compiler::identifier>(element_type_t::identifier);
        for (auto var : identifiers) {
            auto init = var->initializer();
            if (init == nullptr)
                continue;

            auto expr = init->expression();
            if (expr != nullptr
            &&  expr->element_type() == element_type_t::uninitialized_literal) {
                continue;
            }

            infer_type_result_t infer_type_result {};
            if (!init->infer_type(*this, infer_type_result)) {
                // XXX: error
                return false;
            }

            if (!var->type_ref()->type()->type_check(infer_type_result.inferred_type)) {
                error(
                    init->module(),
                    "C051",
                    fmt::format(
                        "type mismatch: cannot assign {} to {}.",
                        infer_type_result.type_name(),
                        var->type_ref()->name()),
                    var->location());
            }
        }

        auto binary_ops = _elements.find_by_type<compiler::binary_operator>(element_type_t::binary_operator);
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

            if (!lhs_type_result.inferred_type->type_check(rhs_type_result.inferred_type)) {
                error(
                    binary_op->module(),
                    "C051",
                    fmt::format(
                        "type mismatch: cannot assign {} to {}.",
                        rhs_type_result.type_name(),
                        lhs_type_result.type_name()),
                    binary_op->rhs()->location());
            }
        }

        return !_result.is_failed();
    }

    bool session::initialize() {
        _ffi.initialize(_result);
        _terp.initialize(_result);
        _assembler.initialize(_result);

        _terp.register_trap(trap_putc, [](vm::terp* terp) {
            vm::register_value_alias_t fp_alias {};
            fp_alias.qw = terp->pop();

            vm::register_value_alias_t ch_alias {};
            ch_alias.qw = terp->pop();

            fputc(ch_alias.dw, stdout);
        });

        _terp.register_trap(trap_getc, [](vm::terp* terp) {
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
        return _elements;
    }

    common::result& session::result() {
        return _result;
    }

    bool session::execute_directives() {
        auto directives = _elements.find_by_type<compiler::directive>(element_type_t::directive);
        for (auto directive_element : directives) {
            if (directive_element->is_parent_element(element_type_t::directive))
                continue;

            if (!directive_element->execute(*this)) {
                error(
                    directive_element->module(),
                    "P044",
                    fmt::format("directive failed to execute: {}", directive_element->name()),
                    directive_element->location());
                return false;
            }
        }
        return true;
    }

    vm::assembler& session::assembler() {
        return _assembler;
    }

    element_builder& session::builder() {
        return _builder;
    }

    vm::allocator* session::allocator() {
        return _options.allocator;
    }

    compiler::program& session::program() {
        return _program;
    }

    bool session::emit_interned_strings() {
        return _interned_strings.emit(*this);
    }

    void session::initialize_core_types() {
        auto parent_scope = _scope_manager.current_scope();

        compiler::numeric_type::make_types(*this, parent_scope);
        _scope_manager.add_type_to_scope(_builder.make_module_type(
            parent_scope,
            _builder.make_block(parent_scope)));
        _scope_manager.add_type_to_scope(_builder.make_namespace_type(parent_scope));
        _scope_manager.add_type_to_scope(_builder.make_bool_type(parent_scope));
        _scope_manager.add_type_to_scope(_builder.make_rune_type(parent_scope));
        _scope_manager.add_type_to_scope(_builder.make_tuple_type(
            parent_scope,
            _builder.make_block(parent_scope)));
        _scope_manager.add_type_to_scope(_builder.make_generic_type(
            parent_scope,
            {}));
    }

    bool session::resolve_unknown_types(bool final) {
        auto& identifiers = _scope_manager.identifiers_with_unknown_types();
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

            infer_type_result_t infer_type_result {};

            if (var->is_parent_element(element_type_t::binary_operator)) {
                auto binary_operator = dynamic_cast<compiler::binary_operator*>(var->parent_element());
                switch (binary_operator->operator_type()) {
                    case operator_type_t::assignment: {
                        if (!binary_operator->rhs()->infer_type(*this, infer_type_result))
                            return false;
                        var->type_ref(infer_type_result.reference);
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
                    auto type = _scope_manager.find_type(
                        unknown_type->symbol()->qualified_symbol(),
                        var->parent_scope());
                    if (type != nullptr) {
                        auto type_ref = _builder.make_type_reference(
                            type->parent_scope(),
                            qualified_symbol_t{},
                            type);
                        if (is_pointer) {
                            pointer->base_type_ref(type_ref);
                        } else {
                            var->type_ref(type_ref);
                        }
                        to_remove.insert(unknown_type->id());
                    }
                } else {
                    if (!expr->infer_type(*this, infer_type_result))
                        return false;
                    if (infer_type_result.reference != nullptr)
                        var->type_ref(infer_type_result.reference);
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
            _elements.remove(id);

        return remaining == 0;
    }

    ast_evaluator& session::evaluator() {
        return _ast_evaluator;
    }

    void session::disassemble(FILE* file) {
        _assembler.disassemble();
        if (file != nullptr) {
            fmt::print(file, "\n");
            _assembler.listing().write(file);
        }
    }

    bool session::fold_constant_expressions() {
        return fold_elements_of_type(element_type_t::intrinsic)
            && fold_elements_of_type(element_type_t::identifier_reference)
            && fold_elements_of_type(element_type_t::unary_operator)
            && fold_elements_of_type(element_type_t::binary_operator)
            && fold_elements_of_type(element_type_t::label_reference);
    }

    bool session::resolve_unknown_identifiers() {
        auto& unresolved = _scope_manager.unresolved_identifier_references();
        auto it = unresolved.begin();
        while (it != unresolved.end()) {
            auto unresolved_reference = *it;
            if (unresolved_reference->resolved()) {
                it = unresolved.erase(it);
                continue;
            }

            compiler::block* type_scope = unresolved_reference->parent_scope();
            if (unresolved_reference->is_parent_element(element_type_t::binary_operator)) {
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
                    if (!type_result.inferred_type->is_composite_type()) {
                        error(
                            unresolved_reference->module(),
                            "X000",
                            "member access requires lhs composite type.",
                            unresolved_reference->symbol().location);
                        return false;
                    }
                    auto composite_type = dynamic_cast<compiler::composite_type*>(type_result.inferred_type);
                    type_scope = composite_type->scope();
                }
            }

            auto vars = _scope_manager.find_identifier(
                unresolved_reference->symbol(),
                type_scope);
            compiler::identifier* identifier = vars.empty() ? nullptr : vars.front();
            if (identifier == nullptr
            &&  unresolved_reference->symbol().is_qualified()) {
                vars = _scope_manager.find_identifier(
                    unresolved_reference->symbol(),
                    type_scope);
                identifier = vars.empty() ? nullptr : vars.front();
            }

            if (identifier == nullptr) {
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

            unresolved_reference->identifier(identifier);

            it = unresolved.erase(it);
        }

        return unresolved.empty();
    }

    syntax::ast_builder& session::ast_builder() {
        return _ast_builder;
    }

    const element_map& session::elements() const {
        return _elements;
    }

    void session::initialize_built_in_procedures() {
    }

    common::source_file* session::pop_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        auto source_file = _source_file_stack.top();
        _source_file_stack.pop();
        return source_file;
    }

    bool session::fold_elements_of_type(element_type_t type) {
        std::vector<common::id_t> to_remove {};

        auto elements = _elements.find_by_type<compiler::element>(type);
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
                        fold_result.element->attributes().add(_builder.make_attribute(
                            _scope_manager.current_scope(),
                            "intrinsic_substitution",
                            _builder.make_string(
                                _scope_manager.current_scope(),
                                intrinsic->name())));
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
            _elements.remove(id);

        return true;
    }

    compiler::scope_manager& session::scope_manager() {
        return _scope_manager;
    }

    const session_options_t& session::options() const {
        return _options;
    }

    const compiler::program& session::program() const {
        return _program;
    }

    const session_task_list_t& session::tasks() const {
        return _tasks;
    }

    common::source_file* session::current_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        return _source_file_stack.top();
    }

    bool session::allocate_address_register(common::id_t id) {
        auto it = _address_registers.find(id);
        if (it != _address_registers.end())
            return true;

        vm::register_t reg {};
        reg.size = vm::op_sizes::qword;
        reg.type = vm::register_type_t::integer;

        if (!_assembler.allocate_reg(reg)) {
            // XXX: error
            return false;
        }

        _address_registers.insert(std::make_pair(id, reg));

        return true;
    }

    std::vector<common::source_file*> session::source_files() {
        std::vector<common::source_file*> list {};
        for (auto& it : _source_files) {
            list.push_back(&it.second);
        }
        return list;
    }

    const compiler::scope_manager& session::scope_manager() const {
        return _scope_manager;
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

    vm::label_ref_t* session::type_info_label(compiler::type* type) {
        auto it = _type_info_labels.find(type->id());
        if (it == _type_info_labels.end())
            return nullptr;
        return it->second;
    }

    void session::push_source_file(common::source_file* source_file) {
        _source_file_stack.push(source_file);
    }

    const address_register_map_t& session::address_registers() const {
        return _address_registers;
    }

    common::id_t session::intern_string(compiler::string_literal* literal) {
        return _interned_strings.intern(literal);
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

    vm::register_t* session::get_address_register(common::id_t id) {
        auto it = _address_registers.find(id);
        if (it != _address_registers.end())
            return &it->second;

        return nullptr;
    }

    compiler::module* session::compile_module(common::source_file* source_file) {
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
            fmt::format(" - parse: {}", source_file->path().filename().string()),
            [&]() {
                auto module_node = parse(source_file);
                if (module_node != nullptr) {
                    return time_task(
                        " - compile",
                        [&]() {
                            module = dynamic_cast<compiler::module*>(_ast_evaluator.evaluate(module_node));
                            if (module != nullptr) {
                                module->source_file(source_file);
                                auto current_module = _scope_manager.current_module();
                                if (current_module == nullptr)
                                    module->parent_element(&_program);
                                else
                                    module->parent_element(current_module);
                                module->is_root(is_root);
                                if (is_root)
                                    _program.module(module);
                                if (!_ast_evaluator.compile_module(module_node, module))
                                    return false;
                            }
                            return true;
                        },
                        false);
                }
                return false;
            },
            false);
        if (!success)
            return nullptr;

        return module;
    }

    void session::type_info_label(compiler::type* type, vm::label_ref_t* label) {
        if (_type_info_labels.count(type->id()) > 0)
            return;
        _type_info_labels.insert(std::make_pair(type->id(), label));
    }

    syntax::ast_node_t* session::parse(common::source_file* source_file) {
        if (source_file->empty()) {
            if (!source_file->load(_result))
                return nullptr;
        }

        _ast_builder.reset();
        syntax::parser alpha_parser(source_file, _ast_builder);

        auto module_node = alpha_parser.parse(_result);
        if (module_node != nullptr && !_result.is_failed()) {
            if (_options.output_ast_graphs) {
                boost::filesystem::path ast_file_path(source_file->path().parent_path());
                auto filename = source_file->path()
                    .filename()
                    .replace_extension("")
                    .string();
                filename += "-ast";
                ast_file_path.append(filename);
                ast_file_path.replace_extension(".dot");
                alpha_parser.write_ast_graph(
                    ast_file_path,
                    module_node);
            }
        }

        return module_node;
    }

    syntax::ast_node_t* session::parse(const boost::filesystem::path& path) {
        auto source_file = find_source_file(path);
        if (source_file == nullptr) {
            source_file = add_source_file(path);
        }
        return parse(source_file);
    }

    std::string session::intern_data_label(compiler::string_literal* literal) const {
        return _interned_strings.data_label(literal);
    }

    common::source_file* session::add_source_file(const boost::filesystem::path& path) {
        auto it = _source_files.insert(std::make_pair(
            path.string(),
            common::source_file(path)));
        if (!it.second)
            return nullptr;
        return &it.first->second;
    }

    common::source_file* session::find_source_file(const boost::filesystem::path& path) {
        auto it = _source_files.find(path.string());
        if (it == _source_files.end())
            return nullptr;
        return &it->second;
    }

};