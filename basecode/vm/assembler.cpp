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
#include "instruction_block.h"

namespace basecode::vm {

    assembler::assembler(vm::terp* terp) : _terp(terp) {
    }

    assembler::~assembler() {
        for (const auto& it : _labels)
            delete it.second;
        _labels.clear();

        for (auto block : _blocks)
            delete block;
        _blocks.clear();
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
                                    r.add_message(
                                        "A031",
                                        fmt::format("unexpected label_ref_t*: {}", label_ref->name),
                                        true);
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
            common::source_file& source_file) {
        enum class assembly_parser_state_t : uint8_t {
            start,
            whitespace,
            comment,
            label,
            mnemonic,
            operand_list,
            encode_instruction,
            encode_directive,
            directive,
            directive_param_list
        };

        enum class wip_type_t {
            none = 0,
            mnemonic,
            directive
        };

        struct wip_t {
            void reset() {
                code.clear();
                params.clear();
                is_valid = false;
                operands.clear();
                instance.m = nullptr;
                size = op_sizes::none;
                type = wip_type_t::none;
            }

            std::string code {};
            bool is_valid = false;
            union {
                mnemonic_t* m = nullptr;
                directive_t* d;
            } instance;
            op_sizes size = op_sizes::none;
            wip_type_t type = wip_type_t::none;
            std::vector<operand_encoding_t> operands {};
            std::vector<directive_param_variant_t> params {};
        };

        auto block = make_basic_block();
        auto state = assembly_parser_state_t::start;
        wip_t wip {};
        size_t start_pos = 0;

        auto make_location = [&](size_t end_pos) -> common::source_location {
            auto start_line = source_file.line_by_index(start_pos);
            auto start_column = source_file.column_by_index(start_pos);
            auto end_line = source_file.line_by_index(end_pos);
            auto end_column = source_file.column_by_index(end_pos);

            common::source_location location;
            location.start(start_line->line, start_column);
            location.end(end_line->line, end_column);
            return location;
        };

        auto parse_immediate_number = [&](
                const std::string& param,
                uint64_t& value) -> bool {
            auto first_char = param[0];
            if (first_char == '#') {
                syntax::token_t number;
                if (param[1] == '$') {
                    number.value = param.substr(2);
                    number.radix = 16;
                } else if (param[1] == '%') {
                    number.value = param.substr(2);
                    number.radix = 2;
                } else if (param[1] == '@') {
                    number.value = param.substr(2);
                    number.radix = 8;
                } else {
                    number.value = param.substr(1);
                    number.radix = 10;
                }

                if (number.parse(value) != syntax::conversion_result_t::success) {
                    source_file.error(
                        r,
                        "A005",
                        "invalid numeric param value, did you forget the #?",
                        make_location(source_file.pos()));
                    return false;
                }
            } else {
                source_file.error(
                    r,
                    "A005",
                    "invalid numeric param value, did you forget the #?",
                    make_location(source_file.pos()));
                return false;
            }

            return true;
        };

        auto parse_comma_separated_tokens = [&](
                common::rune_t& rune,
                std::vector<std::string>& operand_strings) {
            std::stringstream operand_stream;
            auto add_operand_to_list = [&]() {
                operand_strings.push_back(operand_stream.str());
                operand_stream.str("");
            };

            while (true) {
                if (!isspace(rune)) {
                    if (rune == ',') {
                        add_operand_to_list();
                    } else if (rune == ';') {
                        add_operand_to_list();
                        state = assembly_parser_state_t::comment;
                        break;
                    } else {
                        operand_stream << static_cast<char>(rune);
                    }
                }
                if (rune == '\n') {
                    add_operand_to_list();
                    if (wip.type == wip_type_t::mnemonic)
                        state = assembly_parser_state_t::encode_instruction;
                    else
                        state = assembly_parser_state_t::encode_directive;
                    break;
                }
                rune = source_file.next(r);
            }
        };

        while (true) {
            auto rune = source_file.next(r);
            if (rune == common::rune_eof)
                break;

            retry:
            switch (state) {
                case assembly_parser_state_t::start: {
                    state = assembly_parser_state_t::whitespace;
                    goto retry;
                }
                case assembly_parser_state_t::whitespace: {
                    if (!isspace(rune)) {
                        start_pos = source_file.pos();
                        if (rune == ';') {
                            state = assembly_parser_state_t::comment;
                        } else if (rune == '.') {
                            state = assembly_parser_state_t::directive;
                        } else if (isalpha(rune) || rune == '_') {
                            source_file.push_mark();

                            state = assembly_parser_state_t::mnemonic;

                            while (true) {
                                auto next_rune = source_file.next(r);
                                if (next_rune == ':') {
                                    state = assembly_parser_state_t::label;
                                    break;
                                } else if (next_rune == '\n') {
                                    break;
                                }
                            }

                            source_file.seek(source_file.pop_mark());
                            goto retry;
                        }
                    }
                    break;
                }
                case assembly_parser_state_t::comment: {
                    std::stringstream stream;
                    while (true) {
                        stream << static_cast<char>(rune);
                        rune = source_file.next(r);
                        if (rune == '\n')
                            break;
                    }
                    block->comment(stream.str(), 4);

                    if (wip.is_valid) {
                        if (wip.type == wip_type_t::mnemonic)
                            state = assembly_parser_state_t::encode_instruction;
                        else
                            state = assembly_parser_state_t::encode_directive;
                    }
                    else
                        state = assembly_parser_state_t::whitespace;
                    break;
                }
                case assembly_parser_state_t::label: {
                    std::stringstream stream;
                    while (true) {
                        stream << static_cast<char>(rune);
                        rune = source_file.next(r);
                        if (rune == ':')
                            break;
                    }
                    block->label(make_label(stream.str()));
                    state = assembly_parser_state_t::whitespace;
                    break;
                }
                case assembly_parser_state_t::mnemonic: {
                    size_t end_pos = 0;
                    std::stringstream stream;
                    while (true) {
                        stream << static_cast<char>(rune);
                        rune = source_file.next(r);
                        if (!isalpha(rune)) {
                            wip.code = stream.str();
                            wip.type = wip_type_t::mnemonic;
                            end_pos = source_file.pos();

                            if (rune == '.') {
                                rune = source_file.next(r);
                                switch (rune) {
                                    case 'b': {
                                        wip.size = op_sizes::byte;
                                        break;
                                    }
                                    case 'w': {
                                        wip.size = op_sizes::word;
                                        break;
                                    }
                                    case 'd': {
                                        rune = source_file.next(r);
                                        if (rune == 'w') {
                                            wip.size = op_sizes::dword;
                                            break;
                                        }
                                        break;
                                    }
                                    case 'q': {
                                        rune = source_file.next(r);
                                        if (rune == 'w') {
                                            wip.size = op_sizes::qword;
                                            break;
                                        }
                                        break;
                                    }
                                    default:
                                        break;
                                }
                                break;
                            }
                            break;
                        }
                    }

                    common::to_upper(wip.code);
                    wip.instance.m = mnemonic(wip.code);
                    if (wip.instance.m == nullptr) {
                        source_file.error(
                            r,
                            "A003",
                            "unknown mnemonic.",
                            make_location(end_pos));
                        return false;
                    }

                    if (wip.instance.m->operands.size() == 0) {
                        wip.is_valid = true;
                        state = assembly_parser_state_t::encode_instruction;
                    }
                    else
                        state = assembly_parser_state_t::operand_list;
                    break;
                }
                case assembly_parser_state_t::operand_list: {
                    auto required_operand_count = wip.instance.m->required_operand_count();
                    auto commas_found = 0;
                    source_file.push_mark();
                    auto test_rune = rune;
                    while (test_rune != '\n' && test_rune != ';') {
                        if (test_rune == ',')
                            commas_found++;
                        test_rune = source_file.next(r);
                    }

                    if (commas_found < required_operand_count - 1) {
                        source_file.error(
                            r,
                            "A004",
                            fmt::format(
                                "mnemonic '{}' requires '{}' operands.",
                                wip.code,
                                required_operand_count),
                            make_location(source_file.pos()));
                        return false;
                    }

                    source_file.seek(source_file.pop_mark());

                    std::vector<std::string> operands {};
                    parse_comma_separated_tokens(rune, operands);

                    for (const auto& operand : operands) {
                        operand_encoding_t encoding;

                        if (operand[0] == '#') {
                            uint64_t value;
                            if (!parse_immediate_number(operand, value))
                                return false;
                            encoding.type = operand_encoding_t::flags::integer
                                | operand_encoding_t::flags::constant;
                            encoding.value.u = value;
                        } else if (is_integer_register(operand)) {
                            auto number = std::atoi(operand.substr(1).c_str());
                            encoding.type = operand_encoding_t::flags::integer
                                | operand_encoding_t::flags::reg;
                            encoding.value.r = static_cast<uint8_t>(number);
                        } else if (is_float_register(operand)) {
                            if (operand[1] == 'P' || operand[1] == 'p') {
                                encoding.type = operand_encoding_t::flags::reg;
                                encoding.value.r = static_cast<uint8_t>(registers_t::fp);
                            } else {
                                auto number = std::atoi(operand.substr(1).c_str());
                                encoding.type = operand_encoding_t::flags::reg;
                                encoding.value.r = static_cast<uint8_t>(number);
                            }
                        } else if ((operand[0] == 'S' || operand[0] == 's')
                                && (operand[1] == 'P' || operand[1] == 'p')) {
                            encoding.type = operand_encoding_t::flags::reg;
                            encoding.value.r = static_cast<uint8_t>(registers_t::sp);
                        } else if ((operand[0] == 'P' || operand[0] == 'p')
                                && (operand[1] == 'C' || operand[1] == 'c')) {
                            encoding.type = operand_encoding_t::flags::reg;
                            encoding.value.r = static_cast<uint8_t>(registers_t::pc);
                        } else {
                            encoding.type = operand_encoding_t::flags::integer
                                            | operand_encoding_t::flags::constant
                                            | operand_encoding_t::flags::unresolved;
                            auto label_ref = make_label_ref(operand);
                            encoding.value.u = label_ref->id;
                        }

                        wip.operands.push_back(encoding);
                    }

                    wip.is_valid = wip.operands.size() == required_operand_count;
                    goto retry;
                }
                case assembly_parser_state_t::encode_instruction: {
                    if (!wip.is_valid) {
                        source_file.error(
                            r,
                            "A005",
                            "invalid instruction encoding.",
                            make_location(source_file.pos()));
                        return false;
                    }

                    instruction_t inst;
                    inst.size = wip.size;
                    inst.op = wip.instance.m->code;
                    inst.operands_count = static_cast<uint8_t>(wip.operands.size());

                    size_t index = 0;
                    for (const auto& operand : wip.operands)
                        inst.operands[index++] = operand;

                    block->make_block_entry(inst);

                    wip.reset();
                    state = assembly_parser_state_t::whitespace;
                    break;
                }
                case assembly_parser_state_t::encode_directive: {
                    if (!wip.is_valid) {
                        source_file.error(
                            r,
                            "A005",
                            "invalid directive encoding.",
                            make_location(source_file.pos()));
                        return false;
                    }

                    data_definition_t data_def {};

                    switch (wip.instance.d->type) {
                        case directive_type_t::section: {
                            block->make_block_entry(section_type(boost::get<std::string>(wip.params.front())));
                            break;
                        }
                        case directive_type_t::align: {
                            block->make_block_entry(align_t{
                                .size = static_cast<uint8_t>(boost::get<uint64_t>(wip.params.front()))
                            });
                            break;
                        }
                        case directive_type_t::db: {
                            data_def.type = data_definition_type_t::initialized;
                            data_def.size = op_sizes::byte;
                            break;
                        }
                        case directive_type_t::dw: {
                            data_def.type = data_definition_type_t::initialized;
                            data_def.size = op_sizes::word;
                            break;
                        }
                        case directive_type_t::dd: {
                            data_def.type = data_definition_type_t::initialized;
                            data_def.size = op_sizes::dword;
                            break;
                        }
                        case directive_type_t::dq: {
                            data_def.type = data_definition_type_t::initialized;
                            data_def.size = op_sizes::qword;
                            break;
                        }
                        case directive_type_t::rb: {
                            data_def.type = data_definition_type_t::uninitialized;
                            data_def.size = op_sizes::qword;
                            break;
                        }
                        case directive_type_t::rw: {
                            data_def.type = data_definition_type_t::uninitialized;
                            data_def.size = op_sizes::qword;
                            break;
                        }
                        case directive_type_t::rd: {
                            data_def.type = data_definition_type_t::uninitialized;
                            data_def.size = op_sizes::qword;
                            break;
                        }
                        case directive_type_t::rq: {
                            data_def.type = data_definition_type_t::uninitialized;
                            data_def.size = op_sizes::qword;
                            break;
                        }
                    }

                    if (data_def.type != data_definition_type_t::none) {
                        for (const auto& data : wip.params)
                            data_def.values.push_back(boost::get<uint64_t>(data));
                        block->make_block_entry(data_def);
                    }

                    wip.reset();
                    state = assembly_parser_state_t::whitespace;
                    break;
                }
                case assembly_parser_state_t::directive: {
                    std::stringstream stream;
                    size_t end_pos = 0;
                    while (true) {
                        stream << static_cast<char>(rune);
                        rune = source_file.next(r);
                        if (!isalpha(rune)) {
                            end_pos = source_file.pos() - 2;
                            wip.code = stream.str();
                            wip.type = wip_type_t::directive;
                            break;
                        }
                    }

                    common::to_upper(wip.code);
                    wip.instance.d = directive(wip.code);
                    if (wip.instance.d == nullptr) {
                        source_file.error(
                            r,
                            "A003",
                            "unknown directive.",
                            make_location(end_pos));
                        return false;
                    }

                    if (wip.instance.d->params.size() == 0)
                        state = assembly_parser_state_t::whitespace;
                    else
                        state = assembly_parser_state_t::directive_param_list;
                    break;
                }
                case assembly_parser_state_t::directive_param_list: {
                    auto required_params_count = wip.instance.d->required_operand_count();

                    std::vector<std::string> params {};
                    parse_comma_separated_tokens(rune, params);

                    // XXX: must validate the size for numbers
                    if (wip.instance.d->params[0].is_repeating()) {
                        for (const auto& param : params) {
                            uint64_t value;
                            if (!parse_immediate_number(param, value))
                                return false;
                            wip.params.push_back(value);
                        }
                    } else {
                        auto param_index = 0;
                        for (const auto& param_def : wip.instance.d->params) {
                            if (param_index >= params.size())
                                break;
                            auto param = params[param_index++];
                            if (param_def.is_number()) {
                                uint64_t value;
                                if (!parse_immediate_number(param, value))
                                    return false;
                                wip.params.push_back(value);
                            } else if (param_def.is_string()) {
                                if (param[0] == '\'') {
                                    auto param_char_idx = 1;
                                    std::stringstream stream;
                                    while (true) {
                                        if (param_char_idx >= param.length()) {
                                            source_file.error(
                                                r,
                                                "A005",
                                                "invalid string param value, did you forget the closing '?",
                                                make_location(source_file.pos()));
                                            return false;
                                        }
                                        auto token = param[param_char_idx++];
                                        if (token == '\'')
                                            break;
                                        stream << token;
                                    }
                                    wip.params.push_back(stream.str());
                                } else {
                                    source_file.error(
                                        r,
                                        "A005",
                                        "invalid string param value, did you forget the leading '?",
                                        make_location(source_file.pos()));
                                    return false;
                                }
                            } else {
                                source_file.error(
                                    r,
                                    "A005",
                                    "unknown param type.",
                                    make_location(source_file.pos()));
                                return false;
                            }
                        }
                    }

                    wip.is_valid = wip.params.size() >= required_params_count;
                    goto retry;
                }
            }
        }

        return true;
    }

