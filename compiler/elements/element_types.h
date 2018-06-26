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
#include <parser/token.h>

namespace basecode::compiler {

    class cast;
    class type;
    class field;
    class block;
    class label;
    class alias;
    class element;
    class program;
    class comment;
    class any_type;
    class directive;
    class attribute;
    class statement;
    class identifier;
    class expression;
    class array_type;
    class if_element;
    class initializer;
    class string_type;
    class numeric_type;
    class float_literal;
    class operator_base;
    class procedure_type;
    class composite_type;
    class unary_operator;
    class return_element;
    class procedure_call;
    class string_literal;
    class namespace_type;
    class boolean_literal;
    class integer_literal;
    class binary_operator;
    class namespace_element;
    class procedure_instance;

    using label_list_t = std::vector<label*>;
    using block_list_t = std::vector<block*>;
    using element_list_t = std::vector<element*>;
    using comment_list_t = std::vector<comment*>;
    using statement_list_t = std::vector<statement*>;
    using identifier_list_t = std::vector<identifier*>;
    using directive_map_t = std::map<std::string, directive*>;
    using element_map_t = std::unordered_map<common::id_t, element*>;
    using procedure_instance_list_t = std::vector<procedure_instance*>;

    ///////////////////////////////////////////////////////////////////////////

    enum class element_type_t {
        element = 1,
        cast,
        if_e,
        label,
        block,
        field,
        comment,
        program,
        any_type,
        return_e,
        proc_type,
        directive,
        attribute,
        bool_type,
        statement,
        proc_call,
        alias_type,
        array_type,
        identifier,
        expression,
        string_type,
        namespace_e,
        initializer,
        numeric_type,
        proc_instance,
        float_literal,
        string_literal,
        composite_type,
        unary_operator,
        namespace_type,
        boolean_literal,
        integer_literal,
        binary_operator,
    };

    static inline std::unordered_map<element_type_t, std::string> s_element_type_names = {
        {element_type_t::element, "element"},
        {element_type_t::cast, "cast"},
        {element_type_t::if_e, "if"},
        {element_type_t::label, "label"},
        {element_type_t::block, "block"},
        {element_type_t::field, "field"},
        {element_type_t::comment, "comment"},
        {element_type_t::program, "program"},
        {element_type_t::any_type, "any_type"},
        {element_type_t::return_e, "return"},
        {element_type_t::proc_type, "proc_type"},
        {element_type_t::directive, "directive"},
        {element_type_t::attribute, "attribute"},
        {element_type_t::bool_type, "bool_type"},
        {element_type_t::statement, "statement"},
        {element_type_t::proc_call, "proc_call"},
        {element_type_t::alias_type, "alias_type"},
        {element_type_t::array_type, "array_type"},
        {element_type_t::identifier, "identifier"},
        {element_type_t::expression, "expression"},
        {element_type_t::string_type, "string_type"},
        {element_type_t::namespace_e, "namespace"},
        {element_type_t::initializer, "initializer"},
        {element_type_t::numeric_type, "numeric_type"},
        {element_type_t::proc_instance, "proc_instance"},
        {element_type_t::float_literal, "float_literal"},
        {element_type_t::namespace_type, "namespace_type"},
        {element_type_t::string_literal, "string_literal"},
        {element_type_t::composite_type, "composite_type"},
        {element_type_t::unary_operator, "unary_operator"},
        {element_type_t::boolean_literal, "boolean_literal"},
        {element_type_t::integer_literal, "integer_literal"},
        {element_type_t::binary_operator, "binary_operator"},
    };

