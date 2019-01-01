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

#include <parser/token.h>
#include <common/bytes.h>
#include <common/string_support.h>
#include <common/source_location.h>
#include "label.h"
#include "assembler.h"
#include "assembly_parser.h"
#include "instruction_block.h"

namespace basecode::vm {

    assembler::assembler(vm::terp* terp) : _terp(terp) {
    }

    assembler::~assembler() {
        for (const auto& it : _labels)
            delete it.second;

        for (const auto& kvp : _block_registry)
            delete kvp.second;
    }

    void assembler::disassemble() {
        for (auto block : _blocks)
            disassemble(block);
    }

    bool assembler::assemble(common::result& r) {
        uint64_t highest_address = 0;

        for (auto block : _blocks) {
            if (!block->should_emit())
                continue;

            for (auto& entry : block->entries()) {
                highest_address = entry.address();

                switch (entry.type()) {
                    case block_entry_type_t::instruction: {
                        auto inst = entry.data<instruction_t>();
                        auto inst_size = inst->encode(r, entry.address());
                        if (inst_size == 0)
                            return false;
                        break;
                    }
                    case block_entry_type_t::data_definition: {
                        auto data_def = entry.data<data_definition_t>();
                        if (data_def->type == data_definition_type_t::initialized) {
                            auto size_in_bytes = op_size_in_bytes(data_def->size);
                            auto offset = 0;
                            for (auto v : data_def->values) {
                                if (v.which() != 0) {
                                    auto label_ref = boost::get<label_ref_t*>(v);
                                    r.error(
                                        "A031",
                                        fmt::format("unexpected label_ref_t*: {}", label_ref->name));
                                    continue;
                                }
                                _terp->write(
                                    data_def->size,
                                    entry.address() + offset,
                                    boost::get<uint64_t>(v));
                                offset += size_in_bytes;
                            }
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }

        highest_address += 8;
        _terp->heap_free_space_begin(common::align(highest_address, 8));

        return !r.is_failed();
    }

    vm::segment* assembler::segment(
            const std::string& name,
            segment_type_t type) {
        _segments.insert(std::make_pair(
            name,
            vm::segment(name, type)));
        return segment(name);
    }

    bool assembler::assemble_from_source(
            common::result& r,
            common::source_file& source_file,
            const assembly_symbol_resolver_t& resolver) {
        vm::assembly_parser parser(this, source_file, resolver);
        return parser.parse(r);
    }

    instruction_block* assembler::pop_block() {
        if (_block_stack.empty())
            return nullptr;
        auto top = _block_stack.top();
        _block_stack.pop();
        return top;
    }

    vm::assembly_listing& assembler::listing() {
        return _listing;
    }

    segment_list_t assembler::segments() const {
        segment_list_t list {};
        for (const auto& it : _segments) {
            list.push_back(const_cast<vm::segment*>(&it.second));
        }
        return list;
    }

    bool assembler::initialize(common::result& r) {
        _location_counter = _terp->heap_vector(heap_vectors_t::program_start);
        return true;
    }

    instruction_block* assembler::current_block() {
        if (_block_stack.empty())
            return nullptr;
        return _block_stack.top();
    }

    bool assembler::allocate_reg(register_t& reg) {
        return _register_allocator.allocate(reg);
    }

    void assembler::free_reg(const register_t& reg) {
        _register_allocator.free(reg);
    }

    bool assembler::resolve_labels(common::result& r) {
        auto label_refs = label_references();
        for (auto label_ref : label_refs) {
            label_ref->resolved = find_label(label_ref->name);
            if (label_ref->resolved == nullptr) {
                r.error(
                    "A001",
                    fmt::format(
                        "unable to resolve label: {}",
                        label_ref->name));
            }
        }

        for (auto block : _blocks) {
            for (auto& entry : block->entries()) {
                switch (entry.type()) {
                    case block_entry_type_t::instruction: {
                        auto inst = entry.data<instruction_t>();
                        for (size_t i = 0; i < inst->operands_count; i++) {
                            auto& operand = inst->operands[i];
                            if (operand.is_unresolved()) {
                                auto label_ref = find_label_ref(static_cast<uint32_t>(operand.value.u));
                                if (label_ref != nullptr
                                &&  label_ref->resolved != nullptr) {
                                    operand.value.u = label_ref->resolved->address();
                                    operand.clear_unresolved();
                                }
                            }
                        }
                        break;
                    }
                    case block_entry_type_t::data_definition: {
                        auto data_def = entry.data<data_definition_t>();
                        if (data_def->type == data_definition_type_t::uninitialized)
                            break;
                        for (size_t i = 0; i < data_def->values.size(); i++) {
                            auto variant = data_def->values[i];
                            if (variant.which() == 1) {
                                auto label_ref = boost::get<label_ref_t*>(variant);
                                if (label_ref != nullptr) {
                                    data_def->values[i] = label_ref->resolved->address();
                                }
                            }
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }

        return !r.is_failed();
    }

    instruction_block* assembler::make_basic_block() {
        auto block = new instruction_block(instruction_block_type_t::basic);
        register_block(block);
        return block;
    }

    bool assembler::apply_addresses(common::result& r) {
        size_t offset = 0;
        for (auto block : _blocks) {
            if (!block->should_emit())
                continue;

            for (auto& entry : block->entries()) {
                entry.address(_location_counter + offset);
                switch (entry.type()) {
                    case block_entry_type_t::align: {
                        auto alignment = entry.data<align_t>();
                        offset = common::align(offset, alignment->size);
                        entry.address(_location_counter + offset);
                        break;
                    }
                    case block_entry_type_t::section: {
                        auto section = entry.data<section_t>();
                        switch (*section) {
                            case section_t::unknown:
                                break;
                            case section_t::bss:
                                break;
                            case section_t::text:
                                break;
                            case section_t::data:
                                break;
                            case section_t::ro_data:
                                break;
                        }
                        break;
                    }
                    case block_entry_type_t::instruction: {
                        auto inst = entry.data<instruction_t>();
                        offset += inst->encoding_size();
                        break;
                    }
                    case block_entry_type_t::data_definition: {
                        auto data_def = entry.data<data_definition_t>();
                        auto size_in_bytes = op_size_in_bytes(data_def->size);
                        switch (data_def->type) {
                            case data_definition_type_t::initialized: {
                                offset += size_in_bytes * data_def->values.size();
                                break;
                            }
                            case data_definition_type_t::uninitialized: {
                                for (auto v : data_def->values)
                                    offset += size_in_bytes * boost::get<uint64_t>(v);
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }
        return !r.is_failed();
    }

    void assembler::push_block(instruction_block* block) {
        _block_stack.push(block);
    }

    label* assembler::find_label(const std::string& name) {
        const auto it = _labels.find(name);
        if (it == _labels.end())
            return nullptr;
        return it->second;
    }

    void assembler::register_block(instruction_block* block) {
        auto source_file = _listing.current_source_file();
        if (source_file != nullptr)
            block->source_file(source_file);
        _blocks.push_back(block);
        _block_registry.insert(std::make_pair(block->id(), block));
    }

    label_ref_t* assembler::find_label_ref(common::id_t id) {
        auto it = _unresolved_labels.find(id);
        if (it != _unresolved_labels.end()) {
            return &it->second;
        }
        return nullptr;
    }

    vm::segment* assembler::segment(const std::string& name) {
        auto it = _segments.find(name);
        if (it == _segments.end())
            return nullptr;
        return &it->second;
    }

    void assembler::disassemble(instruction_block* block) {
        if (!block->should_emit())
            return;

        auto source_file = block->source_file();
        if (source_file == nullptr)
            return;

        std::stack<vm::comment_t> post_inst_comments {};

        size_t last_indent = 0;
        auto indent_four_spaces = std::string(4, ' ');

        for (auto& entry : block->entries()) {
            std::stringstream line {};
            listing_source_line_type_t type;

            switch (entry.type()) {
                case block_entry_type_t::meta: {
                    type = listing_source_line_type_t::directive;
                    auto meta = entry.data<meta_t>();
                    line << fmt::format(".meta '{}'", meta->label);
                    break;
                }
                case block_entry_type_t::label: {
                    type = listing_source_line_type_t::label;
                    auto label = entry.data<label_t>();
                    line << fmt::format("{}:", label->instance->name());
                    break;
                }
                case block_entry_type_t::blank_line: {
                    source_file->add_blank_lines(entry.address(), 1);
                    continue;
                }
                case block_entry_type_t::comment: {
                    type = listing_source_line_type_t::comment;
                    auto comment = entry.data<comment_t>();
                    if (comment->location == comment_location_t::after_instruction) {
                        post_inst_comments.push(*comment);
                        continue;
                    } else {
                        std::string indent;
                        if (comment->indent > 0)
                            indent = std::string(comment->indent, ' ');
                        line << fmt::format("{}; {}", indent, comment->value);
                    }
                    break;
                }
                case block_entry_type_t::align: {
                    type = listing_source_line_type_t::directive;
                    auto align = entry.data<align_t>();
                    line << fmt::format(".align {}", align->size);
                    break;
                }
                case block_entry_type_t::section: {
                    type = listing_source_line_type_t::directive;
                    auto section = entry.data<section_t>();
                    line << fmt::format(".section '{}'", section_name(*section));
                    break;
                }
                case block_entry_type_t::instruction: {
                    type = listing_source_line_type_t::instruction;
                    auto inst = entry.data<instruction_t>();
                    auto stream = inst->disassemble([&](uint64_t id) -> std::string {
                        auto label_ref = find_label_ref(static_cast<common::id_t>(id));
                        if (label_ref != nullptr) {
                            if (label_ref->resolved != nullptr) {
                                return fmt::format(
                                    "{} (${:08x})",
                                    label_ref->name,
                                    label_ref->resolved->address());
                            } else {
                                return label_ref->name;
                            }
                        }
                        return fmt::format("unresolved_ref_id({})", id);
                    });
                    line << fmt::format("{}{}", indent_four_spaces, stream);
                    break;
                }
                case block_entry_type_t::data_definition: {
                    type = listing_source_line_type_t::data_definition;
                    auto definition = entry.data<data_definition_t>();
                    std::stringstream directive;
                    std::string format_spec;
                    switch (definition->type) {
                        case data_definition_type_t::none: {
                            break;
                        }
                        case data_definition_type_t::initialized: {
                            switch (definition->size) {
                                case op_sizes::byte:
                                    directive << ".db";
                                    format_spec = "${:02X}";
                                    break;
                                case op_sizes::word:
                                    directive << ".dw";
                                    format_spec = "${:04X}";
                                    break;
                                case op_sizes::dword:
                                    directive << ".dd";
                                    format_spec = "${:08X}";
                                    break;
                                case op_sizes::qword:
                                    directive << ".dq";
                                    format_spec = "${:016X}";
                                    break;
                                default: {
                                    break;
                                }
                            }
                            break;
                        }
                        case data_definition_type_t::uninitialized: {
                            format_spec = "${:04X}";
                            switch (definition->size) {
                                case op_sizes::byte:
                                    directive << ".rb";
                                    break;
                                case op_sizes::word:
                                    directive << ".rw";
                                    break;
                                case op_sizes::dword:
                                    directive << ".rd";
                                    break;
                                case op_sizes::qword:
                                    directive << ".rq";
                                    break;
                                default: {
                                    break;
                                }
                            }
                            break;
                        }
                    }

                    auto item_index = 0;
                    auto item_count = definition->values.size();
                    std::string items;
                    while (item_count > 0) {
                        if (!items.empty())
                            items += ", ";
                        auto v = definition->values[item_index++];
                        if (v.which() == 0)
                            items += fmt::format(format_spec, boost::get<uint64_t>(v));
                        else
                            items += boost::get<label_ref_t*>(v)->name;
                        if ((item_index % 8) == 0) {
                            source_file->add_source_line(
                                listing_source_line_type_t::data_definition,
                                entry.address(),
                                fmt::format("{}{:<10}{}", indent_four_spaces, directive.str(), items));
                            items.clear();
                        }
                        --item_count;
                    }
                    if (!items.empty()) {
                        line << fmt::format(
                            "{}{:<10}{}",
                            indent_four_spaces,
                            directive.str(),
                            items);
                    }
                    break;
                }
            }

            if (!post_inst_comments.empty()) {
                auto& top = post_inst_comments.top();
                auto len = line.str().length();
                size_t indent_len;
                if (len == 0)
                    indent_len = last_indent;
                else
                    indent_len = std::max<int64_t>(0, 60 - len);
                std::string indent(indent_len, ' ');
                line << fmt::format("{}; {}", indent, top.value);
                last_indent = len + indent_len;
                post_inst_comments.pop();
            }

            source_file->add_source_line(
                type,
                entry.address(),
                line.str());

            while (!post_inst_comments.empty()) {
                std::stringstream temp {};
                auto& top = post_inst_comments.top();
                std::string indent(last_indent, ' ');
                temp << fmt::format("{}; {}", indent, top.value);
                post_inst_comments.pop();
                source_file->add_source_line(
                    listing_source_line_type_t::comment,
                    entry.address(),
                    temp.str());
            }
        }
    }

    std::vector<label_ref_t*> assembler::label_references() {
        std::vector<label_ref_t*> refs {};
        for (auto& kvp : _unresolved_labels) {
            refs.push_back(&kvp.second);
        }
        return refs;
    }

    vm::label* assembler::make_label(const std::string& name) {
        auto it = _labels.insert(std::make_pair(name, new vm::label(name)));
        return it.first->second;
    }

    instruction_block* assembler::block(common::id_t id) const {
        auto it = _block_registry.find(id);
        if (it == _block_registry.end())
            return nullptr;
        return it->second;
    }

    label_ref_t* assembler::make_label_ref(const std::string& label_name) {
        auto it = _label_to_unresolved_ids.find(label_name);
        if (it != _label_to_unresolved_ids.end()) {
            auto ref_it = _unresolved_labels.find(it->second);
            if (ref_it != _unresolved_labels.end())
                return &ref_it->second;
        }

        auto label = find_label(label_name);
        auto ref_id = common::id_pool::instance()->allocate();
        auto insert_pair = _unresolved_labels.insert(std::make_pair(
            ref_id,
            label_ref_t {
                .id = ref_id,
                .name = label_name,
                .resolved = label
            }));
        _label_to_unresolved_ids.insert(std::make_pair(label_name, ref_id));

        return &insert_pair.first.operator->()->second;
    }

};