    instruction_block* assembler::pop_block() {
        if (_block_stack.empty())
            return nullptr;
        auto top = _block_stack.top();
        if (top->type() == instruction_block_type_t::procedure && _procedure_block_count > 0)
            _procedure_block_count--;
        _block_stack.pop();
        return top;
    }

    vm::assembly_listing& assembler::listing() {
        return _listing;
    }

    bool assembler::in_procedure_scope() const {
        return _procedure_block_count > 0;
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
                r.add_message(
                    "A001",
                    fmt::format(
                        "unable to resolve label: {}",
                        label_ref->name),
                    true);
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
        add_new_block(block);
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

    std::vector<instruction_block*>& assembler::blocks() {
        return _blocks;
    }

    void assembler::push_block(instruction_block* block) {
        _block_stack.push(block);
        if (block->type() == instruction_block_type_t::procedure)
            _procedure_block_count++;
    }

    instruction_block* assembler::make_procedure_block() {
        auto block = new instruction_block(instruction_block_type_t::procedure);
        add_new_block(block);
        return block;
    }

    label* assembler::find_label(const std::string& name) {
        const auto it = _labels.find(name);
        if (it == _labels.end())
            return nullptr;
        return it->second;
    }

    void assembler::add_new_block(instruction_block* block) {
        auto source_file = _listing.current_source_file();
        if (source_file != nullptr)
            block->source_file(source_file);
        _blocks.push_back(block);
    }

    vm::segment* assembler::segment(const std::string& name) {
        auto it = _segments.find(name);
        if (it == _segments.end())
            return nullptr;
        return &it->second;
    }

    label_ref_t* assembler::find_label_ref(common::id_t id) {
        auto it = _unresolved_labels.find(id);
        if (it != _unresolved_labels.end()) {
            return &it->second;
        }
        return nullptr;
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

            switch (entry.type()) {
                case block_entry_type_t::meta: {
                    auto meta = entry.data<meta_t>();
                    line << fmt::format(".meta '{}'", meta->label);
                    break;
                }
                case block_entry_type_t::label: {
                    auto label = entry.data<label_t>();
                    line << fmt::format("{}:", label->instance->name());
                    break;
                }
                case block_entry_type_t::blank_line: {
                    source_file->add_blank_lines(entry.address(), 1);
                    continue;
                }
                case block_entry_type_t::comment: {
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
                    auto align = entry.data<align_t>();
                    line << fmt::format(".align {}", align->size);
                    break;
                }
                case block_entry_type_t::section: {
                    auto section = entry.data<section_t>();
                    line << fmt::format(".section '{}'", section_name(*section));
                    break;
                }
                case block_entry_type_t::instruction: {
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

            source_file->add_source_line(entry.address(), line.str());

            while (!post_inst_comments.empty()) {
                std::stringstream temp {};
                auto& top = post_inst_comments.top();
                std::string indent(last_indent, ' ');
                temp << fmt::format("{}; {}", indent, top.value);
                post_inst_comments.pop();
                source_file->add_source_line(entry.address(), temp.str());
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

    bool assembler::is_float_register(const std::string& value) const {
        if (value.length() > 3)
            return false;

        if (value[0] != 'f' && value[0] != 'F')
            return false;

        if (value.length() == 2) {
            return static_cast<bool>(isdigit(value[1]));
        } else {
            return isdigit(value[1]) && isdigit(value[2]);
        }
    }

    bool assembler::is_integer_register(const std::string& value) const {
        if (value.length() > 3)
            return false;

        if (value[0] != 'i' && value[0] != 'I')
            return false;

        if (value.length() == 2) {
            return static_cast<bool>(isdigit(value[1]));
        } else {
            return isdigit(value[1]) && isdigit(value[2]);
        }
    }

};