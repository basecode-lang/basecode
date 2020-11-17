// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include "types.h"

namespace basecode::dsl::ast {
    using namespace basecode::string;

    static string::slice_t s_node_kinds[] = {
        [(u32) node_kind_t::none]        = "none"_ss,
        [(u32) node_kind_t::field]       = "field"_ss,
        [(u32) node_kind_t::header]      = "header"_ss,
    };

    static string::slice_t s_node_field_types[] = {
        [(u32) node_field_type_t::none]     = "none"_ss,
        [(u32) node_field_type_t::id]       = "id"_ss,
        [(u32) node_field_type_t::lhs]      = "lhs"_ss,
        [(u32) node_field_type_t::rhs]      = "rhs"_ss,
        [(u32) node_field_type_t::arg]      = "arg"_ss,
        [(u32) node_field_type_t::flags]    = "flags"_ss,
        [(u32) node_field_type_t::token]    = "token"_ss,
        [(u32) node_field_type_t::scope]    = "scope"_ss,
        [(u32) node_field_type_t::radix]    = "radix"_ss,
        [(u32) node_field_type_t::child]    = "child"_ss,
        [(u32) node_field_type_t::parent]   = "parent"_ss,
    };

    static string::slice_t s_node_header_types[] = {
        [(u32) node_header_type_t::none]        = "none"_ss,
        [(u32) node_header_type_t::call]        = "call"_ss,
        [(u32) node_header_type_t::read]        = "read"_ss,
        [(u32) node_header_type_t::write]       = "write"_ss,
        [(u32) node_header_type_t::ident]       = "ident"_ss,
        [(u32) node_header_type_t::unary]       = "unary"_ss,
        [(u32) node_header_type_t::scope]       = "scope"_ss,
        [(u32) node_header_type_t::binary]      = "binary"_ss,
        [(u32) node_header_type_t::signal]      = "signal"_ss,
        [(u32) node_header_type_t::comment]     = "comment"_ss,
        [(u32) node_header_type_t::str_lit]     = "str_lit"_ss,
        [(u32) node_header_type_t::num_lit]     = "num_lit"_ss,
        [(u32) node_header_type_t::assignment]  = "assignment"_ss,
    };

    static string::slice_t s_unary_ops[] = {
        [(u32) unary_op_type_t::imm]        = "imm"_ss,
        [(u32) unary_op_type_t::neg]        = "neg"_ss,
        [(u32) unary_op_type_t::pos]        = "pos"_ss,
        [(u32) unary_op_type_t::lnot]       = "lnot"_ss,
        [(u32) unary_op_type_t::bnot]       = "bnot"_ss,
        [(u32) unary_op_type_t::pre_inc]    = "pre_inc"_ss,
        [(u32) unary_op_type_t::pre_dec]    = "pre_dec"_ss,
        [(u32) unary_op_type_t::post_inc]   = "post_inc"_ss,
        [(u32) unary_op_type_t::post_dec]   = "post_dec"_ss,
    };

    static string::slice_t s_binary_ops[] = {
        [(u32) binary_op_type_t::lt]        = "lt"_ss,
        [(u32) binary_op_type_t::gt]        = "gt"_ss,
        [(u32) binary_op_type_t::eq]        = "eq"_ss,
        [(u32) binary_op_type_t::lte]       = "lte"_ss,
        [(u32) binary_op_type_t::gte]       = "gte"_ss,
        [(u32) binary_op_type_t::neq]       = "neq"_ss,
        [(u32) binary_op_type_t::lor]       = "lor"_ss,
        [(u32) binary_op_type_t::shl]       = "shl"_ss,
        [(u32) binary_op_type_t::shr]       = "shr"_ss,
        [(u32) binary_op_type_t::rol]       = "rol"_ss,
        [(u32) binary_op_type_t::ror]       = "ror"_ss,
        [(u32) binary_op_type_t::add]       = "add"_ss,
        [(u32) binary_op_type_t::sub]       = "sub"_ss,
        [(u32) binary_op_type_t::mul]       = "mul"_ss,
        [(u32) binary_op_type_t::div]       = "div"_ss,
        [(u32) binary_op_type_t::mod]       = "mod"_ss,
        [(u32) binary_op_type_t::bor]       = "bor"_ss,
        [(u32) binary_op_type_t::dot]       = "dot"_ss,
        [(u32) binary_op_type_t::band]      = "band"_ss,
        [(u32) binary_op_type_t::bxor]      = "bxor"_ss,
        [(u32) binary_op_type_t::land]      = "land"_ss,
        [(u32) binary_op_type_t::range]     = "range"_ss,
    };

    static u32 page_size() {
        return (memory::os_page_size() * 64) - 16;
    }

    u0 init(
            storage_t& storage,
            memory::allocator_t* allocator) {
        storage.offset = {};
        storage.allocator = allocator;
        storage.head = storage.tail = {};
        array::init(storage.index, allocator);
        make_page(storage);
    }

    b8 write_field(
            node_cursor_t& cursor,
            node_field_type_t type,
            u32 value) {
        cursor.field = (node_field_t*) cursor.page + cursor.offset;
        cursor.field->value = value;
        cursor.field->type = (u8) type;
        cursor.field->kind = (u8) node_kind_t::field;
        cursor.ok = move_next(cursor);
        return cursor.ok;
    }

