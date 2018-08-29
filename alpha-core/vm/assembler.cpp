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
#include "assembler.h"
#include "instruction_block.h"

namespace basecode::vm {

    assembler::assembler(vm::terp* terp) : _terp(terp) {
    }

    assembler::~assembler() {
        for (auto block : _blocks)
            delete block;
        _blocks.clear();
    }

    bool assembler::assemble(
            common::result& r,
            vm::instruction_block* block) {
        if (block == nullptr)
            block = current_block();

        current_block()->walk_blocks([&](instruction_block* block) -> bool {
            for (auto& entry : block->entries()) {
                switch (entry.type()) {
                    case block_entry_type_t::instruction: {
                        auto inst = entry.data<instruction_t>();
                        auto inst_size = inst->encode(
                            r,
                            _terp->heap(),
                            entry.address());
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
                                _terp->write(data_def->size, entry.address() + offset, v);
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
            return true;
        });

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

    //
    // comments:   ; ~~~~~~~~~~~~~~~
    // labels:     name: ~~~~~~~~~~~~~~~~~~~
    // mnemonics:  name[.b|.w|.dw|.qw]
    // operands:   register[, register|immediate][, register|immediate]
    // directives: .[name] [params] [,params]
    //
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
                    current_entry(block)->comment(stream.str(), 4);

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
                    auto label = block->make_label(stream.str());
                    current_entry(block)->label(label);
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

                    if (wip.instance.m->operands.size() == 0)
                        state = assembly_parser_state_t::whitespace;
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

                        auto first_char = toupper(operand[0]);
                        if (first_char == '#') {
                            uint64_t value;
                            if (!parse_immediate_number(operand, value))
                                return false;
                            encoding.type = operand_encoding_t::flags::integer
                                | operand_encoding_t::flags::constant;
                            encoding.value.u = value;
                        } else if (first_char == 'I') {
                            auto number = std::atoi(operand.substr(1).c_str());
                            encoding.type = operand_encoding_t::flags::integer
                                | operand_encoding_t::flags::reg;
                            encoding.value.r = static_cast<uint8_t>(number);
                        } else if (first_char == 'F') {
                            if (operand[1] == 'P') {
                                encoding.type = operand_encoding_t::flags::reg;
                                encoding.value.r = static_cast<uint8_t>(registers_t::fp);
                            } else {
                                auto number = std::atoi(operand.substr(1).c_str());
                                encoding.type = operand_encoding_t::flags::reg;
                                encoding.value.r = static_cast<uint8_t>(number);
                            }
                        } else if (first_char == 'S' && operand[1] == 'P') {
                            encoding.type = operand_encoding_t::flags::reg;
                            encoding.value.r = static_cast<uint8_t>(registers_t::sp);
                        } else if (first_char == 'P' && operand[1] == 'C') {
                            encoding.type = operand_encoding_t::flags::reg;
                            encoding.value.r = static_cast<uint8_t>(registers_t::pc);
                        }

                        wip.operands.push_back(encoding);
                    }

                    wip.is_valid = wip.operands.size() == required_operand_count;
                    break;
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

    void assembler::pop_target_register() {
        if (_target_registers.empty())
            return;
        _target_registers.pop();
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

    instruction_block* assembler::root_block() {
        if (_blocks.empty())
            return nullptr;
        return _blocks.front();
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

    register_t* assembler::current_target_register() {
        if (_target_registers.empty())
            return nullptr;
        return &_target_registers.top();
    }

    bool assembler::resolve_labels(common::result& r) {
        auto root_block = current_block();
        root_block->walk_blocks([&](instruction_block* block) -> bool {
            auto label_refs = block->label_references();
            for (auto label_ref : label_refs) {
                label_ref->resolved = root_block->find_label(label_ref->name);
                if (label_ref->resolved == nullptr) {
                    r.add_message(
                        "A001",
                        fmt::format("unable to resolve label: {}", label_ref->name),
                        true);
                    return false;
                }
            }

            for (auto& entry : block->entries()) {
                if (entry.type() != block_entry_type_t::instruction)
                    continue;

                auto inst = entry.data<instruction_t>();
                for (size_t i = 0; i < inst->operands_count; i++) {
                    auto& operand = inst->operands[i];
                    if (operand.is_unresolved()) {
                        auto label_ref = block->find_unresolved_label_up(
                            static_cast<uint32_t>(operand.value.u));
                        if (label_ref != nullptr) {
                            operand.value.u = label_ref->resolved->address();
                            operand.clear_unresolved();
                        }
                    }
                }
            }

            return true;
        });
        return !r.is_failed();
    }

    bool assembler::apply_addresses(common::result& r) {
        size_t offset = 0;
        current_block()->walk_blocks([&](instruction_block* block) -> bool {
            for (auto& entry : block->entries()) {
                entry.address(_location_counter + offset);
                switch (entry.type()) {
                    case block_entry_type_t::memo: {
                        break;
                    }
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
                    case block_entry_type_t::data_definition:
                        auto data_def = entry.data<data_definition_t>();
                        offset += op_size_in_bytes(data_def->size) * data_def->values.size();
                        break;
                }
            }
            return true;
        });
        return !r.is_failed();
    }

    void assembler::push_block(instruction_block* block) {
        _block_stack.push(block);
        if (block->type() == instruction_block_type_t::procedure)
            _procedure_block_count++;
    }

    void assembler::add_new_block(instruction_block* block) {
        auto source_file = _listing.current_source_file();
        if (source_file != nullptr)
            block->source_file(source_file);
        _blocks.push_back(block);
        auto top_block = current_block();
        if (top_block != nullptr)
            top_block->add_block(block);
    }

    vm::segment* assembler::segment(const std::string& name) {
        auto it = _segments.find(name);
        if (it == _segments.end())
            return nullptr;
        return &it->second;
    }

    void assembler::push_target_register(const register_t& reg) {
        _target_registers.push(reg);
    }

    block_entry_t* assembler::current_entry(instruction_block* block) {
        auto entry = block->current_entry();
        if (entry == nullptr) {
            block->memo();
            entry = block->current_entry();
        }
        return entry;
    }

    instruction_block* assembler::make_basic_block(instruction_block* parent_block) {
        auto block = new instruction_block(
            parent_block != nullptr ? parent_block : current_block(),
            instruction_block_type_t::basic);
        add_new_block(block);
        return block;
    }

    instruction_block* assembler::make_procedure_block(instruction_block* parent_block) {
        auto block = new instruction_block(
            parent_block != nullptr ? parent_block : current_block(),
            instruction_block_type_t::procedure);
        add_new_block(block);
        return block;
    }

};