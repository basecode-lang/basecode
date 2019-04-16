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
#include <stack>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <parser/token.h>
#include <common/id_pool.h>

namespace basecode::compiler {

    using namespace std::literals;

    class cast;
    class type;
    class with;
    class field;
    class block;
    class label;
    class yield;
    class import;
    class module;
    class element;
    class program;
    class comment;
    class raw_block;
    class intrinsic;
    class bool_type;
    class directive;
    class attribute;
    class statement;
    class transmute;
    class rune_type;
    class assignment;
    class identifier;
    class expression;
    class array_type;
    class if_element;
    class tuple_type;
    class fallthrough;
    class declaration;
    class module_type;
    class initializer;
    class nil_literal;
    class for_element;
    class family_type;
    class numeric_type;
    class unknown_type;
    class pointer_type;
    class type_literal;
    class generic_type;
    class case_element;
    class if_directive;
    class run_directive;
    class defer_element;
    class break_element;
    class float_literal;
    class operator_base;
    class argument_list;
    class while_element;
    class argument_pair;
    class type_directive;
    class switch_element;
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
    class label_reference;
    class spread_operator;
    class range_intrinsic;
    class boolean_literal;
    class integer_literal;
    class binary_operator;
    class alloc_intrinsic;
    class continue_element;
    class module_reference;
    class size_of_intrinsic;
    class namespace_element;
    class type_of_intrinsic;
    class character_literal;
    class foreign_directive;
    class value_sink_literal;
    class align_of_intrinsic;
    class assembly_directive;
    class length_of_intrinsic;
    class intrinsic_directive;
    class core_type_directive;
    class address_of_intrinsic;
    class identifier_reference;
    class uninitialized_literal;
    class assembly_literal_label;

    using type_set_t = std::set<type*>;
    using type_list_t = std::vector<type*>;
    using import_set_t = std::set<import*>;
    using label_list_t = std::vector<label*>;
    using block_list_t = std::vector<block*>;
    using field_list_t = std::vector<field*>;
    using string_set_t = std::set<std::string>;
    using import_list_t = std::vector<import*>;
    using element_list_t = std::vector<element*>;
    using comment_list_t = std::vector<comment*>;
    using string_list_t = std::vector<std::string>;
    using defer_stack_t = std::stack<defer_element*>;
    using statement_list_t = std::vector<statement*>;
    using attribute_list_t = std::vector<attribute*>;
    using identifier_list_t = std::vector<identifier*>;
    using block_stack_t = std::stack<compiler::block*>;
    using module_stack_t = std::stack<compiler::module*>;
    using procedure_call_set_t = std::set<procedure_call*>;
    using type_literal_list_t = std::vector<type_literal*>;
    using procedure_type_set_t = std::set<procedure_type*>;
    using string_view_list_t = std::vector<std::string_view>;
    using directive_map_t = std::map<std::string, directive*>;
    using type_reference_list_t = std::vector<type_reference*>;
    using procedure_type_list_t = std::vector<procedure_type*>;
    using const_attribute_list_t = std::vector<const attribute*>;
    using binary_operator_stack_t = std::stack<compiler::binary_operator*>;
    using identifier_reference_stack_t = std::stack<compiler::identifier_reference*>;
    using identifier_reference_list_t = std::vector<compiler::identifier_reference*>;
    using procedure_type_map_t = std::unordered_map<std::string_view, procedure_type*>;

    using element_id_set_t = std::unordered_set<common::id_t>;

    ///////////////////////////////////////////////////////////////////////////

    enum class intrinsic_type_t : uint8_t {
        unknown,
        free,
        copy,
        fill,
        alloc,
        range,
        size_of,
        type_of,
        align_of,
        length_of,
        address_of,
    };

