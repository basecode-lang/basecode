// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include "attribute_container.h"
#include "attribute.h"

namespace basecode::graphviz {

    attribute_container_t::attribute_container_t(
            memory::allocator_t* allocator,
            model_t* model,
            component_type_t type) : _model(model),
                                     _type(type),
                                     _storage(allocator),
                                     _allocator(allocator),
                                     _values(allocator) {
        assert(_model);
        assert(_allocator);
    }

    component_type_t attribute_container_t::type() const {
        return _type;
    }

    attribute_value_list_t attribute_container_t::values() const {
        return _values.values();
    }

    bool attribute_container_t::set_value(result_t& r, attribute_type_t attr, bool v) {
        if (!_model->is_attribute_valid(r, _type, attr)) return false;

        auto wrapper = _values.find(attr);
        if (!wrapper) {
            wrapper = _storage.construct<attribute_value_t>();
            wrapper->type = attr;
            wrapper->value_type = attribute_value_type_t::boolean;
            _values.insert(attr, wrapper);
        }

        wrapper->value.flag = v;

        return true;
    }

    bool attribute_container_t::get_value(result_t& r, attribute_type_t attr, bool& v) {
        if (!_model->is_attribute_valid(r, _type, attr)) return false;

        auto wrapper = _values.find(attr);
        if (!wrapper)
            return false;

        v = wrapper->value.flag;

        return true;
    }

    bool attribute_container_t::set_value(result_t& r, attribute_type_t attr, double v) {
        if (!_model->is_attribute_valid(r, _type, attr)) return false;

        auto wrapper = _values.find(attr);
        if (!wrapper) {
            wrapper = _storage.construct<attribute_value_t>();
            wrapper->type = attr;
            wrapper->value_type = attribute_value_type_t::floating_point;
            _values.insert(attr, wrapper);
        }

        wrapper->value.floating_point = v;

        return true;
    }

    bool attribute_container_t::get_value(result_t& r, attribute_type_t attr, double& v) {
        if (!_model->is_attribute_valid(r, _type, attr)) return false;

        auto wrapper = _values.find(attr);
        if (!wrapper)
            return false;

        v = wrapper->value.floating_point;

        return true;
    }

    bool attribute_container_t::set_value(result_t& r, attribute_type_t attr, int32_t v) {
        if (!_model->is_attribute_valid(r, _type, attr)) return false;

        auto wrapper = _values.find(attr);
        if (!wrapper) {
            wrapper = _storage.construct<attribute_value_t>();
            wrapper->type = attr;
            wrapper->value_type = attribute_value_type_t::integer;
            _values.insert(attr, wrapper);
        }

        wrapper->value.integer = v;

        return true;
    }

    bool attribute_container_t::get_value(result_t& r, attribute_type_t attr, int32_t& v) {
        if (!_model->is_attribute_valid(r, _type, attr)) return false;

        auto wrapper = _values.find(attr);
        if (!wrapper)
            return false;

        v = wrapper->value.integer;

        return true;
    }

    bool attribute_container_t::get_value(result_t& r, attribute_type_t attr, adt::string_t& v) {
        if (!_model->is_attribute_valid(r, _type, attr)) return false;

        auto wrapper = _values.find(attr);
        if (!wrapper)
            return false;

        v = *wrapper->value.string;

        return true;
    }

    bool attribute_container_t::set_value(result_t& r, attribute_type_t attr, const adt::string_t& v) {
        if (!_model->is_attribute_valid(r, _type, attr)) return false;

        auto wrapper = _values.find(attr);
        if (!wrapper) {
            wrapper = _storage.construct<attribute_value_t>();
            wrapper->type = attr;
            wrapper->value_type = attribute_value_type_t::string;
            _values.insert(attr, wrapper);
        }

        auto str = _storage.construct<adt::string_t>(_allocator);
        *str = v;
        wrapper->value.string = str;

        return true;
    }

    bool attribute_container_t::get_value(result_t& r, attribute_type_t attr, enumeration_value_t& v) {
        if (!_model->is_attribute_valid(r, _type, attr)) return false;

        auto wrapper = _values.find(attr);
        if (!wrapper)
            return false;

        enumeration_value_t temp(wrapper->value.string->c_str());
        v = temp;

        return true;
    }

    bool attribute_container_t::set_value(result_t& r, attribute_type_t attr, const enumeration_value_t& v) {
        if (!_model->is_attribute_valid(r, _type, attr)) return false;

        auto wrapper = _values.find(attr);
        if (!wrapper) {
            wrapper = _storage.construct<attribute_value_t>();
            wrapper->type = attr;
            wrapper->value_type = attribute_value_type_t::enumeration;
            _values.insert(attr, wrapper);
        }

        auto str = _storage.construct<adt::string_t>(_allocator);
        *str = v.name;
        wrapper->value.string = str;

        return true;
    }

}