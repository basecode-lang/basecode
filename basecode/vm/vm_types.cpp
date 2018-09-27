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
#include "label.h"
#include "vm_types.h"

namespace basecode::vm {

    ///////////////////////////////////////////////////////////////////////////

    void listing_source_file_t::add_blank_lines(uint64_t address, uint16_t count) {
        for (uint16_t i = 0; i < count; i++) {
            lines.push_back(listing_source_line_t {
                .address = address,
                .source = ""
            });
        }
    }

    void listing_source_file_t::add_source_line(uint64_t address, const std::string& source) {
        lines.push_back(listing_source_line_t {
            .address = address,
            .source = source
        });
    }

    ///////////////////////////////////////////////////////////////////////////

    block_entry_t::block_entry_t() : _data({}),
                                     _type(block_entry_type_t::blank_line) {
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

};