    static inline std::unordered_map<intrinsic_type_t, std::string_view> s_intrinsic_type_names = {
        {intrinsic_type_t::unknown,     "unknown"sv},
        {intrinsic_type_t::free,        "free"sv},
        {intrinsic_type_t::copy,        "copy"sv},
        {intrinsic_type_t::fill,        "fill"sv},
        {intrinsic_type_t::alloc,       "alloc"sv},
        {intrinsic_type_t::range,       "range"sv},
        {intrinsic_type_t::size_of,     "size_of"sv},
        {intrinsic_type_t::type_of,     "type_of"sv},
        {intrinsic_type_t::align_of,    "align_of"sv},
        {intrinsic_type_t::length_of,   "length_of"sv},
        {intrinsic_type_t::address_of,  "address_of"sv},
    };

    static inline std::unordered_map<std::string_view, intrinsic_type_t> s_intrinsic_named_types = {
        {"unknown"sv,     intrinsic_type_t::unknown},
        {"free"sv,        intrinsic_type_t::free},
        {"copy"sv,        intrinsic_type_t::copy},
        {"fill"sv,        intrinsic_type_t::fill},
        {"alloc"sv,       intrinsic_type_t::alloc},
        {"range"sv,       intrinsic_type_t::range},
        {"size_of"sv,     intrinsic_type_t::size_of},
        {"type_of"sv,     intrinsic_type_t::type_of},
        {"align_of"sv,    intrinsic_type_t::align_of},
        {"length_of"sv,   intrinsic_type_t::length_of},
        {"address_of"sv,  intrinsic_type_t::address_of},
    };

    static inline std::string_view intrinsic_type_to_name(intrinsic_type_t type) {
        auto it = s_intrinsic_type_names.find(type);
        if (it == std::end(s_intrinsic_type_names))
            return "unknown"sv;
        return it->second;
    }

