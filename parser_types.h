#pragma once

#include <map>
#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include <variant>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace basecode {

    class result;

    struct ast_node_t;

    using ast_node_shared_ptr = std::shared_ptr<ast_node_t>;
    using ast_node_list = std::vector<ast_node_shared_ptr>;
    using symbol_dict = std::map<std::string, ast_node_shared_ptr>;

    struct variant_meta_t {
        enum types {
            radix_numeric_literal = 0,
            numeric_literal,
            boolean_literal,
            identifier,
            string_literal,
            char_literal,
            operator_literal,
            comment_literal,
            directive,
            label,
            dup_literal,

            // N.B. these must always be the last values
            variadic,
            any
        };

        static std::string to_string(types value) {
            switch (value) {
                case radix_numeric_literal: return "radix_numeric_literal";
                case numeric_literal:       return "numeric_literal";
                case boolean_literal:       return "boolean_literal";
                case identifier:            return "identifier";
                case string_literal:        return "string_literal";
                case char_literal:          return "char_literal";
                case operator_literal:      return "operator_literal";
                case comment_literal:       return "comment_literal";
                case directive:             return "directive";
                case label:                 return "label";
                case variadic:              return "variadic";
                case dup_literal:           return "dup_literal";
                case any:                   return "any";
            }
            return "unknown";
        }
    };

    struct scanner_pos_t {
        uint32_t line;
        size_t index;
        uint32_t column;
    };

    using custom_parser_callable = std::function<ast_node_shared_ptr ()>;

    struct operator_t {
        enum op : uint16_t {
            invert,
            negate,
            pow,
            multiply,
            divide,
            modulo,
            add,
            subtract,
            binary_and,
            binary_or,
            greater_than,
            greater_than_equal,
            less_than,
            less_than_equal,
            equal,
            not_equal,
            logical_and,
            logical_or,
            logical_not,
            left_parenthesis,
            right_parenthesis,
            left_bracket,
            right_bracket,
            shift_left,
            shift_right
        };

        enum op_type : uint8_t {
            no_op = 0b00000000,
            unary = 0b00000001,
            binary= 0b00000010
        };

        enum op_group : uint8_t {
            arithmetic,
            relational,
            grouping,
            logical,
            conversion
        };

        enum associativity_type {
            none = 1,
            right,
            left
        };

        bool not_right_associative() {
            return type != associativity_type::right;
        }

        int compare_precedence(const operator_t& other) {
            return precedence > other.precedence ? 1 : other.precedence == precedence ? 0 : -1;
        }

        friend std::ostream& operator<<(std::ostream& stream, const operator_t& op) {
            stream << op.symbol;
            return stream;
        }

        uint16_t op;
        std::string symbol;
        uint8_t precedence = 0;
        uint8_t type = op_type::no_op;
        associativity_type associativity = associativity_type::none;
        op_group group = op_group::arithmetic;
        custom_parser_callable custom_parser;
    };

    struct radix_numeric_literal_t {
        uint8_t radix = 0;
        std::string value;

        enum conversion_result {
            success,
            overflow,
            underflow,
            inconvertible
        };

        conversion_result parse(uint32_t& out) const {
            const char* s = value.c_str();
            char* end;
            long l;
            errno = 0;
            l = strtol(s, &end, radix);
            if ((errno == ERANGE && l == LONG_MAX) || l > UINT_MAX) {
                return overflow;
            }
            if ((errno == ERANGE && l == LONG_MIN)) {
                return underflow;
            }
            if (*s == '\0' || *end != '\0') {
                return inconvertible;
            }
            out = static_cast<uint32_t>(l);
            return success;
        }

        friend std::ostream& operator<<(std::ostream& stream, const radix_numeric_literal_t& lit) {
            switch (lit.radix) {
                case 2:
                    stream << fmt::format("%{0}", lit.value);
                    break;
                case 8:
                    stream << fmt::format("@{0}", lit.value);
                    break;
                case 16:
                    stream << fmt::format("${0}", lit.value);
                    break;
                default:
                    stream << fmt::format("{0}", lit.value);
                    break;
            }
            return stream;
        }
    };

    struct comment_t {
        std::string value {};

        inline bool empty() const {
            return value.empty();
        }

        operator std::string() {
            return value;
        }

        operator std::string() const {
            return value;
        }

        friend std::ostream& operator<<(
                std::ostream& stream,
                const comment_t& comment) {
            stream << "// " << comment.value;
            return stream;
        }
    };

    struct identifier_t {
        std::string value {};

        inline bool empty() const {
            return value.empty();
        }

        operator std::string() {
            return value;
        }

        operator std::string() const {
            return value;
        }

        friend std::ostream& operator<<(
                std::ostream& stream,
                const identifier_t& identifier) {
            stream << identifier.value;
            return stream;
        }
    };

    struct label_t {
        std::string value {};

        inline bool empty() const {
            return value.empty();
        }

        operator std::string() {
            return value;
        }

        operator std::string() const {
            return value;
        }

        friend std::ostream& operator<<(
                std::ostream& stream,
                const label_t& label) {
            stream << label.value << ":";
            return stream;
        }
    };

    struct string_literal_t {
        std::string value {};

        inline bool empty() const {
            return value.empty();
        }

        operator std::string() {
            return value;
        }

        operator std::string() const {
            return value;
        }

        friend std::ostream& operator<<(
                std::ostream& stream,
                const string_literal_t& lit) {
            stream << "\"" << lit.value << "\"";
            return stream;
        }
    };

    struct boolean_literal_t {
        bool value {};

        operator bool() {
            return value;
        }
        operator bool() const {
            return value;
        }
        operator uint32_t() {
            return value ? 1 : 0;
        }
        operator uint32_t() const {
            return value ? 1 : 0;
        }
        boolean_literal_t operator! () {
            return boolean_literal_t {!value};
        }
        boolean_literal_t operator== (const boolean_literal_t& other) {
            return boolean_literal_t {value == other.value};
        }
        boolean_literal_t operator!= (const boolean_literal_t& other) {
            return boolean_literal_t {value != other.value};
        }
        boolean_literal_t operator&& (const boolean_literal_t& other) {
            return boolean_literal_t {value && other.value};
        }
        boolean_literal_t operator|| (const boolean_literal_t& other) {
            return boolean_literal_t {value || other.value};
        }
        friend std::ostream& operator<<(std::ostream& stream, const boolean_literal_t& lit) {
            stream << std::boolalpha << lit.value;
            return stream;
        }
    };

    struct numeric_literal_t {
        uint64_t value {};

        operator uint64_t() {
            return value;
        }
        operator uint64_t() const {
            return value;
        }
        numeric_literal_t operator~ () {
            return numeric_literal_t {~value};
        }
        numeric_literal_t operator- () {
            return numeric_literal_t {
                static_cast<uint64_t>(-value)
            };
        }
        numeric_literal_t operator+ (const numeric_literal_t& other) {
            return numeric_literal_t {value + other.value};
        }
        numeric_literal_t operator- (const numeric_literal_t& other) {
            return numeric_literal_t {value - other.value};
        }
        numeric_literal_t operator* (const numeric_literal_t& other) {
            return numeric_literal_t {value * other.value};
        }
        numeric_literal_t operator/ (const numeric_literal_t& other) {
            return numeric_literal_t {value / other.value};
        }
        numeric_literal_t operator% (const numeric_literal_t& other) {
            return numeric_literal_t {value % other.value};
        }
        numeric_literal_t operator& (const numeric_literal_t& other) {
            return numeric_literal_t {value & other.value};
        }
        numeric_literal_t operator| (const numeric_literal_t& other) {
            return numeric_literal_t {value | other.value};
        }
        numeric_literal_t operator^ (const numeric_literal_t& other) {
            return numeric_literal_t {value ^ other.value};
        }
        numeric_literal_t operator<< (const numeric_literal_t& other) {
            return numeric_literal_t {value << other.value};
        }
        numeric_literal_t operator>> (const numeric_literal_t& other) {
            return numeric_literal_t {value >> other.value};
        }
        boolean_literal_t operator< (const numeric_literal_t& other) {
            return boolean_literal_t {value < other.value};
        }
        boolean_literal_t operator<= (const numeric_literal_t& other) {
            return boolean_literal_t {value <= other.value};
        }
        boolean_literal_t operator== (const numeric_literal_t& other) {
            return boolean_literal_t {value == other.value};
        }
        boolean_literal_t operator!= (const numeric_literal_t& other) {
            return boolean_literal_t {value != other.value};
        }
        boolean_literal_t operator> (const numeric_literal_t& other) {
            return boolean_literal_t {value > other.value};
        }
        boolean_literal_t operator>= (const numeric_literal_t& other) {
            return boolean_literal_t {value >= other.value};
        }
        friend std::ostream& operator<<(
                std::ostream& stream,
                const numeric_literal_t& lit) {
            stream << lit.value;
            return stream;
        }
    };

    struct char_literal_t {
        unsigned char value {};

        operator unsigned char() {
            return value;
        }
        operator unsigned char() const {
            return value;
        }
        operator numeric_literal_t() {
            return numeric_literal_t {value};
        }
        operator numeric_literal_t() const {
            return numeric_literal_t {value};
        }
        boolean_literal_t operator== (const char_literal_t& other) {
            return boolean_literal_t {value == other.value};
        }
        boolean_literal_t operator!= (const char_literal_t& other) {
            return boolean_literal_t {value != other.value};
        }
        boolean_literal_t operator< (const char_literal_t& other) {
            return boolean_literal_t {value < other.value};
        }
        boolean_literal_t operator<= (const char_literal_t& other) {
            return boolean_literal_t {value <= other.value};
        }
        boolean_literal_t operator> (const char_literal_t& other) {
            return boolean_literal_t {value > other.value};
        }
        boolean_literal_t operator>= (const char_literal_t& other) {
            return boolean_literal_t {value >= other.value};
        }
        friend std::ostream& operator<<(
                std::ostream& stream,
                const char_literal_t& lit) {
            stream << "'" << lit.value << "'";
            return stream;
        }
    };

    struct dup_literal_t {
        uint32_t count;
        std::vector<numeric_literal_t> values;

        friend std::ostream& operator<<(
                std::ostream& stream,
                const dup_literal_t& lit) {
            return stream;
        }
    };

    typedef std::map<std::string, operator_t> operator_dict;

    typedef std::variant<
        radix_numeric_literal_t,
        numeric_literal_t,
        boolean_literal_t,
        identifier_t,
        string_literal_t,
        char_literal_t,
        operator_t,
        comment_t,
        label_t,
        dup_literal_t> variant_t;

    struct ast_node_t {
        enum tokens {
            program,
            comment,
            basic_block,
            statement,
            expression,
            binary_op,
            unary_op,
            identifier,
            label,
            string_literal,
            character_literal,
            number_literal,
            null_literal,
            boolean_literal,
            parameter_list,
            branch,
            address,
            uninitialized_literal,
            assignment
        };

        void serialize(std::ostream& stream) {
            switch (token) {
                case assignment:
                    stream << ":=";
                    break;
                case branch:
                    lhs->serialize(stream);
                    if (rhs != nullptr)
                        rhs->serialize(stream);
                    break;
                case program:
                case basic_block:
                    for (const auto& child : children)
                        child->serialize(stream);
                    break;
                case parameter_list: {
                    auto child_count = children.size();
                    for (size_t i = 0; i < child_count; i++) {
                        const auto& child = children[i];
                        child->serialize(stream);
                        if (i < child_count - 1)
                            stream << ",";
                    }
                    break;
                }
                case expression:
                    stream << "(";
                    for (const auto& child : children)
                        child->serialize(stream);
                    stream << ")";
                    break;
                case binary_op:
                    lhs->serialize(stream);
                    stream << std::get<operator_t>(value);
                    rhs->serialize(stream);
                    break;
                case unary_op:
                    stream << std::get<operator_t>(value);
                    rhs->serialize(stream);
                    break;
                case statement:
                case label:
                case identifier:
                    stream << std::get<identifier_t>(value);
                    break;
                case address:
                case string_literal:
                case number_literal:
                case boolean_literal:
                case character_literal:
                    stream << std::get<string_literal_t>(value);
                    break;
                case comment:
                    stream << std::get<comment_t>(value);
                    break;
                case null_literal:
                    stream << "null";
                    break;
                case uninitialized_literal:
                    stream << "?";
                    break;
            }
        }

        label_t label_type() const {
            return std::get<label_t>(value);
        }

        operator_t operator_type() const {
            return std::get<operator_t>(value);
        }

        identifier_t identifier_type() const {
            return std::get<identifier_t>(value);
        }

        char_literal_t char_literal_type() const {
            return std::get<char_literal_t>(value);
        }

        string_literal_t string_literal_type() const {
            return std::get<string_literal_t>(value);
        }

        boolean_literal_t boolean_literal_type() const {
            return std::get<boolean_literal_t>(value);
        }

        numeric_literal_t numeric_literal_type() const {
            return std::get<numeric_literal_t>(value);
        }

        radix_numeric_literal_t radix_numeric_literal_type() const {
            return std::get<radix_numeric_literal_t>(value);
        }

        tokens token;
        variant_t value;
        ast_node_list children;
        ast_node_shared_ptr lhs = nullptr;
        ast_node_shared_ptr rhs = nullptr;
        ast_node_shared_ptr parent = nullptr;
        uint32_t line = 0;
        uint32_t column = 0;
    };

    typedef std::vector<variant_t> variant_list;

    struct parser_input_t {
        static void split_to_lines(
                const std::string& source,
                std::vector<std::string>& source_lines) {
            source_lines.clear();
            std::stringstream stream;
            stream << source << "\n";
            std::string line;
            while (std::getline(stream, line)) {
                source_lines.push_back(line);
            }
        }

        parser_input_t() : source("") {
        }

        explicit parser_input_t(const std::string& source) {
            this->source = source;
            split_to_lines(this->source, this->source_lines);
        }

        inline bool empty() const {
            return source.empty();
        }

        inline size_t length() const {
            return source.length();
        }

        inline std::string& operator[](size_t index) {
            assert(index >= 0 && index < source_lines.size());
            return source_lines[index];
        }

        inline const std::string& operator[](size_t index) const {
            assert(index >= 0 && index < source_lines.size());
            return source_lines[index];
        }

        std::string source;
        std::vector<std::string> source_lines {};
    };

};