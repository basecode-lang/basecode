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

#include "terp.h"
#include "assembler.h"
#include "basic_block.h"

namespace basecode::vm {

    basic_block::basic_block(basic_block_type_t type): _id(common::id_pool::instance()->allocate()),
                                                       _type(type) {
    }

    basic_block::~basic_block() {
        clear_entries();
    }

    void basic_block::rts() {
        instruction_t rts_op;
        rts_op.op = op_codes::rts;
        make_block_entry(rts_op);
    }

    void basic_block::dup() {
        instruction_t dup_op;
        dup_op.op = op_codes::dup;
        make_block_entry(dup_op);
    }

    void basic_block::nop() {
        instruction_t no_op;
        no_op.op = op_codes::nop;
        make_block_entry(no_op);
    }

    void basic_block::exit() {
        instruction_t exit_op;
        exit_op.op = op_codes::exit;
        make_block_entry(exit_op);
    }

    void basic_block::meta_end() {
        make_block_entry(meta_t {"end"});
    }

    void basic_block::meta_begin() {
        make_block_entry(meta_t {"begin"});
    }

    bool basic_block::apply_operand(
            const instruction_operand_t& operand,
            instruction_t& encoding,
            size_t operand_index) {
        if (operand.is_empty())
            return true;

        if (operand_index >= encoding.operands_count)
            return false;

        auto& op = encoding.operands[operand_index];
        switch (operand.type()) {
            case instruction_operand_type_t::reg: {
                auto reg = *operand.data<register_t>();
                op.size = reg.size;
                op.value.r = reg.number;
                op.type = operand_encoding_t::flags::reg;
                if (reg.type == register_type_t::integer)
                    op.type |= operand_encoding_t::flags::integer;
                break;
            }
            case instruction_operand_type_t::empty: {
                break;
            }
            case instruction_operand_type_t::imm_f32: {
                op.size = op_sizes::dword;
                op.value.f = *operand.data<float>();
                op.type = operand_encoding_t::flags::constant;
                break;
            }
            case instruction_operand_type_t::imm_f64: {
                op.size = op_sizes::qword;
                op.type = operand_encoding_t::flags::constant;
                op.value.d = *operand.data<double>();
                break;
            }
            case instruction_operand_type_t::named_ref: {
                op.type = operand_encoding_t::flags::none;
                op.value.u = 0;
                auto named_ref = operand.data<named_ref_with_offset_t>();
                op.fixup[0].named_ref = named_ref->ref;
                op.fixup[0].offset = named_ref->offset;

                switch (named_ref->ref->type) {
                    case assembler_named_ref_type_t::none: {
                        break;
                    }
                    case assembler_named_ref_type_t::local: {
                        op.size = operand.size();
                        op.type |= operand_encoding_t::flags::reg;
                        break;
                    }
                    case assembler_named_ref_type_t::label: {
                        op.size = op_sizes::qword;
                        op.type |= operand_encoding_t::flags::integer
                            | operand_encoding_t::flags::constant;
                        break;
                    }
                    case assembler_named_ref_type_t::offset: {
                        op.size = operand.size();
                        op.type |= operand_encoding_t::flags::integer
                            | operand_encoding_t::flags::constant;
                        break;
                    }
                }
                break;
            }
            case instruction_operand_type_t::imm_sint: {
                auto imm_value = *operand.data<int64_t>();
                op.size = operand.size();
                op.type = operand_encoding_t::flags::integer
                    | operand_encoding_t::flags::constant;
                if (imm_value < 0) {
                    op.type |= operand_encoding_t::flags::negative;
                    imm_value = -imm_value;
                }
                op.value.u = static_cast<uint64_t>(imm_value);
                break;
            }
            case instruction_operand_type_t::imm_uint: {
                auto imm_value = *operand.data<uint64_t>();
                op.size = operand.size();
                op.type = operand_encoding_t::flags::integer
                    | operand_encoding_t::flags::constant;
                op.value.u = imm_value;
                break;
            }
            case instruction_operand_type_t::reg_range: {
                auto range = *operand.data<register_range_t>();
                op.type = operand_encoding_t::flags::reg | operand_encoding_t::flags::range;
                if (range.begin.type == register_type_t::integer)
                    op.type |= operand_encoding_t::flags::integer;
                op.value.u = static_cast<uint64_t>(static_cast<uint16_t>(range.begin.number) << 8
                                                   | (static_cast<uint16_t>(range.end.number) & 0x00ff));
                break;
            }
            case instruction_operand_type_t::named_ref_range: {
                auto range = *operand.data<named_ref_range_t>();
                op.type = operand_encoding_t::flags::reg | operand_encoding_t::flags::range;
                op.fixup[0].named_ref = range.begin;
                op.fixup[1].named_ref = range.end;
                break;
            }
            default: {
                return false;
            }
        }

        return true;
    }

