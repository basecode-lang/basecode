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

#pragma once

#include <set>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <parser/token.h>
#include <common/id_pool.h>

namespace basecode::compiler {

    class cast;
    class type;
    class with;
    class field;
    class block;
    class label;
    class import;
    class module;
    class spread;
    class element;
    class program;
    class comment;
    class any_type;
    class map_type;
    class raw_block;
    class intrinsic;
    class bool_type;
    class directive;
    class attribute;
    class statement;
    class type_info;
    class transmute;
    class rune_type;
    class assignment;
    class identifier;
    class expression;
    class array_type;
    class if_element;
    class tuple_type;
    class declaration;
    class module_type;
    class initializer;
    class string_type;
    class nil_literal;
    class for_element;
    class numeric_type;
    class unknown_type;
    class pointer_type;
    class defer_element;
    class break_element;
    class float_literal;
    class operator_base;
    class argument_list;
    class while_element;
    class type_reference;
    class assembly_label;
    class free_intrinsic;
    class symbol_element;
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
    class alloc_intrinsic;
    class continue_element;
    class module_reference;
    class array_constructor;
    class size_of_intrinsic;
    class namespace_element;
    class type_of_intrinsic;
    class character_literal;
    class align_of_intrinsic;
    class procedure_instance;
    class address_of_intrinsic;
    class identifier_reference;

    using type_list_t = std::vector<type*>;
    using label_list_t = std::vector<label*>;
    using block_list_t = std::vector<block*>;
    using field_list_t = std::vector<field*>;
    using string_set_t = std::set<std::string>;
    using import_list_t = std::vector<import*>;
    using element_list_t = std::vector<element*>;
    using comment_list_t = std::vector<comment*>;
    using string_list_t = std::vector<std::string>;
    using statement_list_t = std::vector<statement*>;
    using attribute_list_t = std::vector<attribute*>;
    using identifier_list_t = std::vector<identifier*>;
    using directive_map_t = std::map<std::string, directive*>;
    using procedure_type_list_t = std::vector<procedure_type*>;
    using procedure_instance_list_t = std::vector<procedure_instance*>;
    using identifier_reference_list_t = std::vector<identifier_reference*>;

    ///////////////////////////////////////////////////////////////////////////

    enum class composite_types_t {
        enum_type,
        union_type,
        struct_type,
    };

    static inline std::unordered_map<composite_types_t, std::string> s_composite_type_names = {
        {composite_types_t::enum_type, "enum_type"},
        {composite_types_t::union_type, "union_type"},
        {composite_types_t::struct_type, "struct_type"},
    };

