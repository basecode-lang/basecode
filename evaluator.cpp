#include <fstream>
#include <iostream>
#include "evaluator.h"
#include "symbol_table.h"
#include "alpha_compiler.h"

namespace basecode {

    evaluator::evaluator(alpha_compiler* compiler) : _compiler(compiler) {
    }

    void evaluator::error(
            result& result,
            const std::string& code,
            const std::string& message) {
        result.add_message(code, message, true);
        result.fail();
    }

    bool evaluator::transform_program(
            result& result,
            const ast_node_shared_ptr& node) {
        if (node == nullptr)
            return false;

        switch (node->token) {
            case ast_node_t::comment: {
                // XXX: remove from ast
                break;
            }
            case ast_node_t::basic_block: {
                for (auto const& block_child : node->children) {
                    transform_program(result, block_child);
                }
                break;
            }
            case ast_node_t::unary_op:
            case ast_node_t::binary_op:
            case ast_node_t::statement:
            case ast_node_t::expression:
            case ast_node_t::null_literal:
            case ast_node_t::number_literal:
            case ast_node_t::string_literal:
            case ast_node_t::boolean_literal:
            case ast_node_t::character_literal:
                // call evaluate
                break;
            case ast_node_t::label:
            case ast_node_t::identifier: {
                transform_identifier_node(result, node);
                break;
            }
            case ast_node_t::branch: {
                // N.B. currently a nop
                break;
            }
            default:
                error(result, "E001", "undefined ast node");
                break;
        }

        return !result.is_failed();
    }

    bool evaluator::evaluate_program(
            result& result,
            const ast_node_shared_ptr& program_node) {
        if (_compiler == nullptr) {
            return false;
        }

        if (program_node == nullptr) {
            // XXX: add error
            return false;
        }

        for (const auto& program_child : program_node->children) {
            if (!transform_program(result, program_child)) {
                break;
            }
        }

        return !result.is_failed();
    }