    common::id_t basic_block::id() const {
        return _id;
    }

    void basic_block::apply_insertion_point(
            ssize_t temp_point,
            const basic_block::insertion_point_scope_callback_t& callback) {
        const auto current_insertion_point = insertion_point();
        insertion_point(temp_point);
        callback(this);
        insertion_point(current_insertion_point);
    }

    // copy & fill
    void basic_block::copy(
            op_sizes inst_size,
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& size) {
        instruction_t op;
        op.size = inst_size;
        op.op = op_codes::copy;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(src, op, 1);
        apply_operand(size, op, 2);
        make_block_entry(op);
    }

    void basic_block::fill(
            op_sizes inst_size,
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& size) {
        instruction_t op;
        op.size = inst_size;
        op.op = op_codes::fill;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(src, op, 1);
        apply_operand(size, op, 2);
        make_block_entry(op);
    }

    // sections
    void basic_block::section(section_t type) {
        make_block_entry(type);
    }

    // data definitions
    void basic_block::align(uint8_t size) {
        make_block_entry(align_t {
            .size = size
        });
    }

    void basic_block::reserve_byte(uint32_t count) {
        make_block_entry(data_definition_t {
            .size = op_sizes::byte,
            .type = data_definition_type_t::uninitialized,
            .values = {count},
        });
    }

    void basic_block::reserve_word(uint32_t count) {
        make_block_entry(data_definition_t {
            .size = op_sizes::word,
            .type = data_definition_type_t::uninitialized,
            .values = {count},
        });
    }

    void basic_block::reserve_dword(uint32_t count) {
        make_block_entry(data_definition_t {
            .size = op_sizes::dword,
            .type = data_definition_type_t::uninitialized,
            .values = {count},
        });
    }

    void basic_block::reserve_qword(uint32_t count) {
        make_block_entry(data_definition_t {
            .size = op_sizes::qword,
            .type = data_definition_type_t::uninitialized,
            .values = {count},
        });
    }

    void basic_block::string(
            vm::label* start_label,
            vm::label* data_label,
            const std::string_view& value) {
        if (start_label != nullptr)
            label(start_label);
        dwords({static_cast<uint32_t>(value.length())});

        std::vector<uint8_t> str_bytes {};
        for (const auto& c : value)
            str_bytes.emplace_back(static_cast<uint8_t>(c));
        str_bytes.emplace_back(0);

        if (data_label != nullptr)
            label(data_label);
        bytes(str_bytes);
    }

    ssize_t basic_block::insertion_point() const {
        return _insertion_point != -1 ? _insertion_point : _entries.size();
    }

    uint8_t basic_block::pre_blank_lines() const {
        return _pre_blank_lines;
    }

    uint8_t basic_block::post_blank_lines() const {
        return _post_blank_lines;
    }

    void basic_block::insertion_point(ssize_t value) {
        _insertion_point = value;
    }

    void basic_block::reset(const std::string& type) {
        reset_t reset_directive {
            .type = type
        };
        make_block_entry(reset_directive);
    }

    void basic_block::pre_blank_lines(uint8_t value) {
        _pre_blank_lines = value;
    }

    void basic_block::post_blank_lines(uint8_t value) {
        _post_blank_lines = value;
    }

    void basic_block::bytes(const std::vector<uint8_t>& values) {
        data_definition_t def {
            .size = op_sizes::byte,
            .type = data_definition_type_t::initialized,
        };
        for (const auto& v : values)
            def.values.emplace_back(v);
        make_block_entry(def);
    }