    static inline std::string composite_type_name(composite_types_t type) {
        auto it = s_composite_type_names.find(type);
        if (it == s_composite_type_names.end())
            return "unknown_composite_type";
        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////

    enum class element_type_t {
        element = 1,
        cast,
        if_e,
        with,
        for_e,
        label,
        block,
        field,
        defer,
        symbol,
        module,
        spread,
        break_e,
        comment,
        program,
        while_e,
        any_type,
        return_e,
        import_e,
        map_type,
        rune_type,
        raw_block,
        intrinsic,
        proc_type,
        directive,
        attribute,
        bool_type,
        statement,
        proc_call,
        type_info,
        transmute,
        continue_e,
        array_type,
        identifier,
        expression,
        tuple_type,
        assignment,
        declaration,
        string_type,
        namespace_e,
        initializer,
        module_type,
        nil_literal,
        unknown_type,
        numeric_type,
        module_block,
        pointer_type,
        argument_list,
        proc_instance,
        float_literal,
        assembly_label,
        string_literal,
        composite_type,
        unary_operator,
        namespace_type,
        type_reference,
        boolean_literal,
        integer_literal,
        binary_operator,
        module_reference,
        array_constructor,
        character_literal,
        unknown_identifier,
        identifier_reference,
    };

    static inline std::unordered_map<element_type_t, std::string> s_element_type_names = {
        {element_type_t::if_e, "if"},
        {element_type_t::cast, "cast"},
        {element_type_t::with, "with"},
        {element_type_t::for_e, "for"},
        {element_type_t::label, "label"},
        {element_type_t::block, "block"},
        {element_type_t::field, "field"},
        {element_type_t::defer, "defer"},
        {element_type_t::spread, "spread"},
        {element_type_t::module, "module"},
        {element_type_t::symbol, "symbol"},
        {element_type_t::while_e, "while"},
        {element_type_t::break_e, "break"},
        {element_type_t::comment, "comment"},
        {element_type_t::element, "element"},
        {element_type_t::program, "program"},
        {element_type_t::map_type, "map_type"},
        {element_type_t::any_type, "any_type"},
        {element_type_t::return_e, "return"},
        {element_type_t::import_e, "import"},
        {element_type_t::rune_type, "rune_type"},
        {element_type_t::raw_block, "raw_block"},
        {element_type_t::intrinsic, "intrinsic"},
        {element_type_t::transmute, "transmute"},
        {element_type_t::proc_type, "proc_type"},
        {element_type_t::directive, "directive"},
        {element_type_t::attribute, "attribute"},
        {element_type_t::bool_type, "bool_type"},
        {element_type_t::statement, "statement"},
        {element_type_t::proc_call, "proc_call"},
        {element_type_t::type_info, "type_info"},
        {element_type_t::continue_e, "continue"},
        {element_type_t::assignment, "assignment"},
        {element_type_t::tuple_type, "tuple_type"},
        {element_type_t::array_type, "array_type"},
        {element_type_t::identifier, "identifier"},
        {element_type_t::expression, "expression"},
        {element_type_t::nil_literal, "nil_literal"},
        {element_type_t::declaration, "declaration"},
        {element_type_t::module_type, "module_type"},
        {element_type_t::string_type, "string_type"},
        {element_type_t::namespace_e, "namespace"},
        {element_type_t::initializer, "initializer"},
        {element_type_t::module_block, "module_block"},
        {element_type_t::unknown_type, "unknown_type"},
        {element_type_t::pointer_type, "pointer_type"},
        {element_type_t::numeric_type, "numeric_type"},
        {element_type_t::proc_instance, "proc_instance"},
        {element_type_t::float_literal, "float_literal"},
        {element_type_t::argument_list, "argument_list"},
        {element_type_t::namespace_type, "namespace_type"},
        {element_type_t::string_literal, "string_literal"},
        {element_type_t::composite_type, "composite_type"},
        {element_type_t::unary_operator, "unary_operator"},
        {element_type_t::assembly_label, "assembly_label"},
        {element_type_t::type_reference, "type_reference"},
        {element_type_t::boolean_literal, "boolean_literal"},
        {element_type_t::integer_literal, "integer_literal"},
        {element_type_t::binary_operator, "binary_operator"},
        {element_type_t::module_reference, "module_reference"},
        {element_type_t::character_literal, "character_literal"},
        {element_type_t::array_constructor, "array_constructor"},
        {element_type_t::unknown_identifier, "unknown_identifier"},
        {element_type_t::identifier_reference, "identifier_reference"},
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
        pointer_dereference,

        // binary
        add,
        subscript,
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
        assignment,
        member_access,
    };

    static inline std::unordered_map<operator_type_t, std::string> s_operator_type_names = {
        {operator_type_t::unknown,               "unknown"},
        {operator_type_t::negate,                "negate"},
        {operator_type_t::binary_not,            "binary_not"},
        {operator_type_t::logical_not,           "logical_not"},
        {operator_type_t::subscript,             "subscript"},
        {operator_type_t::pointer_dereference,   "pointer_deference"},
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
        {operator_type_t::assignment,            "assignment"},
        {operator_type_t::member_access,         "member_access"},
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
        {syntax::token_types_t::bang,  operator_type_t::logical_not},
        {syntax::token_types_t::caret, operator_type_t::pointer_dereference},
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
        {syntax::token_types_t::exponent,           operator_type_t::exponent},
        {syntax::token_types_t::assignment,         operator_type_t::assignment},
        {syntax::token_types_t::period,             operator_type_t::member_access},
    };

    static inline bool is_relational_operator(operator_type_t op) {
        switch (op) {
            case operator_type_t::equals:
            case operator_type_t::less_than:
            case operator_type_t::not_equals:
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
            case operator_type_t::greater_than:
            case operator_type_t::less_than_or_equal:
            case operator_type_t::greater_than_or_equal:
                return true;
            default: return false;
        }
    }

    static inline bool is_logical_conjunction_operator(operator_type_t op) {
        switch (op) {
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
                return true;
            default: return false;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    struct attribute_map_t {
        inline size_t size() const {
            return _attrs.size();
        }

        void add(attribute* value);

        attribute_list_t as_list() const;

        bool remove(const std::string& name);

        compiler::attribute* find(const std::string& name);

    private:
        std::unordered_map<std::string, attribute*> _attrs {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct field_map_t {
        void add(field* value);

        field_list_t as_list();

        inline size_t size() const {
            return _fields.size();
        }

        bool remove(common::id_t id);

        compiler::field* find(common::id_t id);

        compiler::field* find_by_name(const std::string& name);

    private:
        std::map<common::id_t, field*> _fields {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct identifier_map_t {
        void dump();

        void add(identifier* value);

        bool empty() const {
            return _identifiers.empty();
        }

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

        type_list_t as_list() const;

        void add(compiler::type* type);

        bool remove(const std::string& name);

        compiler::type* find(const std::string& name);

    private:
        std::unordered_map<std::string, type*> _types {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct qualified_symbol_t {
        bool is_qualified() const {
            return !namespaces.empty();
        }
        std::string name {};
        string_list_t namespaces {};
        std::string fully_qualified_name {};
        common::source_location location {};
    };

    std::string make_fully_qualified_name(const symbol_element* symbol);

    std::string make_fully_qualified_name(const qualified_symbol_t& symbol);

    ///////////////////////////////////////////////////////////////////////////

    class element_builder;

    struct type_find_result_t {
        compiler::type_reference* make_type_reference(
            element_builder& builder,
            compiler::block* parent_scope);

        qualified_symbol_t type_name;
        bool is_array = false;
        bool is_spread = false;
        bool is_pointer = false;
        compiler::type* type = nullptr;
        element_list_t array_subscripts {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct fold_result_t {
        bool allow_no_fold_attribute = true;
        compiler::element* element = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    struct infer_type_result_t {
        std::string type_name() const;

        compiler::type* inferred_type = nullptr;
        compiler::type_reference* reference = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    enum register_tags_t : uint8_t {
        tag_rel_expr_target = 1
    };

};