    static inline std::string element_type_name(element_type_t type) {
        auto it = s_element_type_names.find(type);
        if (it == s_element_type_names.end()) {
            return "";
        }
        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////

    enum class comment_type_t {
        line = 1,
        block
    };

    static inline std::string comment_type_name(comment_type_t type) {
        switch (type) {
            case comment_type_t::line:  return "line";
            case comment_type_t::block: return "block";
        }
        return "unknown";
    }

    ///////////////////////////////////////////////////////////////////////////

    enum class operator_type_t {
        unknown,

        // unary
        negate,
        binary_not,
        logical_not,

        // binary
        add,
        subtract,
        multiply,
        divide,
        modulo,
        equals,
        not_equals,
        greater_than,
        less_than,
        greater_than_or_equal,
        less_than_or_equal,
        logical_or,
        logical_and,
        binary_or,
        binary_and,
        binary_xor,
        shift_right,
        shift_left,
        rotate_right,
        rotate_left,
        exponent,
        assignment
    };

    static inline std::unordered_map<operator_type_t, std::string> s_operator_type_names = {
        {operator_type_t::unknown,               "unknown"},
        {operator_type_t::negate,                "negate"},
        {operator_type_t::binary_not,            "binary_not"},
        {operator_type_t::logical_not,           "logical_not"},
        {operator_type_t::add,                   "add"},
        {operator_type_t::subtract,              "subtract"},
        {operator_type_t::multiply,              "multiply"},
        {operator_type_t::divide,                "divide"},
        {operator_type_t::modulo,                "modulo"},
        {operator_type_t::equals,                "equals"},
        {operator_type_t::not_equals,            "not_equals"},
        {operator_type_t::greater_than,          "greater_than"},
        {operator_type_t::less_than,             "less_than"},
        {operator_type_t::greater_than_or_equal, "greater_than_or_equal"},
        {operator_type_t::less_than_or_equal,    "less_than_or_equal"},
        {operator_type_t::logical_or,            "logical_or"},
        {operator_type_t::logical_and,           "logical_and"},
        {operator_type_t::binary_or,             "binary_or"},
        {operator_type_t::binary_and,            "binary_and"},
        {operator_type_t::binary_xor,            "binary_xor"},
        {operator_type_t::shift_right,           "shift_right"},
        {operator_type_t::shift_left,            "shift_left"},
        {operator_type_t::rotate_right,          "rotate_right"},
        {operator_type_t::rotate_left,           "rotate_left"},
        {operator_type_t::exponent,              "exponent"},
        {operator_type_t::assignment,            "assignment"}
    };

    static inline std::string operator_type_name(operator_type_t type) {
        auto it = s_operator_type_names.find(type);
        if (it == s_operator_type_names.end())
            return "";
        return it->second;
    }
    
    static inline std::unordered_map<syntax::token_types_t, operator_type_t> s_unary_operators = {
        {syntax::token_types_t::minus, operator_type_t::negate},
        {syntax::token_types_t::tilde, operator_type_t::binary_not},
        {syntax::token_types_t::bang,  operator_type_t::logical_not}
    };

    static inline std::unordered_map<syntax::token_types_t, operator_type_t> s_binary_operators = {
        {syntax::token_types_t::plus,               operator_type_t::add},
        {syntax::token_types_t::minus,              operator_type_t::subtract},
        {syntax::token_types_t::asterisk,           operator_type_t::multiply},
        {syntax::token_types_t::slash,              operator_type_t::divide},
        {syntax::token_types_t::percent,            operator_type_t::modulo},
        {syntax::token_types_t::equals,             operator_type_t::equals},
        {syntax::token_types_t::not_equals,         operator_type_t::not_equals},
        {syntax::token_types_t::greater_than,       operator_type_t::greater_than},
        {syntax::token_types_t::less_than,          operator_type_t::less_than},
        {syntax::token_types_t::greater_than_equal, operator_type_t::greater_than_or_equal},
        {syntax::token_types_t::less_than_equal,    operator_type_t::less_than_or_equal},
        {syntax::token_types_t::not_equals,         operator_type_t::not_equals},
        {syntax::token_types_t::logical_or,         operator_type_t::logical_or},
        {syntax::token_types_t::logical_and,        operator_type_t::logical_and},
        {syntax::token_types_t::pipe,               operator_type_t::binary_or},
        {syntax::token_types_t::ampersand,          operator_type_t::binary_and},
        {syntax::token_types_t::xor_literal,        operator_type_t::binary_xor},
        {syntax::token_types_t::shl_literal,        operator_type_t::shift_left},
        {syntax::token_types_t::shr_literal,        operator_type_t::shift_right},
        {syntax::token_types_t::rol_literal,        operator_type_t::rotate_left},
        {syntax::token_types_t::ror_literal,        operator_type_t::rotate_right},
        {syntax::token_types_t::caret,              operator_type_t::exponent},
        {syntax::token_types_t::assignment,         operator_type_t::assignment},
    };

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

        identifier_list_t as_list() const;

        bool remove(const std::string& name);

        identifier* find(const std::string& name);

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