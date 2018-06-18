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

#pragma once

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace basecode::compiler {

    class type;
    class field;
    class block;
    class element;
    class program;
    class directive;
    class attribute;
    class identifier;
    class expression;
    class array_type;
    class initializer;
    class string_type;
    class numeric_type;
    class float_literal;
    class operator_base;
    class procedure_type;
    class composite_type;
    class unary_operator;
    class boolean_literal;
    class integer_literal;
    class binary_operator;
    class namespace_element;

    using element_list_t = std::vector<element*>;
    using directive_map_t = std::map<std::string, directive*>;

    struct attribute_map_t {
        attribute_map_t(element* parent);

        ~attribute_map_t();

        inline size_t size() const {
            return _attrs.size();
        }

        bool remove(const std::string& name);

        compiler::attribute* find(const std::string& name);

        attribute* add(const std::string& name, element* expr);

    private:
        element* _parent = nullptr;
        std::unordered_map<std::string, attribute*> _attrs {};
    };

};