    node_cursor_t write_header(
            storage_t& storage,
            node_header_type_t type,
            u32 size) {
        auto node_size = sizeof(node_header_t) + (size + 2) * sizeof(node_field_t);
        if (storage.offset + node_size > page_size())
            make_page(storage);

        auto result = make_cursor(storage);
        result.field = {};
        result.id = result.storage->id++;
        result.header = (node_header_t*) result.page + result.offset;
        result.header->type = (u8) type;
        result.header->size = node_size;
        result.header->kind = (u8) node_kind_t::header;

        if (storage.index.size + 1 > storage.index.capacity)
            array::grow(storage.index);
        auto& index = storage.index[result.id];
        index.page = result.page;
        index.offset = result.offset;
        ++storage.index.size;

        result.end_offset = result.offset + (size + 2) * sizeof(node_field_t);
        result.offset += sizeof(node_header_t);
        storage.offset += result.header->size;

        result.ok = write_field(result, node_field_type_t::id, result.id);
        return result;
    }

    u0 free(storage_t& storage) {
        array::free(storage.index);
        auto curr_page = (u0*) storage.head;
        while (curr_page) {
            auto prev_page = ((page_header_t*) curr_page)->prev;
            memory::deallocate(storage.allocator, curr_page);
            curr_page = prev_page;
        }
        storage.offset = {};
        storage.head = storage.tail = {};
    }

    u8* make_page(storage_t& storage) {
        auto page = (u8*) memory::allocate(storage.allocator, page_size());
        if (!storage.tail)
            storage.tail = page;
        auto page_header = (page_header_t*) page;
        if (storage.head) {
            page_header->prev = storage.head;
            page_header->next = page;
        } else {
            page_header->prev = page_header->next = {};
        }
        storage.head = page;
        storage.offset = sizeof(u8*) * 2;
        return page;
    }

    b8 move_next(node_cursor_t& cursor) {
        if (!cursor.header)
            return false;
        if (cursor.offset < cursor.end_offset) {
            cursor.offset += sizeof(node_field_t);
            return true;
        }
        return false;
    }

    b8 next_field(node_cursor_t& cursor) {
        if (!move_next(cursor))
            return false;
        cursor.field = (node_field_t*) cursor.page + cursor.offset;
        cursor.ok = cursor.field != nullptr
            && cursor.field->kind == (u8) node_kind_t::field;
        return cursor.ok;
    }

    b8 next_header(node_cursor_t& cursor) {
        if (cursor.offset + sizeof(node_header_t) >= page_size()) {
            u8* next_page = (u8*) *((u64*) cursor.page + sizeof(u64));
            if (!next_page) {
                cursor.ok = false;
                return false;
            }
            cursor.page = next_page;
            cursor.offset = sizeof(u8*) * 2;
        } else {
            cursor.offset += cursor.end_offset - cursor.offset;
        }
        cursor.field = {};
        cursor.header = (node_header_t*) cursor.page + cursor.offset;
        cursor.end_offset = cursor.offset + cursor.header->size;
        cursor.ok = cursor.header != nullptr
            && cursor.header->kind == (u8) node_kind_t::header;
        return cursor.ok;
    }

    node_cursor_t make_cursor(storage_t& storage) {
        node_cursor_t cursor{};
        cursor.id = {};
        cursor.ok = true;
        cursor.field = {};
        cursor.header = {};
        cursor.storage = &storage;
        cursor.page = storage.head;
        cursor.offset = storage.offset;
        return cursor;
    }

    node_cursor_t first_header(storage_t& storage) {
        node_cursor_t cursor{};
        cursor.id = {};
        cursor.ok = true;
        cursor.field = {};
        cursor.storage = &storage;
        cursor.page = storage.head;
        cursor.offset = sizeof(u8*) * 2;
        cursor.header = (node_header_t*) cursor.page + cursor.offset;
        cursor.end_offset = cursor.offset + cursor.header->size;
        return cursor;
    }

    storage_t make(memory::allocator_t* allocator) {
        storage_t storage{};
        init(storage, allocator);
        return storage;
    }

    string::slice_t node_kind_name(node_kind_t kind) {
        return s_node_kinds[(u32) kind];
    }

    string::slice_t unary_op_name(unary_op_type_t type) {
        return s_unary_ops[(u32) type];
    }

    node_cursor_t get_header(storage_t& storage, u32 id) {
        auto result = make_cursor(storage);
        if (id > 0 && id < storage.id) {
            const auto& index = storage.index[id - 1];
            result.page = index.page;
            result.offset = index.offset;
            result.header = (node_header_t*) result.page + result.offset;
            result.end_offset = result.offset + result.header->size;
        }
        result.ok = result.header != nullptr
            && result.header->kind == (u8) node_kind_t::header;
        return result;
    }

    node_cursor_t make_cursor(const node_cursor_t& other) {
        node_cursor_t cursor{};
        cursor.ok = true;
        cursor.id = other.id;
        cursor.page = other.page;
        cursor.field = other.field;
        cursor.header = other.header;
        cursor.offset = other.offset;
        cursor.storage = other.storage;
        return cursor;
    }

    string::slice_t binary_op_name(binary_op_type_t type) {
        return s_binary_ops[(u32) type];
    }

    string::slice_t node_field_type_name(node_field_type_t type) {
        return s_node_field_types[(u32) type];
    }

    u32 find_field(node_cursor_t& cursor, node_field_type_t type) {
        cursor.ok = false;
        const auto field_count = (cursor.header->size - sizeof(node_header_t)) / sizeof(node_field_t);
        for (s32 i = 0; i < field_count; ++i) {
            auto field = (node_field_t*) cursor.page + cursor.offset;
            if (field->kind == (u8) node_kind_t::field
            &&  field->type == (u32) type) {
                cursor.ok = true;
                cursor.field = field;
                return cursor.field->value;
            }
            cursor.offset += sizeof(node_field_t);
        }
        return 0;
    }

    string::slice_t node_header_type_name(node_header_type_t type) {
        return s_node_header_types[(u32) type];
    }
}