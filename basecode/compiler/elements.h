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

#include <compiler/elements/type.h>
#include <compiler/elements/cast.h>
#include <compiler/elements/with.h>
#include <compiler/elements/block.h>
#include <compiler/elements/label.h>
#include <compiler/elements/field.h>
#include <compiler/elements/import.h>
#include <compiler/elements/module.h>
#include <compiler/elements/comment.h>
#include <compiler/elements/program.h>
#include <compiler/elements/intrinsic.h>
#include <compiler/elements/raw_block.h>
#include <compiler/elements/bool_type.h>
#include <compiler/elements/attribute.h>
#include <compiler/elements/directive.h>
#include <compiler/elements/statement.h>
#include <compiler/elements/transmute.h>
#include <compiler/elements/rune_type.h>
#include <compiler/elements/assignment.h>
#include <compiler/elements/expression.h>
#include <compiler/elements/identifier.h>
#include <compiler/elements/if_element.h>
#include <compiler/elements/array_type.h>
#include <compiler/elements/tuple_type.h>
#include <compiler/elements/fallthrough.h>
#include <compiler/elements/for_element.h>
#include <compiler/elements/declaration.h>
#include <compiler/elements/initializer.h>
#include <compiler/elements/module_type.h>
#include <compiler/elements/nil_literal.h>
#include <compiler/elements/if_directive.h>
#include <compiler/elements/case_element.h>
#include <compiler/elements/type_literal.h>
#include <compiler/elements/numeric_type.h>
#include <compiler/elements/unknown_type.h>
#include <compiler/elements/pointer_type.h>
#include <compiler/elements/generic_type.h>
#include <compiler/elements/run_directive.h>
#include <compiler/elements/defer_element.h>
#include <compiler/elements/break_element.h>
#include <compiler/elements/while_element.h>
#include <compiler/elements/argument_list.h>
#include <compiler/elements/float_literal.h>
#include <compiler/elements/argument_pair.h>
#include <compiler/elements/type_directive.h>
#include <compiler/elements/switch_element.h>
#include <compiler/elements/copy_intrinsic.h>
#include <compiler/elements/fill_intrinsic.h>
#include <compiler/elements/type_reference.h>
#include <compiler/elements/string_literal.h>
#include <compiler/elements/unary_operator.h>
#include <compiler/elements/composite_type.h>
#include <compiler/elements/procedure_type.h>
#include <compiler/elements/return_element.h>
#include <compiler/elements/procedure_call.h>
#include <compiler/elements/namespace_type.h>
#include <compiler/elements/symbol_element.h>
#include <compiler/elements/assembly_label.h>
#include <compiler/elements/copy_intrinsic.h>
#include <compiler/elements/free_intrinsic.h>
#include <compiler/elements/spread_operator.h>
#include <compiler/elements/label_reference.h>
#include <compiler/elements/range_intrinsic.h>
#include <compiler/elements/alloc_intrinsic.h>
#include <compiler/elements/boolean_literal.h>
#include <compiler/elements/binary_operator.h>
#include <compiler/elements/integer_literal.h>
#include <compiler/elements/assert_directive.h>
#include <compiler/elements/continue_element.h>
#include <compiler/elements/module_reference.h>
#include <compiler/elements/foreign_directive.h>
#include <compiler/elements/type_of_intrinsic.h>
#include <compiler/elements/character_literal.h>
#include <compiler/elements/namespace_element.h>
#include <compiler/elements/size_of_intrinsic.h>
#include <compiler/elements/align_of_intrinsic.h>
#include <compiler/elements/procedure_instance.h>
#include <compiler/elements/assembly_directive.h>
#include <compiler/elements/length_of_intrinsic.h>
#include <compiler/elements/core_type_directive.h>
#include <compiler/elements/intrinsic_directive.h>
#include <compiler/elements/identifier_reference.h>
#include <compiler/elements/address_of_intrinsic.h>
#include <compiler/elements/uninitialized_literal.h>
#include <compiler/elements/assembly_literal_label.h>