    static inline intrinsic_type_t intrinsic_type_from_name(const std::string_view& name) {
        auto it = s_intrinsic_named_types.find(name);
        if (it == std::end(s_intrinsic_named_types))
            return intrinsic_type_t::unknown;
        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////

    enum class directive_type_t : uint32_t {
        unknown,
        run,
        if_e,
        eval,
        type,
        assert,
        foreign,
        assembly,
        core_type,
        intrinsic_e,
    };

    static inline std::unordered_map<directive_type_t, std::string_view> s_directive_type_names = {
        {directive_type_t::unknown,     "unknown"sv},
        {directive_type_t::run,         "run"sv},
        {directive_type_t::if_e,        "if"sv},
        {directive_type_t::eval,        "eval"sv},
        {directive_type_t::type,        "type"sv},
        {directive_type_t::assert,      "assert"sv},
        {directive_type_t::foreign,     "foreign"sv},
        {directive_type_t::assembly,    "assembly"sv},
        {directive_type_t::core_type,   "core_type"sv},
        {directive_type_t::intrinsic_e, "intrinsic"sv},
    };

    static inline std::unordered_map<std::string_view, directive_type_t> s_directive_named_types = {
        {"unknown"sv,     directive_type_t::unknown},
        {"run"sv,         directive_type_t::run},
        {"if"sv,          directive_type_t::if_e},
        {"elif"sv,        directive_type_t::if_e},
        {"else"sv,        directive_type_t::if_e},
        {"eval"sv,        directive_type_t::eval},
        {"type"sv,        directive_type_t::type},
        {"assert"sv,      directive_type_t::assert},
        {"foreign"sv,     directive_type_t::foreign},
        {"assembly"sv,    directive_type_t::assembly},
        {"core_type"sv,   directive_type_t::core_type},
        {"intrinsic"sv,   directive_type_t::intrinsic_e},
    };

    static inline std::string_view directive_type_to_name(directive_type_t type) {
        auto it = s_directive_type_names.find(type);
        if (it == std::end(s_directive_type_names))
            return "unknown"sv;
        return it->second;
    }

    static inline directive_type_t directive_type_from_name(const std::string_view& name) {
        auto it = s_directive_named_types.find(name);
        if (it == std::end(s_directive_named_types))
            return directive_type_t::unknown;
        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////

    enum class visitor_data_type_t {
        none,
        type,
        module,
        identifier,
        identifier_list
    };

    struct visitor_result_t {
    public:
        visitor_result_t() : _type(visitor_data_type_t::none) {
        }

        explicit visitor_result_t(compiler::type* type) : _data(type),
                                                          _type(visitor_data_type_t::type) {
        }

        explicit visitor_result_t(compiler::module* value) : _data(value),
                                                             _type(visitor_data_type_t::module) {
        }

        explicit visitor_result_t(compiler::identifier* var) : _data(var),
                                                               _type(visitor_data_type_t::identifier) {
        }

        explicit visitor_result_t(const compiler::identifier_list_t& list) : _data(list),
                                                                             _type(visitor_data_type_t::identifier_list) {
        }

        template <typename T>
        T* data() {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        template <typename T>
        const T* data() const {
            if (_data.empty())
                return nullptr;
            try {
                return boost::any_cast<T>(&_data);
            } catch (const boost::bad_any_cast& e) {
                return nullptr;
            }
        }

        bool empty() const {
            return _type == visitor_data_type_t::none;
        }

        visitor_data_type_t type() const {
            return _type;
        }

    private:
        boost::any _data;
        visitor_data_type_t _type;
    };

    using block_visitor_callable = std::function<bool (compiler::block*)>;
    using scope_visitor_callable = std::function<visitor_result_t (compiler::block*)>;
    using element_visitor_callable = std::function<visitor_result_t (compiler::element*)>;
    using namespace_visitor_callable = std::function<visitor_result_t (compiler::block*)>;

    ///////////////////////////////////////////////////////////////////////////

    enum class composite_types_t {
        enum_type,
        union_type,
        struct_type,
    };

    static inline std::unordered_map<composite_types_t, std::string_view> s_composite_type_names = {
        {composite_types_t::enum_type, "enum_type"sv},
        {composite_types_t::union_type, "union_type"sv},
        {composite_types_t::struct_type, "struct_type"sv},
    };

    static inline std::string_view composite_type_name(composite_types_t type) {
        auto it = s_composite_type_names.find(type);
        if (it == s_composite_type_names.end())
            return "unknown_composite_type"sv;
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
        yield,
        symbol,
        module,
        case_e,
        break_e,
        comment,
        program,
        while_e,
        return_e,
        import_e,
        switch_e,
        rune_type,
        raw_block,
        intrinsic,
        proc_type,
        directive,
        attribute,
        bool_type,
        statement,
        proc_call,
        transmute,
        continue_e,
        array_type,
        identifier,
        expression,
        tuple_type,
        assignment,
        declaration,
        namespace_e,
        initializer,
        module_type,
        fallthrough,
        nil_literal,
        family_type,
        type_literal,
        unknown_type,
        numeric_type,
        pointer_type,
        generic_type,
        argument_pair,
        argument_list,
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
        spread_operator,
        label_reference,
        module_reference,
        character_literal,
        value_sink_literal,
        unknown_identifier,
        identifier_reference,
        uninitialized_literal,
        assembly_literal_label
    };

    static inline std::unordered_map<element_type_t, std::string_view> s_element_type_names = {
        {element_type_t::if_e, "if"sv},
        {element_type_t::cast, "cast"sv},
        {element_type_t::with, "with"sv},
        {element_type_t::for_e, "for"sv},
        {element_type_t::label, "label"sv},
        {element_type_t::block, "block"sv},
        {element_type_t::field, "field"sv},
        {element_type_t::defer, "defer"sv},
        {element_type_t::case_e, "case"sv},
        {element_type_t::yield, "yield"sv},
        {element_type_t::module, "module"sv},
        {element_type_t::symbol, "symbol"sv},
        {element_type_t::while_e, "while"sv},
        {element_type_t::break_e, "break"sv},
        {element_type_t::comment, "comment"sv},
        {element_type_t::element, "element"sv},
        {element_type_t::program, "program"sv},
        {element_type_t::return_e, "return"sv},
        {element_type_t::import_e, "import"sv},
        {element_type_t::switch_e, "switch"sv},
        {element_type_t::rune_type, "rune_type"sv},
        {element_type_t::raw_block, "raw_block"sv},
        {element_type_t::intrinsic, "intrinsic"sv},
        {element_type_t::transmute, "transmute"sv},
        {element_type_t::proc_type, "proc_type"sv},
        {element_type_t::directive, "directive"sv},
        {element_type_t::attribute, "attribute"sv},
        {element_type_t::bool_type, "bool_type"sv},
        {element_type_t::statement, "statement"sv},
        {element_type_t::proc_call, "proc_call"sv},
        {element_type_t::continue_e, "continue"sv},
        {element_type_t::assignment, "assignment"sv},
        {element_type_t::tuple_type, "tuple_type"sv},
        {element_type_t::array_type, "array_type"sv},
        {element_type_t::identifier, "identifier"sv},
        {element_type_t::expression, "expression"sv},
        {element_type_t::namespace_e, "namespace"sv},
        {element_type_t::fallthrough, "fallthrough"sv},
        {element_type_t::nil_literal, "nil_literal"sv},
        {element_type_t::declaration, "declaration"sv},
        {element_type_t::module_type, "module_type"sv},
        {element_type_t::initializer, "initializer"sv},
        {element_type_t::family_type, "family_type"sv},
        {element_type_t::generic_type, "generic_type"sv},
        {element_type_t::type_literal, "type_literal"sv},
        {element_type_t::unknown_type, "unknown_type"sv},
        {element_type_t::pointer_type, "pointer_type"sv},
        {element_type_t::numeric_type, "numeric_type"sv},
        {element_type_t::float_literal, "float_literal"sv},
        {element_type_t::argument_list, "argument_list"sv},
        {element_type_t::argument_pair, "argument_pair"sv},
        {element_type_t::namespace_type, "namespace_type"sv},
        {element_type_t::string_literal, "string_literal"sv},
        {element_type_t::composite_type, "composite_type"sv},
        {element_type_t::unary_operator, "unary_operator"sv},
        {element_type_t::assembly_label, "assembly_label"sv},
        {element_type_t::type_reference, "type_reference"sv},
        {element_type_t::spread_operator, "spread_operator"sv},
        {element_type_t::label_reference, "label_reference"sv},
        {element_type_t::boolean_literal, "boolean_literal"sv},
        {element_type_t::integer_literal, "integer_literal"sv},
        {element_type_t::binary_operator, "binary_operator"sv},
        {element_type_t::module_reference, "module_reference"sv},
        {element_type_t::character_literal, "character_literal"sv},
        {element_type_t::value_sink_literal, "value_sink_literal"sv},
        {element_type_t::unknown_identifier, "unknown_identifier"sv},
        {element_type_t::identifier_reference, "identifier_reference"sv},
        {element_type_t::uninitialized_literal, "uninitialized_literal"sv},
        {element_type_t::assembly_literal_label, "assembly_literal_label"sv},
    };

    static inline std::string_view element_type_name(element_type_t type) {
        auto it = s_element_type_names.find(type);
        if (it == s_element_type_names.end()) {
            return "unknown"sv;
        }
        return it->second;
    }

    using element_type_set_t = std::set<element_type_t>;

    ///////////////////////////////////////////////////////////////////////////

    enum class comment_type_t {
        line = 1,
        block
    };

    static inline std::string_view comment_type_name(comment_type_t type) {
        switch (type) {
            case comment_type_t::line:  return "line"sv;
            case comment_type_t::block: return "block"sv;
        }
        return "unknown"sv;
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

    static inline std::unordered_map<operator_type_t, std::string_view> s_operator_type_names = {
        {operator_type_t::unknown,               "unknown"sv},
        {operator_type_t::negate,                "negate"sv},
        {operator_type_t::binary_not,            "binary_not"sv},
        {operator_type_t::logical_not,           "logical_not"sv},
        {operator_type_t::subscript,             "subscript"sv},
        {operator_type_t::pointer_dereference,   "pointer_deference"sv},
        {operator_type_t::add,                   "add"sv},
        {operator_type_t::subtract,              "subtract"sv},
        {operator_type_t::multiply,              "multiply"sv},
        {operator_type_t::divide,                "divide"sv},
        {operator_type_t::modulo,                "modulo"sv},
        {operator_type_t::equals,                "equals"sv},
        {operator_type_t::not_equals,            "not_equals"sv},
        {operator_type_t::greater_than,          "greater_than"sv},
        {operator_type_t::less_than,             "less_than"sv},
        {operator_type_t::greater_than_or_equal, "greater_than_or_equal"sv},
        {operator_type_t::less_than_or_equal,    "less_than_or_equal"sv},
        {operator_type_t::logical_or,            "logical_or"sv},
        {operator_type_t::logical_and,           "logical_and"sv},
        {operator_type_t::binary_or,             "binary_or"sv},
        {operator_type_t::binary_and,            "binary_and"sv},
        {operator_type_t::binary_xor,            "binary_xor"sv},
        {operator_type_t::shift_right,           "shift_right"sv},
        {operator_type_t::shift_left,            "shift_left"sv},
        {operator_type_t::rotate_right,          "rotate_right"sv},
        {operator_type_t::rotate_left,           "rotate_left"sv},
        {operator_type_t::exponent,              "exponent"sv},
        {operator_type_t::assignment,            "assignment"sv},
        {operator_type_t::member_access,         "member_access"sv},
    };

    static inline std::string_view operator_type_name(operator_type_t type) {
        auto it = s_operator_type_names.find(type);
        if (it == s_operator_type_names.end())
            return "unknown"sv;
        return it->second;
    }
    
    static inline std::unordered_map<syntax::token_type_t, operator_type_t> s_unary_operators = {
        {syntax::token_type_t::minus, operator_type_t::negate},
        {syntax::token_type_t::tilde, operator_type_t::binary_not},
        {syntax::token_type_t::bang,  operator_type_t::logical_not},
        {syntax::token_type_t::caret, operator_type_t::pointer_dereference},
    };

    static inline std::unordered_map<syntax::token_type_t, operator_type_t> s_binary_operators = {
        {syntax::token_type_t::plus,               operator_type_t::add},
        {syntax::token_type_t::minus,              operator_type_t::subtract},
        {syntax::token_type_t::asterisk,           operator_type_t::multiply},
        {syntax::token_type_t::slash,              operator_type_t::divide},
        {syntax::token_type_t::percent,            operator_type_t::modulo},
        {syntax::token_type_t::equals,             operator_type_t::equals},
        {syntax::token_type_t::not_equals,         operator_type_t::not_equals},
        {syntax::token_type_t::greater_than,       operator_type_t::greater_than},
        {syntax::token_type_t::less_than,          operator_type_t::less_than},
        {syntax::token_type_t::greater_than_equal, operator_type_t::greater_than_or_equal},
        {syntax::token_type_t::less_than_equal,    operator_type_t::less_than_or_equal},
        {syntax::token_type_t::not_equals,         operator_type_t::not_equals},
        {syntax::token_type_t::logical_or,         operator_type_t::logical_or},
        {syntax::token_type_t::logical_and,        operator_type_t::logical_and},
        {syntax::token_type_t::pipe,               operator_type_t::binary_or},
        {syntax::token_type_t::ampersand,          operator_type_t::binary_and},
        {syntax::token_type_t::xor_literal,        operator_type_t::binary_xor},
        {syntax::token_type_t::shl_literal,        operator_type_t::shift_left},
        {syntax::token_type_t::shr_literal,        operator_type_t::shift_right},
        {syntax::token_type_t::rol_literal,        operator_type_t::rotate_left},
        {syntax::token_type_t::ror_literal,        operator_type_t::rotate_right},
        {syntax::token_type_t::exponent,           operator_type_t::exponent},
        {syntax::token_type_t::assignment,         operator_type_t::assignment},
        {syntax::token_type_t::period,             operator_type_t::member_access},
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
        inline bool empty() const {
            return _attrs.empty();
        }

        inline size_t size() const {
            return _attrs.size();
        }

        void add(attribute* value);

        attribute_list_t as_list() const;

        bool remove(const std::string& name);

        compiler::attribute* find(const std::string_view& name) const;

    private:
        std::unordered_map<std::string_view, attribute*> _attrs {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct field_map_t {
        void add(field* value);

        inline bool empty() const {
            return _fields.empty();
        }

        inline size_t size() const {
            return _fields.size();
        }

        size_t size_in_bytes() const;

        field_list_t as_list() const;

        bool remove(common::id_t id);

        compiler::field* find(common::id_t id);

        compiler::field* find_by_name(const std::string_view& name);

    private:
        std::map<common::id_t, field*> _fields {};
    };

    size_t count_anonymous_return_parameters(const field_map_t& fields);

    ///////////////////////////////////////////////////////////////////////////

    struct identifier_map_t {
        void dump();

        void add(identifier* value);

        inline bool empty() const {
            return _identifiers.empty();
        }

        inline size_t size() const {
            return _identifiers.size();
        }

        identifier_list_t as_list() const;

        bool remove(const std::string_view& name);

        identifier_list_t find(const std::string_view& name);

    private:
        std::unordered_multimap<std::string_view, identifier*> _identifiers {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct reference_flow_t {
        enum flags_t : uint8_t {
            none = 0b00000000,
            in   = 0b00000001,
            out  = 0b00000010,
        };

        using flag_t = uint8_t;

        flag_t flags {};
        common::id_t id {};
    };

    struct reference_map_t {
        bool empty() const {
            return _references.empty();
        }

        size_t size() const {
            return _references.size();
        }

        element_id_set_t as_list() const;

        element_id_set_t in_list() const;

        element_id_set_t out_list() const;

        bool add(identifier_reference* value);

        bool remove(identifier_reference* value);

    private:
        std::map<common::id_t, reference_flow_t> _references {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct type_map_t {
        void add(
            compiler::symbol_element* symbol,
            compiler::type* type);

        bool empty() const {
            return _types.empty();
        }

        size_t size() const {
            return _types.size();
        }

        type_list_t as_list() const;

        void add(compiler::type* type);

        string_view_list_t name_list() const;

        bool remove(const std::string_view& name);

        compiler::type* find(const std::string_view& name);

    private:
        std::unordered_map<std::string_view, type*> _types {};
    };

    ///////////////////////////////////////////////////////////////////////////

    struct qualified_symbol_t {
        qualified_symbol_t() = default;

        explicit qualified_symbol_t(std::string_view name) : name(name) {
        }

        bool is_qualified() const {
            return !namespaces.empty();
        }
        std::string_view name {};
        string_view_list_t namespaces {};
        common::source_location location {};
        std::string fully_qualified_name {};
    };

    qualified_symbol_t make_qualified_symbol(const std::string_view& symbol);

    std::string make_fully_qualified_name(const symbol_element* symbol);

    std::string make_fully_qualified_name(const qualified_symbol_t& symbol);

    ///////////////////////////////////////////////////////////////////////////

    class element_builder;

    ///////////////////////////////////////////////////////////////////////////

    struct fold_result_t {
        bool allow_no_fold_attribute = true;
        compiler::element* element = nullptr;
        compiler::type_reference* type_ref = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    struct inferred_type_t {
        std::string_view type_name() const;

        explicit inferred_type_t(
            compiler::type* t,
            compiler::type_reference* r = nullptr) : type(t),
                                                     ref(r) {
        }

        compiler::type* type = nullptr;
        compiler::type_reference* ref = nullptr;
    };

    using inferred_type_list_t = std::vector<inferred_type_t>;

    struct infer_type_result_t {
        inferred_type_list_t types {};
    };

    ///////////////////////////////////////////////////////////////////////////

    static const constexpr uint16_t switch_expression = 1;
    static const constexpr uint16_t previous_element = 2;
    static const constexpr uint16_t next_element = 3;

    ///////////////////////////////////////////////////////////////////////////

    using argument_index_map_t = std::map<std::string, size_t>;

    struct prepare_call_site_result_t {
        common::result messages {};
        element_list_t arguments {};
        argument_index_map_t index {};
        compiler::procedure_type* proc_type = nullptr;
        compiler::identifier_reference* ref = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////

    struct type_check_options_t {
        bool strict = true;
    };

    ///////////////////////////////////////////////////////////////////////////

    union native_integer_t {
        int8_t sb;
        uint8_t b;

        int16_t sw;
        uint16_t w;

        int32_t sdw;
        uint32_t dw;

        int64_t sqw;
        uint64_t qw;
    };

}