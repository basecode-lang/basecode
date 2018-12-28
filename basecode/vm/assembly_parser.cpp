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
#include <common/defer.h>
#include <common/string_support.h>
#include "assembler.h"
#include "assembly_parser.h"
#include "instruction_block.h"

namespace basecode::vm {

    assembly_parser::assembly_parser(
            vm::assembler* assembler,
            common::source_file& source_file,
            const assembly_symbol_resolver_t& resolver) : _source_file(source_file),
                                                          _assembler(assembler),
                                                          _resolver(resolver) {
    }

    assembly_parser::~assembly_parser() {
        for (const auto& kvp : _locals) {
            _assembler->free_reg(kvp.second);
        }
    }

    bool assembly_parser::parse(common::result& r) {
        auto block = _assembler->current_block();

        _state = assembly_parser_state_t::start;
        _start_pos = 0;

        while (true) {
            auto rune = _source_file.next(r);
            if (rune == common::rune_eof)
                break;


        retry:
            switch (_state) {
                case assembly_parser_state_t::start: {
                    _state = assembly_parser_state_t::whitespace;
                    goto retry;
                }
                case assembly_parser_state_t::whitespace: {
                    if (!isspace(rune)) {
                        _start_pos = _source_file.pos();
                        if (rune == ';') {
                            _state = assembly_parser_state_t::comment;
                        } else if (rune == '.') {
                            _state = assembly_parser_state_t::directive;
                        } else if (isalpha(rune) || rune == '_') {
                            _source_file.push_mark();

                            _state = assembly_parser_state_t::mnemonic;

                            while (true) {
                                auto next_rune = _source_file.next(r);
                                if (next_rune == ':') {
                                    auto pos = _source_file.pos();
                                    next_rune = _source_file.next(r);
                                    if (next_rune != ':') {
                                        _source_file.seek(pos);
                                        _state = assembly_parser_state_t::label;
                                    }
                                    break;
                                } else if (next_rune == '\n') {
                                    break;
                                }
                            }

                            _source_file.seek(_source_file.pop_mark());
                            goto retry;
                        }
                    }
                    break;
                }
                case assembly_parser_state_t::comment: {
                    std::stringstream stream;
                    while (true) {
                        stream << static_cast<char>(rune);
                        rune = _source_file.next(r);
                        if (rune == '\n')
                            break;
                    }
                    block->comment(stream.str(), 4);

                    if (_wip.is_valid) {
                        if (_wip.type == wip_type_t::mnemonic)
                            _state = assembly_parser_state_t::encode_instruction;
                        else
                            _state = assembly_parser_state_t::encode_directive;
                    }
                    else
                        _state = assembly_parser_state_t::whitespace;
                    break;
                }
                case assembly_parser_state_t::label: {
                    std::stringstream stream;
                    while (true) {
                        stream << static_cast<char>(rune);
                        rune = _source_file.next(r);
                        if (rune == ':')
                            break;
                    }
                    block->label(_assembler->make_label(stream.str()));
                    _state = assembly_parser_state_t::whitespace;
                    break;
                }
                case assembly_parser_state_t::mnemonic: {
                    size_t end_pos = 0;
                    std::stringstream stream;
                    while (true) {
                        stream << static_cast<char>(rune);
                        rune = _source_file.next(r);
                        if (!isalpha(rune)) {
                            _wip.code = stream.str();
                            _wip.type = wip_type_t::mnemonic;
                            end_pos = _source_file.pos();

                            if (rune == '.') {
                                rune = _source_file.next(r);
                                switch (rune) {
                                    case 'b': {
                                        _wip.size = op_sizes::byte;
                                        break;
                                    }
                                    case 'w': {
                                        _wip.size = op_sizes::word;
                                        break;
                                    }
                                    case 'd': {
                                        rune = _source_file.next(r);
                                        if (rune == 'w') {
                                            _wip.size = op_sizes::dword;
                                            break;
                                        }
                                        break;
                                    }
                                    case 'q': {
                                        rune = _source_file.next(r);
                                        if (rune == 'w') {
                                            _wip.size = op_sizes::qword;
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

                    common::to_upper(_wip.code);
                    _wip.instance.m = mnemonic(_wip.code);
                    if (_wip.instance.m == nullptr) {
                        _source_file.error(
                            r,
                            "A003",
                            "unknown mnemonic.",
                            make_location(end_pos));
                        return false;
                    }

                    if (_wip.instance.m->operands.size() == 0) {
                        _wip.is_valid = true;
                        _state = assembly_parser_state_t::encode_instruction;
                    }
                    else
                        _state = assembly_parser_state_t::operand_list;
                    break;
                }
                case assembly_parser_state_t::operand_list: {
                    auto required_operand_count = _wip.instance.m->required_operand_count();
                    size_t commas_found = 0;
                    _source_file.push_mark();
                    auto test_rune = rune;
                    while (test_rune != '\n' && test_rune != ';') {
                        if (test_rune == ',')
                            commas_found++;
                        test_rune = _source_file.next(r);
                    }

                    if (commas_found < required_operand_count - 1) {
                        _source_file.error(
                            r,
                            "A004",
                            fmt::format(
                                "mnemonic '{}' requires '{}' operands.",
                                _wip.code,
                                required_operand_count),
                            make_location(_source_file.pos()));
                        return false;
                    }

                    _source_file.seek(_source_file.pop_mark());

                    std::vector<std::string> operands {};
                    parse_comma_separated_tokens(r, rune, operands);

                    for (const auto& operand : operands) {
                        operand_encoding_t encoding;

                        if (operand[0] == '#') {
                            uint64_t value;
                            if (!parse_immediate_number(r, operand, value))
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
                            auto number = std::atoi(operand.substr(1).c_str());
                            encoding.type = operand_encoding_t::flags::reg;
                            encoding.value.r = static_cast<uint8_t>(number);
                        } else if ((operand[0] == 'F' || operand[0] == 'f')
                                && (operand[1] == 'P' || operand[1] == 'p')) {
                            encoding.size = op_sizes::qword;
                            encoding.type = operand_encoding_t::flags::reg
                                | operand_encoding_t::flags::integer;
                            encoding.value.r = static_cast<uint8_t>(registers_t::fp);
                        } else if ((operand[0] == 'S' || operand[0] == 's')
                                   && (operand[1] == 'P' || operand[1] == 'p')) {
                            encoding.size = op_sizes::qword;
                            encoding.type = operand_encoding_t::flags::reg
                                | operand_encoding_t::flags::integer;
                            encoding.value.r = static_cast<uint8_t>(registers_t::sp);
                        } else if ((operand[0] == 'P' || operand[0] == 'p')
                                   && (operand[1] == 'C' || operand[1] == 'c')) {
                            encoding.size = op_sizes::qword;
                            encoding.type = operand_encoding_t::flags::reg
                                | operand_encoding_t::flags::integer;
                            encoding.value.r = static_cast<uint8_t>(registers_t::pc);
                        } else {
                            std::string symbol;
                            auto type = symbol_type(operand, symbol);
                            switch (type) {
                                case vm::assembly_symbol_type_t::label: {
                                    vm::assembly_symbol_result_t resolver_result {};
                                    if (_resolver(type, symbol, resolver_result)) {
                                        auto label_data = resolver_result.data<compiler_label_data_t>();
                                        if (label_data != nullptr) {
                                            encoding.type = operand_encoding_t::flags::integer
                                                            | operand_encoding_t::flags::constant
                                                            | operand_encoding_t::flags::unresolved;
                                            auto label_ref = _assembler->make_label_ref(label_data->label);
                                            encoding.value.u = label_ref->id;
                                        }
                                    }
                                    break;
                                }
                                case vm::assembly_symbol_type_t::module: {
                                    vm::assembly_symbol_result_t resolver_result {};
                                    if (_resolver(type, symbol, resolver_result)) {
                                        auto module_data = resolver_result.data<compiler_module_data_t>();
                                        if (module_data != nullptr) {
                                            switch (module_data->type()) {
                                                case vm::compiler_module_data_type_t::reg: {
                                                    auto reg_data = module_data->data<vm::register_t>();
                                                    encoding.size = reg_data->size;
                                                    encoding.type = operand_encoding_t::flags::reg;
                                                    if (reg_data->type == vm::register_type_t::integer)
                                                        encoding.type |= operand_encoding_t::flags::integer;
                                                    encoding.value.r = static_cast<uint8_t>(reg_data->number);
                                                    break;
                                                }
                                                case vm::compiler_module_data_type_t::label: {
                                                    auto label = module_data->data<std::string>();
                                                    encoding.type = operand_encoding_t::flags::integer
                                                                    | operand_encoding_t::flags::constant
                                                                    | operand_encoding_t::flags::unresolved;
                                                    auto label_ref = _assembler->make_label_ref(*label);
                                                    encoding.value.u = label_ref->id;
                                                    break;
                                                }
                                                case vm::compiler_module_data_type_t::imm_f32: {
                                                    auto value = module_data->data<float>();
                                                    encoding.type = operand_encoding_t::flags::constant;
                                                    encoding.value.f = *value;
                                                    break;
                                                }
                                                case vm::compiler_module_data_type_t::imm_f64: {
                                                    auto value = module_data->data<double>();
                                                    encoding.type = operand_encoding_t::flags::constant;
                                                    encoding.value.d = *value;
                                                    break;
                                                }
                                                case vm::compiler_module_data_type_t::imm_integer: {
                                                    auto value = module_data->data<uint64_t>();
                                                    encoding.type = operand_encoding_t::flags::integer
                                                                    | operand_encoding_t::flags::constant;
                                                    encoding.value.u = *value;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                                case vm::assembly_symbol_type_t::local: {
                                    vm::assembly_symbol_result_t resolver_result {};
                                    if (_resolver(type, symbol, resolver_result)) {
                                        auto local_data = resolver_result.data<compiler_local_data_t>();
                                        if (local_data != nullptr) {
                                            auto offset = local_data->offset;
                                            encoding.size = op_sizes::word;
                                            encoding.type = operand_encoding_t::flags::integer
                                                            | operand_encoding_t::flags::constant;
                                            if (local_data->offset < 0) {
                                                encoding.type |= operand_encoding_t::flags::negative;
                                                offset = -offset;
                                            }
                                            encoding.value.u = static_cast<uint64_t>(offset);
                                        }
                                    }
                                    break;
                                }
                                case vm::assembly_symbol_type_t::assembler: {
                                    auto it = _locals.find(operand);
                                    if (it != _locals.end()) {
                                        encoding.size = it->second.size;
                                        encoding.type = operand_encoding_t::flags::reg;
                                        if (it->second.type == vm::register_type_t::integer)
                                            encoding.type |= operand_encoding_t::flags::integer;
                                        encoding.value.r = static_cast<uint8_t>(it->second.number);
                                    } else {
                                        encoding.type = operand_encoding_t::flags::integer
                                                        | operand_encoding_t::flags::constant
                                                        | operand_encoding_t::flags::unresolved;
                                        auto label_ref = _assembler->make_label_ref(operand);
                                        encoding.value.u = label_ref->id;
                                        break;
                                    }
                                }
                            }
                        }

                        _wip.operands.push_back(encoding);
                    }

                    _wip.is_valid = _wip.operands.size() >= required_operand_count;
                    goto retry;
                }
                case assembly_parser_state_t::encode_instruction: {
                    if (!_wip.is_valid) {
                        _source_file.error(
                            r,
                            "A005",
                            "invalid instruction encoding.",
                            make_location(_source_file.pos()));
                        return false;
                    }

                    instruction_t inst;
                    inst.size = _wip.size;
                    inst.op = _wip.instance.m->code;
                    inst.operands_count = static_cast<uint8_t>(_wip.operands.size());

                    size_t index = 0;
                    for (const auto& operand : _wip.operands)
                        inst.operands[index++] = operand;

                    block->make_block_entry(inst);

                    _wip.reset();
                    _state = assembly_parser_state_t::whitespace;
                    break;
                }
                case assembly_parser_state_t::encode_directive: {
                    if (!_wip.is_valid) {
                        _source_file.error(
                            r,
                            "A005",
                            "invalid directive encoding.",
                            make_location(_source_file.pos()));
                        return false;
                    }

                    data_definition_t data_def {};

                    switch (_wip.instance.d->type) {
                        case directive_type_t::meta: {
                            block->make_block_entry(meta_t{boost::get<std::string>(_wip.params.front())});
                            break;
                        }
                        case directive_type_t::ilocal: {
                            auto symbol = boost::get<std::string>(_wip.params.front());
                            auto it = _locals.find(symbol);
                            if (it == _locals.end()) {
                                vm::register_t local;
                                local.size = vm::op_sizes::qword;
                                local.type = vm::register_type_t::integer;
                                if (_assembler->allocate_reg(local)) {
                                    _locals.insert(std::make_pair(symbol, local));
                                }
                            }
                            break;
                        }
                        case directive_type_t::flocal: {
                            auto symbol = boost::get<std::string>(_wip.params.front());
                            auto it = _locals.find(symbol);
                            if (it == _locals.end()) {
                                vm::register_t local;
                                local.size = vm::op_sizes::qword;
                                local.type = vm::register_type_t::floating_point;
                                if (_assembler->allocate_reg(local)) {
                                    _locals.insert(std::make_pair(symbol, local));
                                }
                            }
                            break;
                        }
                        case directive_type_t::section: {
                            block->make_block_entry(section_type(boost::get<std::string>(
                                _wip.params.front())));
                            break;
                        }
                        case directive_type_t::align: {
                            block->make_block_entry(align_t{
                                .size = static_cast<uint8_t>(boost::get<uint64_t>(_wip.params.front()))
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
                        for (const auto& data : _wip.params)
                            data_def.values.emplace_back(boost::get<uint64_t>(data));
                        block->make_block_entry(data_def);
                    }

                    _wip.reset();
                    _state = assembly_parser_state_t::whitespace;
                    break;
                }
                case assembly_parser_state_t::directive: {
                    std::stringstream stream;
                    size_t end_pos = 0;
                    while (true) {
                        stream << static_cast<char>(rune);
                        rune = _source_file.next(r);
                        if (!isalpha(rune)) {
                            end_pos = _source_file.pos() - 2;
                            _wip.code = stream.str();
                            _wip.type = wip_type_t::directive;
                            break;
                        }
                    }

                    common::to_upper(_wip.code);
                    _wip.instance.d = directive(_wip.code);
                    if (_wip.instance.d == nullptr) {
                        _source_file.error(
                            r,
                            "A003",
                            "unknown directive.",
                            make_location(end_pos));
                        return false;
                    }

                    if (_wip.instance.d->params.size() == 0)
                        _state = assembly_parser_state_t::whitespace;
                    else
                        _state = assembly_parser_state_t::directive_param_list;
                    break;
                }
                case assembly_parser_state_t::directive_param_list: {
                    auto required_params_count = _wip.instance.d->required_operand_count();

                    std::vector<std::string> params {};
                    parse_comma_separated_tokens(r, rune, params);

                    // XXX: must validate the size for numbers
                    if (_wip.instance.d->params[0].is_repeating()) {
                        for (const auto& param : params) {
                            uint64_t value;
                            if (!parse_immediate_number(r, param, value))
                                return false;
                            _wip.params.push_back(value);
                        }
                    } else {
                        size_t param_index = 0;
                        for (const auto& param_def : _wip.instance.d->params) {
                            if (param_index >= params.size())
                                break;
                            auto param = params[param_index++];
                            if (param_def.is_number()) {
                                uint64_t value;
                                if (!parse_immediate_number(r, param, value))
                                    return false;
                                _wip.params.push_back(value);
                            } else if (param_def.is_string()) {
                                if (param[0] == '\'') {
                                    size_t param_char_idx = 1;
                                    std::stringstream stream;
                                    while (true) {
                                        if (param_char_idx >= param.length()) {
                                            _source_file.error(
                                                r,
                                                "A005",
                                                "invalid string param value, did you forget the closing '?",
                                                make_location(_source_file.pos()));
                                            return false;
                                        }
                                        auto token = param[param_char_idx++];
                                        if (token == '\'')
                                            break;
                                        stream << token;
                                    }
                                    _wip.params.push_back(stream.str());
                                } else {
                                    _source_file.error(
                                        r,
                                        "A005",
                                        "invalid string param value, did you forget the leading '?",
                                        make_location(_source_file.pos()));
                                    return false;
                                }
                            } else if (param_def.is_symbol()) {
                                _wip.params.push_back(param);
                            } else {
                                _source_file.error(
                                    r,
                                    "A005",
                                    "unknown param type.",
                                    make_location(_source_file.pos()));
                                return false;
                            }
                        }
                    }

                    _wip.is_valid = _wip.params.size() >= required_params_count;
                    goto retry;
                }
            }
        }

        return true;
    }

    bool assembly_parser::parse_immediate_number(
            common::result& r,
            const std::string& param,
            uint64_t& value) {
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
                _source_file.error(
                    r,
                    "A005",
                    "invalid numeric param value, did you forget the #?",
                    make_location(_source_file.pos()));
                return false;
            }
        } else {
            _source_file.error(
                r,
                "A005",
                "invalid numeric param value, did you forget the #?",
                make_location(_source_file.pos()));
            return false;
        }

        return true;
    }

    void assembly_parser::parse_comma_separated_tokens(
            common::result& r,
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
                    _state = assembly_parser_state_t::comment;
                    break;
                } else {
                    operand_stream << static_cast<char>(rune);
                }
            }
            if (rune == '\n') {
                add_operand_to_list();
                if (_wip.type == wip_type_t::mnemonic)
                    _state = assembly_parser_state_t::encode_instruction;
                else
                    _state = assembly_parser_state_t::encode_directive;
                break;
            }
            rune = _source_file.next(r);
        }
    }

    vm::assembly_symbol_type_t assembly_parser::symbol_type(
            const std::string& operand,
            std::string& symbol) {
        auto type = vm::assembly_symbol_type_t::assembler;

        auto pos = operand.find("local(");
        if (pos != std::string::npos) {
            type = vm::assembly_symbol_type_t::local;
            symbol = operand.substr(6, operand.size() - 7);
        } else {
            pos = operand.find("module(");
            if (pos != std::string::npos) {
                type = vm::assembly_symbol_type_t::module;
                symbol = operand.substr(7, operand.size() - 8);
            } else {
                pos = operand.find("label(");
                if (pos != std::string::npos) {
                    type = vm::assembly_symbol_type_t::label;
                    symbol = operand.substr(6, operand.size() - 7);
                } else {
                    symbol = operand;
                }
            }
        }

        return type;
    }

    common::source_location assembly_parser::make_location(size_t end_pos) {
        auto start_line = _source_file.line_by_index(_start_pos);
        auto start_column = _source_file.column_by_index(_start_pos);
        auto end_line = _source_file.line_by_index(end_pos);
        auto end_column = _source_file.column_by_index(end_pos);

        common::source_location location;
        location.start(start_line->line, start_column);
        location.end(end_line->line, end_column);
        return location;
    }

    bool assembly_parser::is_float_register(const std::string& value) const {
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

    bool assembly_parser::is_integer_register(const std::string& value) const {
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