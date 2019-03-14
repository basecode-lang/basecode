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

#include <iomanip>
#include <parser/token.h>
#include <common/bytes.h>
#include <common/string_support.h>
#include <common/source_location.h>
#include "label.h"
#include "assembler.h"
#include "basic_block.h"
#include "assembly_parser.h"

namespace basecode::vm {

    assembler::assembler(
            vm::terp* terp,
            vm::register_allocator* allocator) : _terp(terp),
                                                 _register_allocator(allocator) {
    }

    bool assembler::assemble(
            common::result& r,
            const label_map& labels) {
        if (!apply_addresses(r))
            return false;

        if (!resolve_labels(r, labels))
            return false;

        uint64_t highest_address = 0;

        for (auto block : _blocks) {
            for (auto& entry : block->entries()) {
                highest_address = entry.address();

                switch (entry.type()) {
                    case block_entry_type_t::reset: {
                        auto reset = entry.data<reset_t>();
                        if (reset->type == "local") {
                            for (const auto& kvp : _locals) {
                                const assembler_local_t& local = kvp.second;
                                _register_allocator->release(local.reg);
                            }
                            _locals.clear();
                        } else if (reset->type == "frame") {
                            _frame_offsets.clear();
                        } else {
                            r.error(
                                "A031",
                                fmt::format("unknown reset operand: {}", reset->type));
                            return false;
                        }
                        break;
                    }
                    case block_entry_type_t::local: {
                        auto data = entry.data<local_t>();
                        assembler_local_t local {};
                        local.name = data->name;
                        local.offset = data->offset;
                        switch (data->type) {
                            case local_type_t::integer: {
                                local.reg.type = register_type_t::integer;
                                break;
                            }
                            case local_type_t::floating_point: {
                                local.reg.type = register_type_t::floating_point;
                                break;
                            }
                        }

                        if (!_register_allocator->allocate(local.reg))
                            return false;

                        if (!data->frame_offset.empty()) {
                            auto it = _frame_offsets.find(data->frame_offset);
                            if (it != _frame_offsets.end()) {
                                local.offset += it->second;
                            }
                        }

                        _locals.insert(std::make_pair(data->name, local));
                        break;
                    }
                    case block_entry_type_t::frame_offset: {
                        auto data = entry.data<frame_offset_t>();
                        _frame_offsets.insert(std::make_pair(data->name, data->offset));
                        break;
                    }
                    case block_entry_type_t::instruction: {
                        auto inst = entry.data<instruction_t>();

                        for (size_t i = 0; i < inst->operands_count; i++) {
                            auto& operand = inst->operands[i];
                            if (operand.fixup_ref1 == nullptr)
                                continue;

                            switch (operand.fixup_ref1->type) {
                                case assembler_named_ref_type_t::local: {
                                    auto it = _locals.find(operand.fixup_ref1->name);
                                    if (it != _locals.end()) {
                                        const auto& local1 = it->second;

                                        if (operand.fixup_ref2 != nullptr) {
                                            it = _locals.find(operand.fixup_ref2->name);
                                            if (it != _locals.end()) {
                                                const auto& local2 = it->second;

                                                operand.size = op_sizes::qword;
                                                operand.type = operand_encoding_t::flags::reg
                                                    | operand_encoding_t::flags::range;
                                                if (local1.reg.type == register_type_t::integer)
                                                    operand.type |= operand_encoding_t::flags::integer;
                                                operand.value.u = static_cast<uint64_t>(
                                                    static_cast<uint16_t>(local1.reg.number) << 8
                                                    | (static_cast<uint16_t>(local2.reg.number) & 0x00ff));
                                            }
                                        } else {
                                            operand.value.r = local1.reg.number;
                                            operand.size = operand.fixup_ref1->size;
                                            switch (local1.reg.type) {
                                                case register_type_t::integer: {
                                                    operand.type |= operand_encoding_t::flags::integer;
                                                    break;
                                                }
                                                default: {
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                                case assembler_named_ref_type_t::offset: {
                                    auto it = _locals.find(operand.fixup_ref1->name);
                                    if (it != _locals.end()) {
                                        const auto& local = it->second;
                                        auto imm_value = local.offset;
                                        if (imm_value < 0) {
                                            operand.type |= operand_encoding_t::flags::negative;
                                            imm_value = -imm_value;
                                        }
                                        operand.size = operand.fixup_ref1->size;
                                        operand.value.u = static_cast<uint64_t>(imm_value);
                                    }
                                    break;
                                }
                                default: {
                                    break;
                                }
                            }
                        }

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
                                    auto label_ref = boost::get<assembler_named_ref_t*>(v);
                                    r.error(
                                        "A031",
                                        fmt::format("unexpected assembler_named_ref_t*: {}", label_ref->name));
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

    void assembler::disassemble() {
        for (auto block : _blocks)
            disassemble(block);
    }

    bool assembler::resolve_labels(
            common::result& r,
            const label_map& labels) {
        for (auto block : _blocks) {
            for (auto& entry : block->entries()) {
                switch (entry.type()) {
                    case block_entry_type_t::instruction: {
                        auto inst = entry.data<instruction_t>();
                        for (size_t i = 0; i < inst->operands_count; i++) {
                            auto& operand = inst->operands[i];
                            if (operand.fixup_ref1 == nullptr)
                                continue;

                            switch (operand.fixup_ref1->type) {
                                case assembler_named_ref_type_t::label: {
                                    auto label = labels.find(operand.fixup_ref1->name);
                                    if (label != nullptr) {
                                        operand.value.u = label->address();
                                    }
                                    break;
                                }
                                default: {
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    case block_entry_type_t::data_definition: {
                        auto data_def = entry.data<data_definition_t>();
                        if (data_def->type == data_definition_type_t::uninitialized)
                            break;
                        for (auto& value : data_def->values) {
                            auto variant = value;
                            if (variant.which() == 1) {
                                auto named_ref = boost::get<assembler_named_ref_t*>(variant);
                                if (named_ref != nullptr) {
                                    switch (named_ref->type) {
                                        case assembler_named_ref_type_t::label: {
                                            auto label = labels.find(named_ref->name);
                                            if (label != nullptr) {
                                                value = label->address();
                                            }
                                            break;
                                        }
                                        default: {
                                            break;
                                        }
                                    }
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
            label_map& labels,
            common::source_file& source_file,
            vm::basic_block* block,
            void* data) {
        vm::assembly_parser parser(this, &labels, source_file, data);
        return parser.parse(r, block);
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

    vm::basic_block_list_t& assembler::blocks() {
        return _blocks;
    }

    bool assembler::initialize(common::result& r) {
        _location_counter = _terp->heap_vector(heap_vectors_t::program_start);
        return true;
    }

    assembler_named_ref_t* assembler::find_named_ref(
            const std::string& name,
            vm::assembler_named_ref_type_t type) {
        auto key = fmt::format("{}{}", name, static_cast<uint8_t>(type));
        auto it = _named_refs.find(key);
        if (it != _named_refs.end()) {
            return &it->second;
        }
        return nullptr;
    }

    assembler_named_ref_t* assembler::make_named_ref(
            assembler_named_ref_type_t type,
            const std::string& name,
            vm::op_sizes size) {
        auto key = fmt::format("{}{}", name, static_cast<uint8_t>(type));
        auto it = _named_refs.find(key);
        if (it != _named_refs.end()) {
            auto ref = &it->second;
            ref->size = size;
            return ref;
        }

        auto insert_pair = _named_refs.insert(std::make_pair(
            key,
            assembler_named_ref_t {
                .name = name,
                .size = size,
                .type = type
            }));
        return &insert_pair.first->second;
    }

    bool assembler::apply_addresses(common::result& r) {
        size_t offset = 0;
        for (auto block : _blocks) {
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

    void assembler::disassemble(basic_block* block) {
        auto source_file = _listing.current_source_file();
        if (source_file == nullptr || block->entries().empty())
            return;

        std::stack<vm::comment_t> post_inst_comments {};

        size_t last_indent = 0;
        auto indent_four_spaces = std::string(4, ' ');

        auto start_address = block->entries().front().address();
        auto pre_blank_lines = block->pre_blank_lines();
        if (pre_blank_lines > 0)
            source_file->add_blank_lines(start_address, pre_blank_lines);

        source_file->add_source_line(
            listing_source_line_type_t::directive,
            start_address,
            fmt::format(".block {}", block->id()));

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
                case block_entry_type_t::reset: {
                    type = listing_source_line_type_t::directive;
                    auto reset = entry.data<reset_t>();
                    line << indent_four_spaces;
                    line << std::left << std::setw(10) << ".reset";
                    line << fmt::format("'{}'", reset->type);
                    break;
                }
                case block_entry_type_t::local: {
                    type = listing_source_line_type_t::directive;

                    line << indent_four_spaces;

                    auto local = entry.data<local_t>();
                    switch (local->type) {
                        case local_type_t::integer: {
                            line << std::left << std::setw(10) << ".ilocal";
                            break;
                        }
                        case local_type_t::floating_point: {
                            line << std::left << std::setw(10) << ".flocal";
                            break;
                        }
                    }

                    line << fmt::format("'{}', {}", local->name, local->offset);

                    if (!local->frame_offset.empty())
                        line << fmt::format(", '{}'", local->frame_offset);

                    break;
                }
                case block_entry_type_t::label: {
                    type = listing_source_line_type_t::label;
                    auto label = entry.data<label_t>();
                    line << fmt::format("{}:", label->instance->name());
                    break;
                }
                case block_entry_type_t::align: {
                    type = listing_source_line_type_t::directive;
                    auto align = entry.data<align_t>();
                    line << fmt::format(".align {}", align->size);
                    break;
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
                case block_entry_type_t::section: {
                    type = listing_source_line_type_t::directive;
                    auto section = entry.data<section_t>();
                    line << fmt::format(".section '{}'", section_name(*section));
                    break;
                }
                case block_entry_type_t::frame_offset: {
                    type = listing_source_line_type_t::directive;
                    auto frame_offset = entry.data<frame_offset_t>();
                    line << indent_four_spaces;
                    line << std::left << std::setw(10) << ".frame";
                    line << fmt::format("'{}', {}", frame_offset->name, frame_offset->offset);
                    break;
                }
                case block_entry_type_t::blank_line: {
                    source_file->add_blank_lines(entry.address(), 1);
                    continue;
                }
                case block_entry_type_t::instruction: {
                    type = listing_source_line_type_t::instruction;
                    auto inst = entry.data<instruction_t>();
                    auto stream = inst->disassemble();
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
                            items += boost::get<assembler_named_ref_t*>(v)->name;
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
                    indent_len = (size_t) std::max<int64_t>(0, 60 - len);
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

        auto final_address = block->entries().back().address();
        source_file->add_source_line(
            listing_source_line_type_t::directive,
            final_address,
            ".end");

        auto post_blank_lines = block->post_blank_lines();
        if (post_blank_lines > 0)
            source_file->add_blank_lines(final_address, post_blank_lines);
    }

    bool assembler::has_local(const std::string& name) const {
        return _locals.count(name) > 0;
    }

    vm::segment* assembler::segment(const std::string& name) {
        auto it = _segments.find(name);
        if (it == _segments.end())
            return nullptr;
        return &it->second;
    }

    const assembly_symbol_resolver_t& assembler::resolver() const {
        return _resolver;
    }

    const int64_t assembler::frame_offset(const std::string& name) const {
        auto it = _frame_offsets.find(name);
        if (it == _frame_offsets.end())
            return 0;
        return it->second;
    }

    void assembler::resolver(const assembly_symbol_resolver_t& resolver) {
        _resolver = resolver;
    }

    const vm::assembler_local_t* assembler::local(const std::string& name) const {
        auto it = _locals.find(name);
        if (it == _locals.end())
            return nullptr;
        return &it->second;
    }

}