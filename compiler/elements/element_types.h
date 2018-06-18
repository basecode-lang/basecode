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

    using attribute_map_t = std::map<std::string, attribute*>;
    using directive_map_t = std::map<std::string, directive*>;
};