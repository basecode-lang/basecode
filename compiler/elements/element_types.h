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
    class any_type;
    class directive;
    class attribute;
    class identifier;
    class expression;
    class array_type;
    class initializer;
    class string_type;
    class line_comment;
    class numeric_type;
    class block_comment;
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

    ///////////////////////////////////////////////////////////////////////////

    struct attribute_map_t {
        inline size_t size() const {
            return _attrs.size();
        }

        void add(attribute* value);

        bool remove(const std::string& name);

        compiler::attribute* find(const std::string& name);

    private:
        std::unordered_map<std::string, attribute*> _attrs {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct field_map_t {
        void add(field* value);

        inline size_t size() const {
            return _fields.size();
        }

        bool remove(const std::string& name);

        compiler::field* find(const std::string& name);

    private:
        std::unordered_map<std::string, field*> _fields {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct identifier_map_t {
        void add(identifier* value);

        size_t size() const {
            return _identifiers.size();
        }

        bool remove(const std::string& name);

        identifier* find(const std::string& name);

        // XXX: add ability to get range of identifiers for overloads

    private:
        std::unordered_multimap<std::string, identifier*> _identifiers {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct type_map_t {
        size_t size() const {
            return _types.size();
        }

        void add(compiler::type* type);

        bool remove(const std::string& name);

        compiler::type* find(const std::string& name);

    private:
        std::unordered_map<std::string, type*> _types {};
    };
};