    void basic_block::words(const std::vector<uint16_t>& values) {
        data_definition_t def {
            .size = op_sizes::word,
            .type = data_definition_type_t::initialized,
        };
        for (const auto& v : values)
            def.values.emplace_back(v);
        make_block_entry(def);
    }

    void basic_block::dwords(const std::vector<uint32_t>& values) {
        data_definition_t def {
            .size = op_sizes::dword,
            .type = data_definition_type_t::initialized,
        };
        for (const auto& v : values)
            def.values.emplace_back(v);
        make_block_entry(def);
    }

    void basic_block::values(const data_value_variant_list_t& list) {
        std::unordered_map<data_value_variant_type_t, data_definition_t> defs {};
        for (const auto& v : list) {
            auto type = static_cast<data_value_variant_type_t>(v.which());

            data_definition_t* def = nullptr;
            auto it = defs.find(type);
            if (it == std::end(defs)) {
                op_sizes op_size;
                switch (type) {
                    case data_value_variant_type_t::u8: op_size = op_sizes::byte; break;
                    case data_value_variant_type_t::u16:op_size = op_sizes::word; break;
                    case data_value_variant_type_t::f32:
                    case data_value_variant_type_t::u32:op_size = op_sizes::dword; break;
                    case data_value_variant_type_t::f64:
                    case data_value_variant_type_t::named_ref:
                    case data_value_variant_type_t::u64:op_size = op_sizes::qword; break;
                }
                auto result = defs.insert(std::make_pair(type, data_definition_t {
                    .size = op_size,
                    .type = data_definition_type_t::initialized,
                }));
                def = &result.first->second;
            } else {
                def = &it->second;
            }

            def->values.emplace_back(v);
        }

        for (const auto& kvp : defs)
            make_block_entry(kvp.second);
    }

    void basic_block::qwords(const data_value_variant_list_t& values) {
        data_definition_t def {
            .size = op_sizes::qword,
            .type = data_definition_type_t::initialized,
        };
        for (const auto& v : values)
            def.values.emplace_back(v);
        make_block_entry(def);
    }

    // load variations
    void basic_block::load(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::load;
        op.operands_count = static_cast<uint8_t>(offset.is_empty() ? 2 : 3);
        apply_operand(dest, op, 0);
        apply_operand(src, op, 1);
        apply_operand(offset, op, 2);
        make_block_entry(op);
    }

    // store variations
    void basic_block::store(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset) {
        instruction_t op;
        op.size = src.size();
        op.op = op_codes::store;
        op.operands_count = static_cast<uint8_t>(offset.is_empty() ? 2 : 3);
        apply_operand(dest, op, 0);
        apply_operand(src, op, 1);
        apply_operand(offset, op, 2);
        make_block_entry(op);
    }

    void basic_block::move(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset) {
        instruction_t op;
        op.size = src.size();
        op.op = op_codes::move;
        op.operands_count = static_cast<uint8_t>(offset.is_empty() ? 2 : 3);
        apply_operand(dest, op, 0);
        apply_operand(src, op, 1);
        apply_operand(offset, op, 2);
        make_block_entry(op);
    }

    void basic_block::moves(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::moves;
        op.operands_count = static_cast<uint8_t>(offset.is_empty() ? 2 : 3);
        apply_operand(dest, op, 0);
        apply_operand(src, op, 1);
        apply_operand(offset, op, 2);
        make_block_entry(op);
    }

