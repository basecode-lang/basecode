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

#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "block.h"
#include "label.h"
#include "raw_block.h"
#include "assembly_directive.h"

namespace basecode::compiler {

    assembly_directive::assembly_directive(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::element* expression) : directive(module, parent_scope, "assembly"),
                                             _expression(expression) {
    }

    bool assembly_directive::on_emit(
            compiler::session& session,
            compiler::emit_context_t& context,
            compiler::emit_result_t& result) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto raw_block = dynamic_cast<compiler::raw_block*>(_expression);

        common::source_file source_file;
        if (!source_file.load(session.result(), raw_block->value() + "\n"))
            return false;

        vm::assemble_from_source_result_t assemble_result {};
        auto success = assembler.assemble_from_source(
            session.result(),
            source_file,
            [&](vm::assembly_symbol_type_t type,
                    const std::string& symbol,
                    vm::assembly_symbol_result_t& result) {
                switch (type) {
                    case vm::assembly_symbol_type_t::local: {
                        auto entry = _expression->parent_scope()->find_active_frame_entry(symbol);
                        if (entry != nullptr) {
                            vm::compiler_local_data_t data {};
                            const auto& offsets = entry->owning_frame()->offsets();
                            switch (entry->type()) {
                                case stack_frame_entry_type_t::local: {
                                    data.offset = -offsets.locals + entry->offset();
                                    break;
                                }
                                case stack_frame_entry_type_t::parameter: {
                                    data.offset = offsets.parameters + entry->offset();
                                    break;
                                }
                                case stack_frame_entry_type_t::return_slot: {
                                    data.offset = offsets.return_slot + entry->offset();
                                    break;
                                }
                            }
                            data.reg = vm::registers_t::fp;
                            result.data(data);
                            return true;
                        }
                        break;
                    }
                    case vm::assembly_symbol_type_t::label: {
                        auto labels = session
                            .elements()
                            .find_by_type<compiler::label>(element_type_t::label);
                        for (auto label : labels) {
                            if (label->name() == symbol) {
                                vm::compiler_label_data_t data {};
                                data.label = label->label_name();
                                result.data(data);
                                return true;
                            }
                        }

                        break;
                    }
                    case vm::assembly_symbol_type_t::module: {
                        auto var = session.scope_manager().find_identifier(
                            qualified_symbol_t(symbol),
                            _expression->parent_scope());
                        if (var != nullptr) {
                            vm::compiler_label_data_t data {};
                            data.label = var->label_name();
                            result.data(data);
                            return true;
                        }
                        break;
                    }
                    case vm::assembly_symbol_type_t::assembler: {
                        break;
                    }
                }
                return false;
            },
            assemble_result);
        if (success) {
            for (const auto& entry : assemble_result.block->entries()) {
                block->comment(
                    "directive: assembly",
                    vm::comment_location_t::after_instruction);
                block->add_entry(entry);
            }
        }

        return true;
    }

    void assembly_directive::on_owned_elements(element_list_t& list) {
        list.emplace_back(_expression);
    }

    bool assembly_directive::on_evaluate(compiler::session& session) {
        auto is_valid = _expression != nullptr
            && _expression->element_type() == element_type_t::raw_block;
        if (!is_valid) {
            session.error(
                module(),
                "P004",
                "#assembly expects a valid raw block expression.",
                location());
            return false;
        }

        return true;
    }

};