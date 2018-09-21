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
#include "session.h"
#include "elements/type.h"
#include "elements/block.h"
#include "elements/module.h"
#include "elements/program.h"
#include "elements/any_type.h"
#include "elements/intrinsic.h"
#include "elements/directive.h"
#include "elements/bool_type.h"
#include "elements/type_info.h"
#include "code_dom_formatter.h"
#include "elements/identifier.h"
#include "elements/tuple_type.h"
#include "elements/string_type.h"
#include "elements/initializer.h"
#include "elements/module_type.h"
#include "elements/numeric_type.h"
#include "elements/unknown_type.h"
#include "elements/argument_list.h"
#include "elements/float_literal.h"
#include "elements/unary_operator.h"
#include "elements/symbol_element.h"
#include "elements/string_literal.h"
#include "elements/procedure_type.h"
#include "elements/namespace_type.h"
#include "elements/type_reference.h"
#include "elements/binary_operator.h"
#include "elements/integer_literal.h"
#include "elements/identifier_reference.h"

namespace basecode::compiler {

    bool variable_register_t::matches(vm::register_t* other_reg) {
        if (other_reg == nullptr)
            return true;
        return (*other_reg).number == reg.number;
    }

    bool variable_register_t::reserve(compiler::session& session) {
        allocated = session.assembler().allocate_reg(reg);
        return allocated;
    }

    void variable_register_t::release(compiler::session& session) {
        if (!allocated)
            return;
        session.assembler().free_reg(reg);
        allocated = false;
    }

    ///////////////////////////////////////////////////////////////////////////

    bool variable_t::init(compiler::session& session) {
        if (!live)
            return false;

        if (address_loaded)
            return true;

        if (usage == identifier_usage_t::heap) {
            if (!address_reg.reserve(session))
                return false;

            auto& assembler = session.assembler();
            auto block = assembler.current_block();

            block->comment(
                fmt::format(
                    "identifier '{}' address (global)",
                    name),
                4);

            auto label_ref = assembler.make_label_ref(name);
            if (address_offset != 0) {
                block->move_label_to_reg_with_offset(
                    address_reg.reg,
                    label_ref,
                    address_offset);
            } else {
                block->move_label_to_reg(address_reg.reg, label_ref);
            }
        }

        value_reg.reg.type = vm::register_type_t::integer;
        if (type != nullptr) {
            if (type->access_model() == type_access_model_t::value) {
                value_reg.reg.size = vm::op_size_for_byte_size(type->size_in_bytes());
                if (type->number_class() == type_number_class_t::floating_point) {
                    value_reg.reg.type = vm::register_type_t::floating_point;
                }
            } else {
                value_reg.reg.size = vm::op_sizes::qword;
            }
        }

        address_loaded = true;

        return true;
    }

    bool variable_t::read(compiler::session& session) {
        if (!live)
            return false;

        if (!init(session))
            return false;

        std::string type_name = "global";
        if (requires_read) {
            if (!value_reg.reserve(session))
                return false;

            auto& assembler = session.assembler();
            auto block = assembler.current_block();

            block->comment(
                fmt::format(
                    "load identifier '{}' value ({})",
                    name,
                    type_name),
                4);

            if (value_reg.reg.size != vm::op_sizes::qword)
                block->clr(vm::op_sizes::qword, value_reg.reg);

            if (usage == identifier_usage_t::stack) {
                type_name = stack_frame_entry_type_name(frame_entry->type);
                block->load_to_reg(
                    value_reg.reg,
                    vm::register_t::fp(),
                    frame_entry->offset);
            } else {
                block->load_to_reg(value_reg.reg, address_reg.reg);
            }

            requires_read = false;
        }

        return true;
    }

    bool variable_t::write(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto target_reg = assembler.current_target_register();
        if (target_reg == nullptr)
            return false;

        block->store_from_reg(
            address_reg.reg,
            *target_reg,
            frame_entry != nullptr ? frame_entry->offset : 0);

        written = true;
        requires_read = true;

        return true;
    }

    void variable_t::make_live(compiler::session& session) {
        if (live)
            return;
        live = true;
        address_loaded = false;
        requires_read = address_offset == 0;
    }

    void variable_t::make_dormant(compiler::session& session) {
        if (!live)
            return;
        live = false;
        requires_read = false;
        address_loaded = false;
        value_reg.release(session);
        address_reg.release(session);
    }

    ///////////////////////////////////////////////////////////////////////////