    void basic_block::movez(
            const instruction_operand_t& dest,
            const instruction_operand_t& src,
            const instruction_operand_t& offset) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::movez;
        op.operands_count = static_cast<uint8_t>(offset.is_empty() ? 2 : 3);
        apply_operand(dest, op, 0);
        apply_operand(src, op, 1);
        apply_operand(offset, op, 2);
        make_block_entry(op);
    }

    void basic_block::clr(
            op_sizes size,
            const instruction_operand_t& dest) {
        instruction_t op;
        op.size = size;
        op.op = op_codes::clr;
        op.operands_count = 1;
        apply_operand(dest, op, 0);
        make_block_entry(op);
    }

    void basic_block::clr(const instruction_operand_t& dest) {
        clr(dest.size(), dest);
    }

    // not variations
    void basic_block::not_op(
            const instruction_operand_t& dest,
            const instruction_operand_t& src) {
        instruction_t op;
        op.size = src.size();
        op.op = op_codes::not_op;
        op.operands_count = 2;
        apply_operand(dest, op, 0);
        apply_operand(src, op, 1);
        make_block_entry(op);
    }

    // neg variations
    void basic_block::neg(
            const instruction_operand_t& dest,
            const instruction_operand_t& src) {
        instruction_t op;
        op.size = src.size();
        op.op = op_codes::neg;
        op.operands_count = 2;
        apply_operand(dest, op, 0);
        apply_operand(src, op, 1);
        make_block_entry(op);
    }

    // pow variations
    void basic_block::pow(
            const instruction_operand_t& dest,
            const instruction_operand_t& base,
            const instruction_operand_t& exponent) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::pow;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(base, op, 1);
        apply_operand(exponent, op, 2);
        make_block_entry(op);
    }

    // mul variations
    void basic_block::mul(
            const instruction_operand_t& dest,
            const instruction_operand_t& multiplicand,
            const instruction_operand_t& multiplier) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::mul;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(multiplicand, op, 1);
        apply_operand(multiplier, op, 2);
        make_block_entry(op);
    }

    // madd variations
    void basic_block::madd(
            const instruction_operand_t& dest,
            const instruction_operand_t& multiplicand,
            const instruction_operand_t& multiplier,
            const instruction_operand_t& addened) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::madd;
        op.operands_count = 4;
        apply_operand(dest, op, 0);
        apply_operand(multiplicand, op, 1);
        apply_operand(multiplier, op, 2);
        apply_operand(addened, op, 3);
        make_block_entry(op);
    }

    // add variations
    void basic_block::add(
            const instruction_operand_t& dest,
            const instruction_operand_t& augend,
            const instruction_operand_t& addened) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::add;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(augend, op, 1);
        apply_operand(addened, op, 2);
        make_block_entry(op);
    }

    // sub variations
    void basic_block::sub(
            const instruction_operand_t& dest,
            const instruction_operand_t& minuend,
            const instruction_operand_t& subtrahend) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::sub;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(minuend, op, 1);
        apply_operand(subtrahend, op, 2);
        make_block_entry(op);
    }

    // div variations
    void basic_block::div(
            const instruction_operand_t& dest,
            const instruction_operand_t& dividend,
            const instruction_operand_t& divisor) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::div;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(dividend, op, 1);
        apply_operand(divisor, op, 2);
        make_block_entry(op);
    }

    // mod variations
    void basic_block::mod(
            const instruction_operand_t& dest,
            const instruction_operand_t& dividend,
            const instruction_operand_t& divisor) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::mod;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(dividend, op, 1);
        apply_operand(divisor, op, 2);
        make_block_entry(op);
    }

    void basic_block::swi(uint8_t index) {
        instruction_t swi_op;
        swi_op.op = op_codes::swi;
        swi_op.size = op_sizes::byte;
        swi_op.operands_count = 1;
        swi_op.operands[0].type = operand_encoding_t::flags::integer;
        swi_op.operands[0].value.u = index;
        make_block_entry(swi_op);
    }

    void basic_block::trap(uint8_t index) {
        instruction_t trap_op;
        trap_op.op = op_codes::trap;
        trap_op.size = op_sizes::byte;
        trap_op.operands_count = 1;
        trap_op.operands[0].type = operand_encoding_t::flags::integer;
        trap_op.operands[0].value.u = index;
        make_block_entry(trap_op);
    }

    void basic_block::clear_entries() {
        _entries.clear();
        _recent_inst_index = -1;
    }

    // push variations
    void basic_block::push(const instruction_operand_t& operand) {
        instruction_t op;
        op.size = operand.size();
        op.operands_count = 1;
        op.op = op_codes::push;
        apply_operand(operand, op, 0);
        make_block_entry(op);
    }

    void basic_block::pushm(const instruction_operand_list_t& operands) {
        instruction_t op;
        op.op = op_codes::pushm;
        op.size = op_sizes::qword;
        op.operands_count = operands.size();

        size_t index = 0;
        for (const auto& operand : operands) {
            if (operand.is_empty()) {
                op.operands_count--;
                continue;
            }
            apply_operand(operand, op, index++);
        }

        if (op.operands_count == 0)
            return;

        make_block_entry(op);
    }

    // alloc/free
    void basic_block::alloc(
            op_sizes inst_size,
            const instruction_operand_t& dest,
            const instruction_operand_t& size) {
        instruction_t op;
        op.size = inst_size;
        op.operands_count = 2;
        op.op = op_codes::alloc;
        apply_operand(dest, op, 0);
        apply_operand(size, op, 1);
        make_block_entry(op);
    }

    void basic_block::free(const instruction_operand_t& addr) {
        instruction_t op;
        op.size = addr.size();
        op.operands_count = 1;
        op.op = op_codes::free;
        apply_operand(addr, op, 0);
        make_block_entry(op);
    }

    // convert
    void basic_block::convert(
            const instruction_operand_t& dest,
            const instruction_operand_t& src) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::convert;
        op.operands_count = 2;
        apply_operand(dest, op, 0);
        apply_operand(src, op, 1);
        make_block_entry(op);
    }

    // cmp variations
    void basic_block::cmp(
            op_sizes size,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs) {
        instruction_t op;
        op.size = size;
        op.op = op_codes::cmp;
        op.operands_count = 2;
        apply_operand(lhs, op, 0);
        apply_operand(rhs, op, 1);
        make_block_entry(op);
    }

    void basic_block::cmp(
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs) {
        instruction_t op;
        op.size = lhs.size();
        op.op = op_codes::cmp;
        op.operands_count = 2;
        apply_operand(lhs, op, 0);
        apply_operand(rhs, op, 1);
        make_block_entry(op);
    }

    // meta variations
    void basic_block::meta(
            const instruction_operand_t& type,
            const instruction_operand_t& size) {
        instruction_t op;
        op.operands_count = 2;
        op.op = op_codes::meta;
        op.size = op_sizes::word;
        apply_operand(type, op, 0);
        apply_operand(size, op, 1);
        make_block_entry(op);
    }

    // inc variations
    void basic_block::inc(const instruction_operand_t& target) {
        instruction_t op;
        op.operands_count = 1;
        op.op = op_codes::inc;
        op.size = target.size();
        apply_operand(target, op, 0);
        make_block_entry(op);
    }

    // dec variations
    void basic_block::dec(const instruction_operand_t& target) {
        instruction_t op;
        op.operands_count = 1;
        op.op = op_codes::dec;
        op.size = target.size();
        apply_operand(target, op, 0);
        make_block_entry(op);
    }

    // pop variations
    void basic_block::pop(const instruction_operand_t& dest) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::pop;
        op.operands_count = 1;
        apply_operand(dest, op, 0);
        make_block_entry(op);
    }

    void basic_block::popm(const instruction_operand_list_t& operands) {
        instruction_t op;
        op.op = op_codes::popm;
        op.size = op_sizes::qword;
        op.operands_count = operands.size();

        size_t index = 0;
        for (const auto& operand : operands) {
            if (operand.is_empty()) {
                op.operands_count--;
                continue;
            }
            apply_operand(operand, op, index++);
        }

        if (op.operands_count == 0)
            return;

        make_block_entry(op);
    }

    // setxx
    void basic_block::sets(const instruction_operand_t& dest) {
        make_set(op_codes::sets, dest.size(), dest);
    }

    void basic_block::setns(const instruction_operand_t& dest) {
        make_set(op_codes::setns, dest.size(), dest);
    }

    void basic_block::seto(const instruction_operand_t& dest) {
        make_set(op_codes::seto, dest.size(), dest);
    }

    void basic_block::setno(const instruction_operand_t& dest) {
        make_set(op_codes::setno, dest.size(), dest);
    }

    void basic_block::seta(const instruction_operand_t& dest) {
        make_set(op_codes::seta, dest.size(), dest);
    }

    void basic_block::setna(const instruction_operand_t& dest) {
        make_set(op_codes::setna, dest.size(), dest);
    }

    void basic_block::setae(const instruction_operand_t& dest) {
        make_set(op_codes::setae, dest.size(), dest);
    }

    void basic_block::setnae(const instruction_operand_t& dest) {
        make_set(op_codes::setnae, dest.size(), dest);
    }

    void basic_block::setb(const instruction_operand_t& dest) {
        make_set(op_codes::setb, dest.size(), dest);
    }

    void basic_block::setnb(const instruction_operand_t& dest) {
        make_set(op_codes::setnb, dest.size(), dest);
    }

    void basic_block::setbe(const instruction_operand_t& dest) {
        make_set(op_codes::setbe, dest.size(), dest);
    }

    void basic_block::setnbe(const instruction_operand_t& dest) {
        make_set(op_codes::setnbe, dest.size(), dest);
    }

    void basic_block::setc(const instruction_operand_t& dest) {
        make_set(op_codes::setc, dest.size(), dest);
    }

    void basic_block::setnc(const instruction_operand_t& dest) {
        make_set(op_codes::setnc, dest.size(), dest);
    }

    void basic_block::setg(const instruction_operand_t& dest) {
        make_set(op_codes::setg, dest.size(), dest);
    }

    void basic_block::setng(const instruction_operand_t& dest) {
        make_set(op_codes::setng, dest.size(), dest);
    }

    void basic_block::setge(const instruction_operand_t& dest) {
        make_set(op_codes::setge, dest.size(), dest);
    }

    void basic_block::setnge(const instruction_operand_t& dest) {
        make_set(op_codes::setnge, dest.size(), dest);
    }

    void basic_block::setl(const instruction_operand_t& dest) {
        make_set(op_codes::setl, dest.size(), dest);
    }

    void basic_block::setnl(const instruction_operand_t& dest) {
        make_set(op_codes::setnl, dest.size(), dest);
    }

    void basic_block::setle(const instruction_operand_t& dest) {
        make_set(op_codes::setle, dest.size(), dest);
    }

    void basic_block::setnle(const instruction_operand_t& dest) {
        make_set(op_codes::setnle, dest.size(), dest);
    }

    void basic_block::setz(const instruction_operand_t& dest) {
        make_set(op_codes::setz, dest.size(), dest);
    }

    void basic_block::setnz(const instruction_operand_t& dest) {
        make_set(op_codes::setnz, dest.size(), dest);
    }

    void basic_block::make_set(
            op_codes code,
            op_sizes size,
            const instruction_operand_t& dest) {
        instruction_t op;
        op.op = code;
        op.size = size;
        op.operands_count = 1;
        apply_operand(dest, op, 0);
        make_block_entry(op);
    }

    basic_block_type_t basic_block::type() const {
        return _type;
    }

    void basic_block::bb(const instruction_operand_t& dest) {
        make_branch(op_codes::bb, op_sizes::qword, dest);
    }

    void basic_block::ba(const instruction_operand_t& dest) {
        make_branch(op_codes::ba, op_sizes::qword, dest);
    }

    void basic_block::bs(const instruction_operand_t& dest) {
        make_branch(op_codes::bs, op_sizes::qword, dest);
    }

    void basic_block::bo(const instruction_operand_t& dest) {
        make_branch(op_codes::bo, op_sizes::qword, dest);
    }

    void basic_block::bg(const instruction_operand_t& dest) {
        make_branch(op_codes::bg, op_sizes::qword, dest);
    }

    void basic_block::bl(const instruction_operand_t& dest) {
        make_branch(op_codes::bl, op_sizes::qword, dest);
    }

    void basic_block::bge(const instruction_operand_t& dest) {
        make_branch(op_codes::bge, op_sizes::qword, dest);
    }

    void basic_block::ble(const instruction_operand_t& dest) {
        make_branch(op_codes::ble, op_sizes::qword, dest);
    }

    void basic_block::beq(const instruction_operand_t& dest) {
        make_branch(op_codes::beq, op_sizes::qword, dest);
    }

    void basic_block::bne(const instruction_operand_t& dest) {
        make_branch(op_codes::bne, op_sizes::qword, dest);
    }

    void basic_block::bcc(const instruction_operand_t& dest) {
        make_branch(op_codes::bcc, op_sizes::qword, dest);
    }

    void basic_block::bcs(const instruction_operand_t& dest) {
        make_branch(op_codes::bcs, op_sizes::qword, dest);
    }

    void basic_block::bbe(const instruction_operand_t& dest) {
        make_branch(op_codes::bbe, op_sizes::qword, dest);
    }

    void basic_block::bae(const instruction_operand_t& dest) {
        make_branch(op_codes::bae, op_sizes::qword, dest);
    }

    void basic_block::make_branch(
            op_codes code,
            op_sizes size,
            const instruction_operand_t& dest) {
        instruction_t op;
        op.op = code;
        op.size = size;
        op.operands_count = 1;
        apply_operand(dest, op, 0);
        make_block_entry(op);
    }

    // bz & bnz
    void basic_block::bz(
            const instruction_operand_t& src,
            const instruction_operand_t& dest) {
        instruction_t op;
        op.size = src.size();
        op.op = op_codes::bz;
        op.operands_count = 2;
        apply_operand(src, op, 0);
        apply_operand(dest, op, 1);
        make_block_entry(op);
    }

    void basic_block::bnz(
            const instruction_operand_t& src,
            const instruction_operand_t& dest) {
        instruction_t op;
        op.size = src.size();
        op.op = op_codes::bnz;
        op.operands_count = 2;
        apply_operand(src, op, 0);
        apply_operand(dest, op, 1);
        make_block_entry(op);
    }

    void basic_block::call(const instruction_operand_t& target) {
        instruction_t op;
        op.op = op_codes::jsr;
        op.size = op_sizes::qword;
        op.operands_count = 1;
        apply_operand(target, op, 0);
        make_block_entry(op);
    }

    void basic_block::call_foreign(
            const instruction_operand_t& address,
            const instruction_operand_t& signature_id) {
        instruction_t op;
        op.op = op_codes::ffi;
        op.size = op_sizes::qword;
        op.operands_count = static_cast<uint8_t>(signature_id.is_empty() ? 1 : 2);
        apply_operand(address, op, 0);
        apply_operand(signature_id, op, 1);
        make_block_entry(op);
    }

    void basic_block::jump_direct(const instruction_operand_t& target) {
        instruction_t op;
        op.op = op_codes::jmp;
        op.size = op_sizes::qword;
        op.operands_count = 1;
        apply_operand(target, op, 0);
        make_block_entry(op);
    }

    void basic_block::jump_indirect(const instruction_operand_t& target) {
        instruction_t op;
        op.operands_count = 1;
        op.op = op_codes::jmp;
        op.size = op_sizes::qword;
        apply_operand(target, op, 0);
        make_block_entry(op);
    }

    void basic_block::or_op(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::or_op;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(lhs, op, 1);
        apply_operand(rhs, op, 2);
        make_block_entry(op);
    }

    void basic_block::xor_op(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::xor_op;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(lhs, op, 1);
        apply_operand(rhs, op, 2);
        make_block_entry(op);
    }

    void basic_block::and_op(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::and_op;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(lhs, op, 1);
        apply_operand(rhs, op, 2);
        make_block_entry(op);
    }

    void basic_block::shl(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::shl;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(lhs, op, 1);
        apply_operand(rhs, op, 2);
        make_block_entry(op);
    }

    void basic_block::shr(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::shr;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(lhs, op, 1);
        apply_operand(rhs, op, 2);
        make_block_entry(op);
    }

    void basic_block::rol(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::rol;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(lhs, op, 1);
        apply_operand(rhs, op, 2);
        make_block_entry(op);
    }

    void basic_block::ror(
            const instruction_operand_t& dest,
            const instruction_operand_t& lhs,
            const instruction_operand_t& rhs) {
        instruction_t op;
        op.size = dest.size();
        op.op = op_codes::ror;
        op.operands_count = 3;
        apply_operand(dest, op, 0);
        apply_operand(lhs, op, 1);
        apply_operand(rhs, op, 2);
        make_block_entry(op);
    }

    void basic_block::local(
            local_type_t type,
            const std::string& name,
            int64_t offset,
            const std::string& frame_offset) {
        if (_locals.count(name) > 0)
            return;

        make_block_entry(local_t{
            .offset = offset,
            .type = type,
            .name = name,
            .frame_offset = frame_offset
        });
        _locals.insert(std::make_pair(name, _entries.size() - 1));
    }

    void basic_block::comment(
            const std::string& value,
            uint8_t indent,
            comment_location_t location) {
        comment_t comment {};
        comment.indent = indent;
        comment.value = value;
        comment.location = location;
        make_block_entry(comment);
    }

    void basic_block::comment(
            const std::string& value,
            comment_location_t location) {
        comment_t comment {};
        comment.value = value;
        comment.location = location;
        make_block_entry(comment);
    }

    void basic_block::program_end() {
        if (_insertion_point != -1) {
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t(program_end_t()));
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t(program_end_t()));
        }
    }

    void basic_block::blank_line() {
        if (_insertion_point != -1) {
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t());
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t());
        }
    }

    void basic_block::label(vm::label* value) {
        make_block_entry(label_t {
            .instance = value
        });
    }

    block_entry_list_t& basic_block::entries() {
        return _entries;
    }

    vm::basic_block_list_t& basic_block::successors() {
        return _successors;
    }

    vm::basic_block_list_t& basic_block::predecessors() {
        return _predecessors;
    }

    void basic_block::copy_locals(vm::basic_block* block) {
        for (const auto& kvp : block->_locals) {
            auto l = block->local(kvp.first);
            local(l->type, l->name, l->offset, l->frame_offset);
        }
    }

    void basic_block::make_block_entry(const meta_t& meta) {
        if (_insertion_point != -1) {
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t(meta));
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t(meta));
        }
    }

    bool basic_block::is_current_instruction(op_codes code) {
        if (_entries.empty() || _recent_inst_index == -1)
            return false;

        auto& current_entry = _entries[_recent_inst_index];
        if (current_entry.type() == block_entry_type_t::instruction) {
            auto inst = current_entry.data<instruction_t>();
            return inst != nullptr && inst->op == code;
        }

        return false;
    }

    void basic_block::make_block_entry(const local_t& local) {
        if (_insertion_point != -1) {
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t(local));
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t(local));
        }
    }

    void basic_block::make_block_entry(const label_t& label) {
        if (_insertion_point != -1) {
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t(label));
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t(label));
        }
    }

    void basic_block::make_block_entry(const align_t& align) {
        if (_insertion_point != -1) {
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t(align));
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t(align));
        }
    }

    void basic_block::make_block_entry(const reset_t& reset) {
        if (_insertion_point != -1) {
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t(reset));
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t(reset));
        }
    }

    bool basic_block::has_local(const std::string& name) const {
        return _locals.count(name) > 0;
    }

    void basic_block::make_block_entry(const comment_t& comment) {
        if (_insertion_point != -1) {
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t(comment));
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t(comment));
        }
    }

    void basic_block::make_block_entry(const section_t& section) {
        if (_insertion_point != -1) {
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t(section));
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t(section));
        }
    }

    void basic_block::make_block_entry(const instruction_t& inst) {
        if (_insertion_point != -1) {
            _recent_inst_index = _insertion_point;
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t(inst));
            _insertion_point++;
        }
        else {
            _recent_inst_index = static_cast<int64_t>(_entries.size());
            _entries.push_back(block_entry_t(inst));
        }
    }

    void basic_block::make_block_entry(const data_definition_t& data) {
        if (_insertion_point != -1) {
            _entries.insert(std::begin(_entries) + _insertion_point, block_entry_t(data));
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t(data));
        }
    }

    const vm::local_t* basic_block::local(const std::string& name) const {
        auto it = _locals.find(name);
        if (it == _locals.end())
            return nullptr;
        return _entries[it->second].data<local_t>();
    }

    void basic_block::add_successors(const vm::basic_block_list_t& list) {
        for (auto x : list)
            _successors.emplace_back(x);
    }

    void basic_block::add_predecessors(const vm::basic_block_list_t& list) {
        for (auto x : list)
            _predecessors.emplace_back(x);
    }

    void basic_block::frame_offset(const std::string& name, int64_t offset) {
        if (_insertion_point != -1) {
            _entries.insert(
                std::begin(_entries) + _insertion_point,
                block_entry_t(frame_offset_t{offset, name}));
            _insertion_point++;
        } else {
            _entries.push_back(block_entry_t(frame_offset_t{offset, name}));
        }
    }

}