    variant_t evaluator::evaluate(
            result& result,
            const ast_node_shared_ptr& node) {
        if (node == nullptr)
            return {};

        switch (node->token) {
            case ast_node_t::assignment:
                break;
            case ast_node_t::comment:
            case ast_node_t::null_literal:
            case ast_node_t::string_literal:
            case ast_node_t::boolean_literal:
            case ast_node_t::character_literal:
                return node->value;
            case ast_node_t::label:
            case ast_node_t::identifier: {
                if (_symbol_table == nullptr) {
                    error(result, "E006", "no symbol table assigned");
                    break;
                }
                auto identifier = node->identifier_type();
                auto value = _symbol_table->get(identifier.value);
                if (value == nullptr && _symbol_table->missing_is_error()) {
                    error(result, "E006", fmt::format("unknown identifier: {}", identifier.value));
                } else {
                    return evaluate(result, value);
                }
                break;
            }
            case ast_node_t::address:
            case ast_node_t::number_literal: {
                switch (node->value.index()) {
                    case variant_meta_t::types::radix_numeric_literal: {
                        uint32_t out = 0;
                        auto value = node->radix_numeric_literal_type();
                        if (value.parse(out) != radix_numeric_literal_t::conversion_result::success) {
                            error(result, "E004", "numeric conversion error");
                            break;
                        }
                        return numeric_literal_t{out};
                    }
                    case variant_meta_t::types::char_literal: {
                        return numeric_literal_t {node->char_literal_type().value};
                    }
                    case variant_meta_t::types::numeric_literal: {
                        return node->value;
                    }
                    default: {
                        error(result, "E004", "numeric conversion error");
                        break;
                    }
                }
                break;
            }
            case ast_node_t::branch:
            case ast_node_t::program:
            case ast_node_t::statement:
            case ast_node_t::basic_block:
            case ast_node_t::parameter_list:
            case ast_node_t::uninitialized_literal: {
                break;
            }
            case ast_node_t::expression: {
                return evaluate(result, node->children[0]);
            }
            case ast_node_t::binary_op: {
                auto op = node->operator_type();
                switch (op.group) {
                    case operator_t::op_group::conversion: {
                        switch (op.op) {
                            default: {
                                error(
                                    result,
                                    "E008",
                                    fmt::format("operator {} is not supported", op.symbol));
                                break;
                            }
                        }
                        break;
                    }
                    case operator_t::op_group::relational:
                    case operator_t::op_group::arithmetic: {
                        numeric_literal_t lhs;
                        numeric_literal_t rhs;

                        auto lhs_node = evaluate(result, node->lhs);
                        auto rhs_node = evaluate(result, node->rhs);

                        if (lhs_node.index() == variant_meta_t::types::boolean_literal) {
                            lhs = numeric_literal_t {
                                    static_cast<uint32_t>(std::get<boolean_literal_t>(lhs_node).value ? 1 : 0)
                            };
                        } else if (lhs_node.index() == variant_meta_t::types::numeric_literal) {
                            lhs = std::get<numeric_literal_t>(lhs_node);
                        } else if(lhs_node.index() == variant_meta_t::types::char_literal) {
                            lhs = numeric_literal_t {std::get<char_literal_t>(rhs_node).value};
                        } else {
                            error(result,
                                  "E005",
                                  "only integer values can be used with arithmetic/relational operators");
                        }
                        if (rhs_node.index() == variant_meta_t::types::boolean_literal) {
                            rhs = numeric_literal_t {
                                static_cast<uint32_t>(std::get<boolean_literal_t>(rhs_node).value ? 1 : 0)
                            };
                        } else if (rhs_node.index() == variant_meta_t::types::numeric_literal) {
                            rhs = std::get<numeric_literal_t>(rhs_node);
                        } else if(rhs_node.index() == variant_meta_t::types::char_literal) {
                            rhs = numeric_literal_t {std::get<char_literal_t>(rhs_node).value};
                        } else {
                            error(result,
                                  "E005",
                                  "only integer values can be used with arithmetic/relational operators");
                        }

                        if (result.is_failed())
                            break;

                        switch (op.op) {
                            case operator_t::op::add:                   return lhs + rhs;
                            case operator_t::op::subtract:              return lhs - rhs;
                            case operator_t::op::multiply:              return lhs * rhs;
                            case operator_t::op::divide:                return lhs / rhs;
                            case operator_t::op::modulo:                return lhs % rhs;
                            case operator_t::op::binary_and:            return lhs & rhs;
                            case operator_t::op::binary_or:             return lhs | rhs;
                            case operator_t::op::pow:                   return lhs ^ rhs;
                            case operator_t::op::less_than:             return lhs < rhs;
                            case operator_t::op::less_than_equal:       return lhs <= rhs;
                            case operator_t::op::equal:                 return lhs == rhs;
                            case operator_t::op::not_equal:             return lhs != rhs;
                            case operator_t::op::greater_than:          return lhs > rhs;
                            case operator_t::op::greater_than_equal:    return lhs >= rhs;
                            case operator_t::op::shift_left:            return lhs << rhs;
                            case operator_t::op::shift_right:           return lhs >> rhs;
                            default:
                                error(result, "E008", fmt::format("operator {} is not supported", op.symbol));
                                break;
                        }
                        break;
                    }
                    case operator_t::op_group::logical: {
                        boolean_literal_t lhs;
                        boolean_literal_t rhs;

                        auto lhs_node = evaluate(result, node->lhs);
                        auto rhs_node = evaluate(result, node->rhs);

                        if (lhs_node.index() == variant_meta_t::types::numeric_literal) {
                            lhs = boolean_literal_t {std::get<numeric_literal_t>(lhs_node).value == 1};
                        } else if (lhs_node.index() == variant_meta_t::types::boolean_literal) {
                            lhs = std::get<boolean_literal_t>(lhs_node);
                        } else {
                            error(result,
                                  "E005",
                                  "only boolean values can be used with logical operators");
                        }
                        if (rhs_node.index() == variant_meta_t::types::numeric_literal) {
                            rhs = boolean_literal_t {std::get<numeric_literal_t>(rhs_node).value == 1};
                        } else if (rhs_node.index() == variant_meta_t::types::boolean_literal) {
                            rhs = std::get<boolean_literal_t>(rhs_node);
                        } else {
                            error(result,
                                  "E005",
                                  "only boolean values can be used with logical operators");
                        }

                        if (result.is_failed())
                            break;

                        switch (op.op) {
                            case operator_t::op::equal:       return lhs == rhs;
                            case operator_t::op::not_equal:   return lhs == rhs;
                            case operator_t::op::logical_or:  return lhs || rhs;
                            case operator_t::op::logical_and: return lhs && rhs;
                            default:
                                error(result, "E008", fmt::format("operator {} is not supported", op.symbol));
                                break;
                        }
                        break;
                    }
                    default: {
                        error(result, "E008", "unknown binary operator group");
                        break;
                    }
                }
                break;
            }
            case ast_node_t::unary_op: {
                auto op = std::get<operator_t>(node->value);
                switch (op.group) {
                    case operator_t::conversion: {
                        break;
                    }
                    case operator_t::arithmetic: {
                        auto rhs_node = evaluate(result, node->rhs);
                        numeric_literal_t rhs;
                        if (rhs_node.index() == variant_meta_t::types::boolean_literal) {
                            rhs = numeric_literal_t {
                                static_cast<uint32_t>(std::get<boolean_literal_t>(rhs_node).value ? 1 : 0)
                            };
                        } else if (rhs_node.index() == variant_meta_t::types::numeric_literal) {
                            rhs = std::get<numeric_literal_t>(rhs_node);
                        } else {
                            error(result, "E005", "only integer values can be used with arithmetic operators");
                            break;
                        }
                        switch (op.op) {
                            case operator_t::op::negate: return -rhs;
                            case operator_t::op::invert: return ~rhs;
                            default:
                                error(result, "E008", fmt::format("operator {} is not supported", op.symbol));
                                break;
                        }
                        break;
                    }
                    case operator_t::logical: {
                        auto rhs_node = evaluate(result, node->rhs);
                        boolean_literal_t rhs;
                        if (rhs_node.index() == variant_meta_t::types::numeric_literal) {
                            rhs = boolean_literal_t {std::get<numeric_literal_t>(rhs_node).value == 1};
                        } else if (rhs_node.index() == variant_meta_t::types::boolean_literal) {
                            rhs = std::get<boolean_literal_t>(rhs_node);
                        } else {
                            error(result, "E005", "only boolean values can be used with logical operators");
                            break;
                        }
                        switch (op.op) {
                            case operator_t::op::logical_not: return !rhs;
                            default:
                                error(result, "E008", fmt::format("operator {} is not supported", op.symbol));
                                break;
                        }
                        break;
                    }
                    default:
                        error(result, "E008", "unknown unary operator group");
                        break;
                }
                break;
            }
        }

        return {};
    }

    bool evaluator::transform_identifier_node(
            result& result,
            const ast_node_shared_ptr& node) {
        if (node == nullptr)
            return false;

        std::string identifier_name;
        switch (node->token) {
            case ast_node_t::label:
                identifier_name = node->label_type().value;
                break;
            case ast_node_t::identifier:
                identifier_name = node->identifier_type().value;
                break;
            default:
                error(
                        result,
                        "E004",
                        "unknown assembler state; this should not happen");
                break;
        }

        auto address_node = std::make_shared<ast_node_t>();
        address_node->token = ast_node_t::tokens::address;
        address_node->value = numeric_literal_t {_compiler->address()};
        address_node->lhs = node;
//        _compiler
//                ->symbol_table()
//                ->put(identifier_name, address_node);

        return !result.is_failed();
    }

    basecode::symbol_table* evaluator::symbol_table() {
        return _symbol_table;
    }

    void evaluator::symbol_table(basecode::symbol_table* value) {
        _symbol_table = value;
    }

}