    session::session(
            const session_options_t& options,
            const path_list_t& source_files) : _terp(options.heap_size, options.stack_size),
                                               _builder(*this),
                                               _assembler(&_terp),
                                               _ast_evaluator(*this),
                                               _options(options),
                                               _stack_frame(nullptr),
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
            const std::string& code,
            const std::string& message,
            const common::source_location& location) {
        auto source_file = current_source_file();
        if (source_file == nullptr)
            return;
        source_file->error(_result, code, message, location);
    }

    void session::error(
            compiler::element* element,
            const std::string& code,
            const std::string& message,
            const common::source_location& location) {
        element->module()
            ->source_file()
            ->error(_result, code, message, location);
    }

    bool session::compile() {
        auto& top_level_stack = _scope_manager.top_level_stack();

        auto& listing = _assembler.listing();
        listing.add_source_file("top_level.basm");
        listing.select_source_file("top_level.basm");

        _program.block(_scope_manager.push_new_block());
        _program.block()->parent_element(&_program);

        top_level_stack.push(_program.block());

        initialize_core_types();
        initialize_built_in_procedures();

        for (auto source_file : source_files()) {
            auto module = compile_module(source_file);
            if (module == nullptr)
                return false;
        }

        auto directives = _elements.find_by_type(element_type_t::directive);
        for (auto directive : directives) {
            auto directive_element = dynamic_cast<compiler::directive*>(directive);
            if (!directive_element->execute(*this)) {
                error(
                    directive_element,
                    "P044",
                    fmt::format("directive failed to execute: {}", directive_element->name()),
                    directive->location());
                return false;
            }
        }

        if (!resolve_unknown_identifiers())
            return false;

        if (!resolve_unknown_types())
            return false;

        if (!type_check())
            return false;

        if (!fold_constant_intrinsics())
            return false;

        if (!_result.is_failed()) {
            _program.emit(*this);

            _assembler.apply_addresses(_result);
            _assembler.resolve_labels(_result);

            if (_assembler.assemble(_result)) {
                run();
            }
        }

        top_level_stack.pop();

        return !_result.is_failed();
    }

    vm::terp& session::terp() {
        return _terp;
    }

    void session::raise_phase(
            session_compile_phase_t phase,
            const boost::filesystem::path& source_file) {
        if (_options.compile_callback == nullptr)
            return;
        _options.compile_callback(phase, source_file);
    }

    void session::finalize() {
        if (_options.verbose) {
            try {
                disassemble(stdout);
                if (!_options.dom_graph_file.empty())
                    write_code_dom_graph(_options.dom_graph_file);
            } catch (const fmt::format_error& e) {
                fmt::print("fmt::format_error caught: {}\n", e.what());
            }
        }
    }

    void session::free_variable(
            compiler::session& session,
            const std::string& name) {
        auto var = variable(name);
        if (var != nullptr) {
            var->make_dormant(session);
            _variables.erase(name);
        }
    }

    bool session::type_check() {
        auto identifiers = _elements.find_by_type(element_type_t::identifier);
        for (auto identifier : identifiers) {
            auto var = dynamic_cast<compiler::identifier*>(identifier);
            auto init = var->initializer();
            if (init == nullptr)
                continue;
            infer_type_result_t infer_type_result {};
            if (!init->infer_type(*this, infer_type_result)) {
                // XXX: error
                return false;
            }
            if (!var->type_ref()->type()->type_check(infer_type_result.inferred_type)) {
                error(
                    init,
                    "C051",
                    fmt::format(
                        "type mismatch: cannot assign {} to {}.",
                        infer_type_result.type_name(),
                        var->type_ref()->name()),
                    var->location());
            }
        }

        auto binary_ops = _elements.find_by_type(element_type_t::binary_operator);
        for (auto op : binary_ops) {
            auto binary_op = dynamic_cast<compiler::binary_operator*>(op);
            if (binary_op->operator_type() != operator_type_t::assignment)
                continue;

            // XXX: revisit this for destructuring/multiple assignment
            auto var = dynamic_cast<compiler::identifier*>(binary_op->lhs());
            infer_type_result_t infer_type_result {};
            if (!binary_op->rhs()->infer_type(*this, infer_type_result)) {
                error(
                    binary_op->rhs(),
                    "P052",
                    "unable to infer type.",
                    binary_op->rhs()->location());
                return false;
            }
            if (!var->type_ref()->type()->type_check(infer_type_result.inferred_type)) {
                error(
                    binary_op,
                    "C051",
                    fmt::format(
                        "type mismatch: cannot assign {} to {}.",
                        infer_type_result.type_name(),
                        var->type_ref()->name()),
                    binary_op->rhs()->location());
            }
        }

        return !_result.is_failed();
    }

    bool session::initialize() {
        _terp.initialize(_result);
        _assembler.initialize(_result);
        return !_result.is_failed();
    }

    element_map& session::elements() {
        return _elements;
    }

    common::result& session::result() {
        return _result;
    }

    vm::assembler& session::assembler() {
        return _assembler;
    }

    element_builder& session::builder() {
        return _builder;
    }

    compiler::program& session::program() {
        return _program;
    }

    void session::initialize_core_types() {
        auto parent_scope = _scope_manager.current_scope();

        compiler::numeric_type::make_types(*this, parent_scope);
        _scope_manager.add_type_to_scope(_builder.make_module_type(
            parent_scope,
            _builder.make_block(parent_scope, element_type_t::block)));
        _scope_manager.add_type_to_scope(_builder.make_namespace_type(parent_scope));
        _scope_manager.add_type_to_scope(_builder.make_bool_type(parent_scope));
        _scope_manager.add_type_to_scope(_builder.make_string_type(
            parent_scope,
            _builder.make_block(parent_scope, element_type_t::block)));

        _scope_manager.add_type_to_scope(_builder.make_type_info_type(
            parent_scope,
            _builder.make_block(parent_scope, element_type_t::block)));
        _scope_manager.add_type_to_scope(_builder.make_tuple_type(
            parent_scope,
            _builder.make_block(parent_scope, element_type_t::block)));
        _scope_manager.add_type_to_scope(_builder.make_any_type(
            parent_scope,
            _builder.make_block(parent_scope, element_type_t::block)));
    }

    bool session::resolve_unknown_types() {
        auto& identifiers = _scope_manager.identifiers_with_unknown_types();

        auto it = identifiers.begin();
        while (it != identifiers.end()) {
            auto var = *it;

            if (var->type_ref() != nullptr
            &&  var->type_ref()->type()->element_type() != element_type_t::unknown_type) {
                it = identifiers.erase(it);
                continue;
            }

            infer_type_result_t infer_type_result {};

            if (var->is_parent_element(element_type_t::binary_operator)) {
                auto binary_operator = dynamic_cast<compiler::binary_operator*>(var->parent_element());
                if (binary_operator->operator_type() == operator_type_t::assignment) {
                    if (!binary_operator->rhs()->infer_type(*this, infer_type_result))
                        return false;
                    var->type_ref(infer_type_result.reference);
                }
            } else {
                if (var->initializer() == nullptr) {
                    auto unknown_type = dynamic_cast<compiler::unknown_type*>(var->type_ref()->type());

                    type_find_result_t find_result {};
                    find_result.type_name = unknown_type->symbol()->qualified_symbol();
                    find_result.is_array = unknown_type->is_array();
                    find_result.is_pointer = unknown_type->is_pointer();
                    find_result.array_subscripts = unknown_type->subscripts();

                    find_result.type = _builder.make_complete_type(
                        find_result,
                        var->parent_scope());
                    if (find_result.type != nullptr) {
                        var->type_ref(find_result.make_type_reference(
                            _builder,
                            find_result.type->parent_scope()));
                        _elements.remove(unknown_type->id());
                    }
                } else {
                    if (!var->initializer()->expression()->infer_type(*this, infer_type_result))
                        return false;
                    var->type_ref(infer_type_result.reference);
                }
            }

            if (var->type_ref()->type()->element_type() != element_type_t::unknown_type) {
                var->inferred_type(true);
                it = identifiers.erase(it);
            } else {
                ++it;
                error(
                    var,
                    "P004",
                    fmt::format(
                        "unable to resolve type for identifier: {}",
                        var->symbol()->name()),
                    var->symbol()->location());
            }
        }

        return identifiers.empty();
    }

    ast_evaluator& session::evaluator() {
        return _ast_evaluator;
    }

    variable_t* session::allocate_variable(
            const std::string& name,
            compiler::type* type,
            identifier_usage_t usage,
            vm::stack_frame_entry_t* frame_entry) {
        auto var = variable(name);
        if (var != nullptr)
            return var;

        auto it = _variables.insert(std::make_pair(
            name,
            variable_t {
                .name = name,
                .written = false,
                .usage = usage,
                .requires_read = false,
                .address_loaded = false,
                .type = type,
                .frame_entry = frame_entry,
            }));
        return it.second ? &it.first->second : nullptr;
    }

    void session::disassemble(FILE* file) {
        _assembler.disassemble();
        if (file != nullptr) {
            fmt::print(file, "\n");
            _assembler.listing().write(file);
        }
    }

    bool session::fold_constant_intrinsics() {
        auto intrinsics = _elements.find_by_type(element_type_t::intrinsic);
        for (auto e : intrinsics) {
            if (!e->is_constant())
                continue;

            fold_result_t fold_result {};
            if (!e->fold(*this, fold_result))
                return false;

            if (fold_result.element != nullptr) {
                auto intrinsic = dynamic_cast<compiler::intrinsic*>(e);
                auto parent = intrinsic->parent_element();
                if (parent != nullptr) {
                    fold_result.element->attributes().add(_builder.make_attribute(
                        _scope_manager.current_scope(),
                        "intrinsic_substitution",
                        _builder.make_string(
                            _scope_manager.current_scope(),
                            intrinsic->name())));
                    switch (parent->element_type()) {
                        case element_type_t::initializer: {
                            auto initializer = dynamic_cast<compiler::initializer*>(parent);
                            initializer->expression(fold_result.element);
                            break;
                        }
                        case element_type_t::argument_list: {
                            auto arg_list = dynamic_cast<compiler::argument_list*>(parent);
                            auto index = arg_list->find_index(intrinsic->id());
                            if (index == -1) {
                                return false;
                            }
                            arg_list->replace(static_cast<size_t>(index), fold_result.element);
                            break;
                        }
                        case element_type_t::unary_operator: {
                            auto unary_op = dynamic_cast<compiler::unary_operator*>(parent);
                            unary_op->rhs(fold_result.element);
                            break;
                        }
                        case element_type_t::binary_operator: {
                            auto binary_op = dynamic_cast<compiler::binary_operator*>(parent);
                            if (binary_op->lhs() == intrinsic) {
                                binary_op->lhs(fold_result.element);
                            } else if (binary_op->rhs() == intrinsic) {
                                binary_op->rhs(fold_result.element);
                            } else {
                                // XXX: error
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    fold_result.element->parent_element(parent);
                    _elements.remove(intrinsic->id());
                }
            }
        }
        return true;
    }

    vm::stack_frame_t* session::stack_frame() {
        return &_stack_frame;
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

            auto identifier = _scope_manager.find_identifier(
                unresolved_reference->symbol(),
                unresolved_reference->parent_scope());
            if (identifier == nullptr) {
                ++it;
                error(
                    unresolved_reference,
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

    const element_map& session::elements() const {
        return _elements;
    }

    void session::initialize_built_in_procedures() {
//        auto parent_scope = _scope_manager.current_scope();
    }

    common::source_file* session::pop_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        auto source_file = _source_file_stack.top();
        _source_file_stack.pop();
        return source_file;
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

    common::source_file* session::current_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        return _source_file_stack.top();
    }

    variable_t* session::variable(const std::string& name) {
        const auto it = _variables.find(name);
        if (it == _variables.end())
            return nullptr;
        return &it->second;
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

    vm::label_ref_t* session::type_info_label(compiler::type* type) {
        auto it = _type_info_labels.find(type->id());
        if (it == _type_info_labels.end())
            return nullptr;
        return it->second;
    }

    void session::push_source_file(common::source_file* source_file) {
        _source_file_stack.push(source_file);
    }

    variable_t* session::variable_for_element(compiler::element* element) {
        if (element == nullptr)
            return nullptr;
        switch (element->element_type()) {
            case element_type_t::identifier: {
                auto identifier = dynamic_cast<compiler::identifier*>(element);
                return variable(identifier->symbol()->name());
            }
            case element_type_t::string_literal: {
                auto string_literal = dynamic_cast<compiler::string_literal*>(element);
                return variable(string_literal->label_name());
            }
            case element_type_t::identifier_reference: {
                auto identifier = dynamic_cast<compiler::identifier_reference*>(element)->identifier();
                return variable(identifier->symbol()->name());
            }
            default:
                return nullptr;
        }
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
        auto is_root = current_source_file() == nullptr;

        push_source_file(source_file);
        defer({
            pop_source_file();
        });

        compiler::module* module = (compiler::module*)nullptr;
        auto module_node = parse(source_file);
        if (module_node != nullptr) {
            module = dynamic_cast<compiler::module*>(_ast_evaluator.evaluate(module_node.get()));
            if (module != nullptr) {
                module->parent_element(&_program);
                module->is_root(is_root);
            }
        }

        return module;
    }

    void session::type_info_label(compiler::type* type, vm::label_ref_t* label) {
        if (_type_info_labels.count(type->id()) > 0)
            return;
        _type_info_labels.insert(std::make_pair(type->id(), label));
    }

    syntax::ast_node_shared_ptr session::parse(common::source_file* source_file) {
        raise_phase(session_compile_phase_t::start, source_file->path());
        defer({
            if (_result.is_failed()) {
              raise_phase(session_compile_phase_t::failed, source_file->path());
            } else {
              raise_phase(session_compile_phase_t::success, source_file->path());
            }
        });

        if (source_file->empty()) {
            if (!source_file->load(_result))
                return nullptr;
        }

        syntax::parser alpha_parser(source_file);
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

    syntax::ast_node_shared_ptr session::parse(const boost::filesystem::path& path) {
        auto source_file = find_source_file(path);
        if (source_file == nullptr) {
            source_file = add_source_file(path);
        }
        return parse(source_file);
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