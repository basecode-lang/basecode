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

    element::element(element* parent) : _id(common::id_pool::instance()->allocate()),
                                        _parent(parent) {
    }

    element::~element() {
    }

    id_t element::id() const {
        return _id;
    }

    element* element::parent() {
        return _parent;
    }

    attribute_map_t& element::attributes() {
        return _attributes;
    }

    bool element::fold(common::result& result) {
        return on_fold(result);
    }

    bool element::on_fold(common::result& result) {
        return true;
    }

};