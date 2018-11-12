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
#include "elements/rune_type.h"
#include "elements/bool_type.h"
#include "elements/type_info.h"
#include "code_dom_formatter.h"
#include "elements/identifier.h"
#include "elements/tuple_type.h"
#include "elements/string_type.h"
#include "elements/initializer.h"
#include "elements/module_type.h"
#include "elements/generic_type.h"
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

    session::session(
            const session_options_t& options,
            const path_list_t& source_files) : _ffi(options.ffi_heap_size),
                                               _terp(&_ffi, options.allocator, options.heap_size, options.stack_size),
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

    bool session::variable(
            compiler::element* element,
            variable_handle_t& handle,
            bool activate) {
        compiler::element* target_element = element;

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
                    if (lhs_read)
                        vars.back()->read();
                }

                while (!member_accesses.empty()) {
                    bin_op = member_accesses.top();
                    member_accesses.pop();

                    auto& previous_var = vars.back();
                    vars.push_back({});

                    auto rhs_read = should_read_variable(bin_op->rhs());
                    previous_var->field(bin_op->rhs(), vars.back(), rhs_read);
                    if (rhs_read)
                        vars.back()->read();
                }

                vars.back().skip_deactivate();
                handle.set(vars.back().get(), activate);
                return true;
            }
            case element_type_t::identifier_reference: {
                auto ref = dynamic_cast<compiler::identifier_reference*>(element);
                target_element = ref->identifier();
                break;
            }
            default: {
                break;
            }
        }

        auto it = _variables.find(target_element->id());
        if (it == _variables.end()) {
            compiler::variable var(*this, target_element);
            if (!var.initialize())
                return false;

            auto new_it = _variables.insert(std::make_pair(
                target_element->id(),
                var));
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
                if (_options.verbose) {
                    disassemble(stdout);
                    fmt::print("\n");
                }

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
                if (!_options.dom_graph_file.empty())
                    write_code_dom_graph(_options.dom_graph_file);
            } catch (const fmt::format_error& e) {
                fmt::print("fmt::format_error caught: {}\n", e.what());
            }
        }
    }


    bool session::allocate_reg(
            vm::register_t& reg,
            compiler::element* element) {
        if (!_assembler.allocate_reg(reg)) {
            error(
                element,
                "P052",
                "assembler registers exhausted.",
                element->location());
            return false;
        }
        return true;
    }

    bool session::type_check() {
        auto identifiers = _elements.find_by_type(element_type_t::identifier);
        for (auto identifier : identifiers) {
            auto var = dynamic_cast<compiler::identifier*>(identifier);
            if (var == nullptr)
                continue;

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

            infer_type_result_t rhs_inferred_type {};
            if (!binary_op->rhs()->infer_type(*this, rhs_inferred_type)) {
                error(
                    binary_op->rhs(),
                    "P052",
                    "unable to infer type.",
                    binary_op->rhs()->location());
                return false;
            }

            switch (binary_op->lhs()->element_type()) {
                case element_type_t::identifier: {
                    auto var = dynamic_cast<compiler::identifier*>(binary_op->lhs());
                    if (!var->type_ref()->type()->type_check(rhs_inferred_type.inferred_type)) {
                        error(
                            binary_op,
                            "C051",
                            fmt::format(
                                "type mismatch: cannot assign {} to {}.",
                                rhs_inferred_type.type_name(),
                                var->type_ref()->name()),
                            binary_op->rhs()->location());
                    }
                    break;
                }
                case element_type_t::unary_operator: {
                    auto lhs_unary_op = dynamic_cast<compiler::binary_operator*>(binary_op->lhs());
                    if (lhs_unary_op->operator_type() != operator_type_t::pointer_dereference)
                        break;

                    // XXX: todo
                    break;
                }
                case element_type_t::binary_operator: {
                    auto lhs_bin_op = dynamic_cast<compiler::binary_operator*>(binary_op->lhs());
                    if (lhs_bin_op->operator_type() != operator_type_t::member_access)
                        break;

                    variable_handle_t field_var {};
                    if (!variable(lhs_bin_op, field_var, false)) {
                        // XXX: error
                        return false;
                    }

                    auto type_result = field_var->type_result();
                    if (!type_result.inferred_type->type_check(rhs_inferred_type.inferred_type)) {
                        error(
                            binary_op,
                            "C051",
                            fmt::format(
                                "type mismatch: cannot assign {} to {}.",
                                rhs_inferred_type.type_name(),
                                type_result.type_name()),
                            binary_op->rhs()->location());
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        return !_result.is_failed();
    }

    bool session::initialize() {
        _ffi.initialize(_result);
        _terp.initialize(_result);
        _assembler.initialize(_result);

        return !_result.is_failed();
    }

    bool session::emit_to_temp(
            compiler::element* element,
            vm::op_sizes reg_size,
            vm::register_type_t reg_type) {
        vm::register_t temp_reg;
        temp_reg.size = reg_size;
        temp_reg.type = reg_type;
        if (!allocate_reg(temp_reg, element))
            return false;

        _assembler.push_target_register(temp_reg);
        element->emit(*this);

        return true;
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
            _builder.make_block(parent_scope, element_type_t::block)));
        _scope_manager.add_type_to_scope(_builder.make_namespace_type(parent_scope));
        _scope_manager.add_type_to_scope(_builder.make_bool_type(parent_scope));
        _scope_manager.add_type_to_scope(_builder.make_rune_type(parent_scope));
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
        _scope_manager.add_type_to_scope(_builder.make_generic_type(
            parent_scope,
            {}));
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

                    // XXX: rework this with new type declaration structure
//                    type_find_result_t find_result {};
//                    find_result.type_name = unknown_type->symbol()->qualified_symbol();
//                    find_result.is_array = unknown_type->is_array();
//                    find_result.is_pointer = unknown_type->is_pointer();
//                    find_result.array_subscripts = unknown_type->subscripts();
//
//                    find_result.type = _builder.make_complete_type(
//                        find_result,
//                        var->parent_scope());
//                    if (find_result.type != nullptr) {
//                        var->type_ref(find_result.make_type_reference(
//                            _builder,
//                            find_result.type->parent_scope()));
//                        _elements.remove(unknown_type->id());
//                    }
                } else {
                    if (!var->initializer()->expression()->infer_type(*this, infer_type_result))
                        return false;
                    var->type_ref(infer_type_result.reference);
                }
            }

            auto type_ref = var->type_ref();
            if (type_ref != nullptr
            &&  type_ref->type()->element_type() != element_type_t::unknown_type) {
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
            auto intrinsic = dynamic_cast<compiler::intrinsic*>(e);
            if (!intrinsic->can_fold())
                continue;

            fold_result_t fold_result {};
            if (!intrinsic->fold(*this, fold_result))
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

    syntax::ast_node_shared_ptr session::parse(const boost::filesystem::path& path) {
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