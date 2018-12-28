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

#include <dyncall/dyncall.h>
#include <dynload/dynload.h>
#include <dyncall/dyncall_struct.h>
#include <dyncall/dyncall_signature.h>
#include "terp.h"
#include "label.h"
#include "vm_types.h"
#include "assembler.h"

namespace basecode::vm {

    ///////////////////////////////////////////////////////////////////////////

    void listing_source_file_t::add_source_line(
            listing_source_line_type_t type,
            uint64_t address,
            const std::string& source) {
        lines.push_back(listing_source_line_t {
            .address = address,
            .source = source,
            .type = type
        });
    }

    void listing_source_file_t::add_blank_lines(
            uint64_t address,
            uint16_t count) {
        for (uint16_t i = 0; i < count; i++) {
            lines.push_back(listing_source_line_t {
                .address = address,
                .source = "",
                .type = listing_source_line_type_t::blank
            });
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    block_entry_t::block_entry_t() : _data({}),
                                     _type(block_entry_type_t::blank_line) {
    }

    block_entry_t::block_entry_t(const meta_t& meta) : _data(boost::any(meta)),
                                                       _type(block_entry_type_t::meta) {

    }

    block_entry_t::block_entry_t(const block_entry_t& other) : _data(other._data),
                                                               _address(other._address),
                                                               _type(other._type) {
    }

    block_entry_t::block_entry_t(const align_t& align) : _data(boost::any(align)),
                                                         _type(block_entry_type_t::align) {
    }

    block_entry_t::block_entry_t(const label_t& label) : _data(boost::any(label)),
                                                         _type(block_entry_type_t::label) {
    }

    block_entry_t::block_entry_t(const section_t& section) : _data(boost::any(section)),
                                                             _type(block_entry_type_t::section) {
    }

    block_entry_t::block_entry_t(const comment_t& comment) : _data(boost::any(comment)),
                                                             _type(block_entry_type_t::comment) {
    }

    block_entry_t::block_entry_t(const data_definition_t& data) : _data(boost::any(data)),
                                                                  _type(block_entry_type_t::data_definition) {
    }

    block_entry_t::block_entry_t(const instruction_t& instruction) : _data(boost::any(instruction)),
                                                                     _type(block_entry_type_t::instruction) {
    }

    uint64_t block_entry_t::address() const {
        return _address;
    }

    block_entry_type_t block_entry_t::type() const {
        return _type;
    }

    block_entry_t* block_entry_t::address(uint64_t value) {
        _address = value;
        if (_type == block_entry_type_t::label) {
            auto label = data<label_t>();
            label->instance->address(value);
        }
        return this;
    }

    ///////////////////////////////////////////////////////////////////////////

    shared_library_t::shared_library_t() {
    }

    shared_library_t::~shared_library_t() {
        if (_library != nullptr)
            dlFreeLibrary(_library);
    }

    bool shared_library_t::initialize(
            common::result& r,
            const boost::filesystem::path& path) {
        _library = dlLoadLibrary(path.string().c_str());
        if (_library == nullptr) {
            r.add_message(
                "B062",
                fmt::format("unable to load library image file: {}.", path.string()),
                true);
            return false;
        }
        get_library_path();
        return true;
    }

    bool shared_library_t::initialize(common::result& r) {
        _library = dlLoadLibrary(nullptr);
        if (_library == nullptr) {
            r.add_message(
                "B062",
                fmt::format("unable to load library image for self."),
                true);
            return false;
        }
        get_library_path();
        return true;
    }

    void shared_library_t::get_library_path() {
        if (_library == nullptr)
            return;

        char library_path[PATH_MAX];
        dlGetLibraryPath(_library, library_path, PATH_MAX);
        _path = library_path;
    }

    void shared_library_t::load_symbols(const char* path) {
        _symbols.clear();
        auto symbol_ptr = dlSymsInit(path);
        if (symbol_ptr != nullptr) {
            int count = dlSymsCount(symbol_ptr);
            for (int i = 0; i < count; i++) {
                const char* symbol_name = dlSymsName(symbol_ptr, i);
                if (symbol_name != nullptr)
                    _symbols.insert(std::make_pair(symbol_name, nullptr));
            }
            dlSymsCleanup(symbol_ptr);
        }
    }

    bool shared_library_t::exports_symbol(const std::string& symbol_name) {
        return _symbols.count(symbol_name) > 0;
    }

    void* shared_library_t::symbol_address(const std::string& symbol_name) {
        DLLib* effective_library = nullptr;
#if defined(__FreeBSD__)
        effective_library = _self_loaded ? nullptr : _library;
#else
        effective_library = _library;
#endif
        auto it = _symbols.find(symbol_name);
        if (it == _symbols.end()) {
            auto func_ptr = dlFindSymbol(effective_library, symbol_name.c_str());
            _symbols.insert(std::make_pair(symbol_name, func_ptr));
            return func_ptr;
        }

        if (it->second == nullptr) {
            it->second = dlFindSymbol(effective_library, symbol_name.c_str());
        }

        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////

    allocator::~allocator() {
    }

    ///////////////////////////////////////////////////////////////////////////

    bool instruction_operand_t::allocate(
            vm::assembler& assembler,
            instruction_operand_t& operand,
            op_sizes size,
            register_type_t type) {
        register_t reg {
            .size = size,
            .type = type
        };
        auto success = assembler.allocate_reg(reg);
        if (success) {
            operand._data = reg;
            operand._size = size;
            operand._type = instruction_operand_type_t::reg;
        }
        return success;
    }

    instruction_operand_t instruction_operand_t::offset(
            int64_t value,
            op_sizes size) {
        return value == 0 ?
               instruction_operand_t() :
               instruction_operand_t(value, size);
    }

    instruction_operand_t instruction_operand_t::fp() {
        return instruction_operand_t(register_t::fp());
    }

    instruction_operand_t instruction_operand_t::sp() {
        return instruction_operand_t(register_t::sp());
    }

    instruction_operand_t instruction_operand_t::pc() {
        return instruction_operand_t(register_t::pc());
    }

    instruction_operand_t instruction_operand_t::empty() {
        return instruction_operand_t {};
    }

    instruction_operand_t::instruction_operand_t() : _type(instruction_operand_type_t::empty) {
    }

    instruction_operand_t::instruction_operand_t(
            int64_t immediate,
            op_sizes size) : _data(immediate),
                             _size(size),
                             _type(instruction_operand_type_t::imm_sint) {
    }

    instruction_operand_t::instruction_operand_t(
            uint64_t immediate,
            op_sizes size) : _data(immediate),
                             _size(size),
                             _type(instruction_operand_type_t::imm_uint) {
    }

    instruction_operand_t::instruction_operand_t(register_t reg) : _data(reg),
                                                                   _size(reg.size),
                                                                   _type(instruction_operand_type_t::reg) {
    }

    instruction_operand_t::instruction_operand_t(float immediate) : _data(immediate),
                                                                    _size(op_sizes::dword),
                                                                    _type(instruction_operand_type_t::imm_f32) {
    }

    instruction_operand_t::instruction_operand_t(double immediate) : _data(immediate),
                                                                     _size(op_sizes::qword),
                                                                     _type(instruction_operand_type_t::imm_f64) {
    }

    instruction_operand_t::instruction_operand_t(label_ref_t* label_ref): _data(label_ref),
                                                                          _type(instruction_operand_type_t::label_ref) {
    }

    void instruction_operand_t::free(vm::assembler& assembler) {
        if (_type != instruction_operand_type_t::reg)
            return;

        assembler.free_reg(*data<register_t>());
    }

    ///////////////////////////////////////////////////////////////////////////

    void assembly_symbol_result_t::data(const compiler_label_data_t& value) {
        _data = value;
        _is_set = true;
    }

    void assembly_symbol_result_t::data(const compiler_local_data_t& value) {
        _data = value;
        _is_set = true;
    }

    void assembly_symbol_result_t::data(const compiler_module_data_t& value) {
        _data = value;
        _is_set = true;
    }
};