// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#include <common/id_pool.h>
#include "element.h"

namespace basecode::compiler {

    element::element() : _id(common::id_pool::instance()->allocate()) {
    }

    element::~element() {
    }

    id_t element::id() const {
        return _id;
    }

    bool element::fold(common::result& result) {
        return on_fold(result);
    }

    bool element::on_fold(common::result& result) {
        return true;
    }

    bool element::remove_attribute(const std::string& name) {
        return _attributes.erase(name) > 0;
    }

    bool element::remove_directive(const std::string& name) {
        return _directives.erase(name) > 0;
    }

    attribute* element::find_attribute(const std::string& name) {
        auto it = _attributes.find(name);
        if (it != _attributes.end())
            return it->second;
        return nullptr;
    }

    directive* element::find_directive(const std::string& name) {
        auto it = _directives.find(name);
        if (it != _directives.end())
            return it->second;
        return nullptr;
    }

};