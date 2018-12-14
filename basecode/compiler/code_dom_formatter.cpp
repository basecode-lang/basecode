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

#include <fmt/format.h>
#include <compiler/elements.h>
#include <common/graphviz_formatter.h>
#include "code_dom_formatter.h"

namespace basecode::compiler {

    code_dom_formatter::code_dom_formatter(
            const compiler::session& session,
            FILE* output_file) : _file(output_file),
                                 _session(session) {
    }

    void code_dom_formatter::add_primary_edge(
            element* parent,
            element* child,
            const std::string& label) {
        if (parent == nullptr || child == nullptr)
            return;

        std::string attributes;
        if (!label.empty())
            attributes = fmt::format("[ label=\"{}\"; ]", label);

        _edges.insert(fmt::format(
            "{} -> {} {};",
            get_vertex_name(parent),
            get_vertex_name(child),
            attributes));
    }

    void code_dom_formatter::add_secondary_edge(
            element* parent,
            element* child,
            const std::string& label) {
        if (parent == nullptr || child == nullptr)
            return;

        std::string attributes = "[ style=dashed; ";
        if (!label.empty())
            attributes = fmt::format("label=\"{}\"; ", label);
        attributes += "]";

        _edges.insert(fmt::format(
            "{} -> {} {};",
            get_vertex_name(parent),
            get_vertex_name(child),
            attributes));
    }

    std::string code_dom_formatter::format_node(element* node) {
        auto node_vertex_name = get_vertex_name(node);

        for (auto attr : node->attributes().as_list())
            add_primary_edge(node, attr);

        for (auto comment : node->comments())
            add_primary_edge(node, comment);

        switch (node->element_type()) {
            case element_type_t::module: {
                auto element = dynamic_cast<module*>(node);
                auto style = ", fillcolor=grey, style=\"filled\"";
                add_primary_edge(element, element->scope());
                return fmt::format(
                    "{}[shape=record,label=\"module|{}\"{}];",
                    node_vertex_name,
                    element->source_file()->path().string(),
                    style);
            }
            case element_type_t::raw_block: {
                auto element = dynamic_cast<raw_block*>(node);
                auto style = ", fillcolor=grey, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"raw_block|{}\"{}];",
                    node_vertex_name,
                    element->value(),
                    style);
            }
            case element_type_t::module_reference: {
                auto element = dynamic_cast<module_reference*>(node);
                auto style = ", fillcolor=grey, style=\"filled\"";
                add_primary_edge(element, element->expression());
                add_primary_edge(element, element->module());
                return fmt::format(
                    "{}[shape=record,label=\"module_reference\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::symbol: {
                auto element = dynamic_cast<symbol_element*>(node);
                auto style = ", fillcolor=pink, style=\"filled\"";
                for (auto type_param_ref : element->type_parameters())
                    add_primary_edge(element, type_param_ref);
                return fmt::format(
                    "{}[shape=record,label=\"symbol|{}\"{}];",
                    node_vertex_name,
                    element->fully_qualified_name(),
                    style);
            }
            case element_type_t::comment: {
                auto comment_element = dynamic_cast<comment*>(node);
                auto style = ", fillcolor=green, style=\"filled\"";
                auto details = fmt::format(
                    "comment|{{type: {} | value: '{}' }}",
                    comment_type_name(comment_element->type()),
                    common::graphviz_formatter::escape_chars(comment_element->value()));
                return fmt::format(
                    "{}[shape=record,label=\"{}\"{}];",
                    node_vertex_name,
                    details,
                    style);
            }
            case element_type_t::cast: {
                auto element = dynamic_cast<cast*>(node);
                auto style = ", fillcolor=deeppink, style=\"filled\"";
                add_primary_edge(element, element->expression());
                return fmt::format(
                    "{}[shape=record,label=\"cast|{}\"{}];",
                    node_vertex_name,
                    element->type()->symbol().name,
                    style);
            }
            case element_type_t::case_e: {
                auto element = dynamic_cast<case_element*>(node);
                auto style = ", fillcolor=deeppink, style=\"filled\"";
                add_primary_edge(element, element->expression());
                add_primary_edge(element, element->scope());
                return fmt::format(
                    "{}[shape=record,label=\"case\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::switch_e: {
                auto element = dynamic_cast<switch_element*>(node);
                auto style = ", fillcolor=purple, style=\"filled\"";
                add_primary_edge(element, element->expression());
                add_primary_edge(element, element->scope());
                return fmt::format(
                    "{}[shape=record,label=\"switch\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::fallthrough: {
                auto element = dynamic_cast<fallthrough*>(node);
                auto style = ", fillcolor=red, style=\"filled\"";
                add_primary_edge(element, element->label());
                return fmt::format(
                    "{}[shape=record,label=\"fallthrough\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::transmute: {
                auto element = dynamic_cast<transmute*>(node);
                auto style = ", fillcolor=deeppink, style=\"filled\"";
                add_primary_edge(element, element->expression());
                return fmt::format(
                    "{}[shape=record,label=\"transmute|{}\"{}];",
                    node_vertex_name,
                    element->type()->symbol().name,
                    style);
            }
            case element_type_t::if_e: {
                auto element = dynamic_cast<if_element*>(node);
                auto style = ", fillcolor=cornsilk1, style=\"filled\"";
                add_primary_edge(element, element->predicate(), "predicate");
                add_primary_edge(element, element->true_branch(), "true");
                add_primary_edge(element, element->false_branch(), "false");
                return fmt::format(
                    "{}[shape=record,label=\"{}\"{}];",
                    node_vertex_name,
                    element->is_else_if() ? "else if" : "if",
                    style);
            }
            case element_type_t::break_e: {
                auto element = dynamic_cast<break_element*>(node);
                auto style = ", fillcolor=cornsilk4, style=\"filled\"";
                if (element->label() != nullptr)
                    add_primary_edge(element, element->label());
                return fmt::format(
                    "{}[shape=record,label=\"break\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::continue_e: {
                auto element = dynamic_cast<continue_element*>(node);
                auto style = ", fillcolor=cornsilk4, style=\"filled\"";
                if (element->label() != nullptr)
                    add_primary_edge(element, element->label());
                return fmt::format(
                    "{}[shape=record,label=\"continue\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::while_e: {
                auto element = dynamic_cast<while_element*>(node);
                auto style = ", fillcolor=cornsilk3, style=\"filled\"";
                add_primary_edge(element, element->predicate(), "predicate");
                add_primary_edge(element, element->body(), "body");
                return fmt::format(
                    "{}[shape=record,label=\"while\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::label: {
                auto element = dynamic_cast<label*>(node);
                auto style = ", fillcolor=lightblue, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"label|{}\"{}];",
                    node_vertex_name,
                    element->name(),
                    style);
            }
            case element_type_t::import_e: {
                auto element = dynamic_cast<import*>(node);
                auto style = ", fillcolor=cyan, style=\"filled\"";
                add_primary_edge(element, element->expression());
                auto from_expr = element->from_expression();
                if (from_expr != nullptr)
                    add_primary_edge(element, from_expr);
                return fmt::format(
                    "{}[shape=record,label=\"import\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::block: {
                auto element = dynamic_cast<block*>(node);
                auto style = ", fillcolor=floralwhite, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"block|{}\"{}];",
                    node_vertex_name,
                    element->id(),
                    style);
            }
            case element_type_t::module_block: {
                auto element = dynamic_cast<block*>(node);
                auto style = ", fillcolor=floralwhite, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"block|module|{}\"{}];",
                    node_vertex_name,
                    element->id(),
                    style);
            }
            case element_type_t::field: {
                auto element = dynamic_cast<field*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                add_primary_edge(element, element->identifier());
                return fmt::format(
                    "{}[shape=record,label=\"field|{}\"{}];",
                    node_vertex_name,
                    element->identifier()->symbol()->name(),
                    style);
            }
            case element_type_t::program: {
                auto program_element = dynamic_cast<program*>(node);
                auto style = ", fillcolor=aliceblue, style=\"filled\"";
                add_primary_edge(program_element, program_element->block());
                return fmt::format(
                    "{}[shape=record,label=\"program\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::rune_type: {
                auto element = dynamic_cast<rune_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"rune_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::any_type: {
                auto element = dynamic_cast<any_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                for (auto fld : element->fields().as_list())
                    add_primary_edge(element, fld);
                add_primary_edge(element, element->scope());
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"any_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::type_info: {
                auto element = dynamic_cast<type_info*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                for (auto fld : element->fields().as_list())
                    add_primary_edge(element, fld);
                add_primary_edge(element, element->scope());
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"type_info|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::return_e: {
                auto element = dynamic_cast<return_element*>(node);
                auto style = ", fillcolor=brown1, style=\"filled\"";
                for (const auto& expr : element->expressions())
                    add_primary_edge(element, expr);
                return fmt::format(
                    "{}[shape=record,label=\"return\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::proc_type: {
                auto element = dynamic_cast<procedure_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                add_primary_edge(element, element->scope());
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"proc_type|{}|foreign: {}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    element->is_foreign(),
                    style);
            }
            case element_type_t::directive: {
                auto directive_element = dynamic_cast<directive*>(node);
                auto style = ", fillcolor=darkolivegreen1, style=\"filled\"";
                add_primary_edge(directive_element, directive_element->lhs());
                auto rhs = directive_element->rhs();
                if (rhs != nullptr)
                    add_primary_edge(directive_element, directive_element->rhs());
                auto body = directive_element->body();
                if (body != nullptr)
                    add_primary_edge(directive_element, directive_element->body());
                return fmt::format(
                    "{}[shape=record,label=\"directive|{}\"{}];",
                    node_vertex_name,
                    directive_element->name(),
                    style);
            }
            case element_type_t::attribute: {
                auto attribute_element = dynamic_cast<attribute*>(node);
                auto style = ", fillcolor=azure3, style=\"filled\"";
                add_primary_edge(attribute_element, attribute_element->expression());
                return fmt::format(
                    "{}[shape=record,label=\"attribute|{}\"{}];",
                    node_vertex_name,
                    attribute_element->name(),
                    style);
            }
            case element_type_t::statement: {
                auto statement_element = dynamic_cast<statement*>(node);
                auto style = ", fillcolor=azure, style=\"filled\"";
                if (statement_element->expression() != nullptr)
                    add_primary_edge(statement_element, statement_element->expression());
                for (auto lbl : statement_element->labels())
                    add_primary_edge(statement_element, lbl);
                return fmt::format(
                    "{}[shape=record,label=\"statement\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::argument_pair: {
                auto arg_pair = dynamic_cast<argument_pair*>(node);
                auto style = ", fillcolor=azure, style=\"filled\"";
                add_primary_edge(arg_pair, arg_pair->lhs());
                add_primary_edge(arg_pair, arg_pair->rhs());
                return fmt::format(
                    "{}[shape=record,label=\"argument_pair\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::argument_list: {
                auto args = dynamic_cast<argument_list*>(node);
                auto style = ", fillcolor=azure, style=\"filled\"";
                for (auto arg_element : args->elements())
                    add_primary_edge(args, arg_element);
                return fmt::format(
                    "{}[shape=record,label=\"argument_list\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::type_reference: {
                auto element = dynamic_cast<type_reference*>(node);
                auto style = ", fillcolor=yellow, style=\"filled\"";
                if (element->type() != nullptr)
                    add_primary_edge(element, element->type());
                return fmt::format(
                    "{}[shape=record,label=\"type_reference\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::assignment: {
                auto element = dynamic_cast<assignment*>(node);
                auto style = ", fillcolor=lightblue, style=\"filled\"";
                for (auto expr : element->expressions())
                    add_primary_edge(element, expr);
                return fmt::format(
                    "{}[shape=record,label=\"assignment\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::declaration: {
                auto element = dynamic_cast<declaration*>(node);
                auto style = ", fillcolor=brown, style=\"filled\"";
                if (element->identifier() != nullptr)
                    add_primary_edge(element, element->identifier());
                if (element->assignment() != nullptr)
                    add_primary_edge(element, element->assignment());
                return fmt::format(
                    "{}[shape=record,label=\"declaration\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::identifier_reference: {
                auto element = dynamic_cast<identifier_reference*>(node);
                auto style = ", fillcolor=yellow, style=\"filled\"";
                if (element->identifier() != nullptr)
                    add_primary_edge(element, element->identifier());
                return fmt::format(
                    "{}[shape=record,label=\"identifier_reference\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::proc_call: {
                auto element = dynamic_cast<procedure_call*>(node);
                auto style = ", fillcolor=darkorchid1, style=\"filled\"";
                add_primary_edge(element, element->arguments());
                add_primary_edge(element, element->reference());
                return fmt::format(
                    "{}[shape=record,label=\"proc_call\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::pointer_type: {
                auto element = dynamic_cast<pointer_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                std::string base_type_name = "unknown";
                if (element->base_type_ref() != nullptr)
                    base_type_name = element->base_type_ref()->name();
                add_primary_edge(element, element->base_type_ref());
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"pointer_type|type: {}\"{}];",
                    node_vertex_name,
                    base_type_name,
                    style);
            }
            case element_type_t::array_type: {
                auto element = dynamic_cast<array_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                std::string entry_type_name = "unknown";
                if (element->entry_type_ref() != nullptr)
                    entry_type_name = element->entry_type_ref()->name();
                add_primary_edge(element, element->entry_type_ref());
                for (auto fld : element->fields().as_list())
                    add_primary_edge(element, fld);
                add_primary_edge(element, element->scope());
                add_primary_edge(element, element->symbol());
                for (auto s : element->subscripts())
                    add_primary_edge(element, s);
                return fmt::format(
                    "{}[shape=record,label=\"array_type|{}|type: {}\"{}];",
                    node_vertex_name,
                    element->name(),
                    entry_type_name,
                    style);
            }
            case element_type_t::identifier: {
                auto identifier_element = dynamic_cast<identifier*>(node);
                auto style = ", fillcolor=deepskyblue1, style=\"filled\"";
                std::string type_name = "unknown";
                if (identifier_element->type_ref() != nullptr)
                    type_name = identifier_element->type_ref()->symbol().name;
                auto details = fmt::format(
                    "identifier|{}|{{type: {} | inferred: {} | constant: {} }}",
                    identifier_element->symbol()->name(),
                    type_name,
                    identifier_element->inferred_type(),
                    identifier_element->symbol()->is_constant());
                add_primary_edge(identifier_element, identifier_element->type_ref());
                add_primary_edge(identifier_element, identifier_element->symbol());
                add_primary_edge(identifier_element, identifier_element->initializer());
                return fmt::format(
                    "{}[shape=record,label=\"{}\"{}];",
                    node_vertex_name,
                    details,
                    style);
            }
            case element_type_t::expression: {
                auto element = dynamic_cast<expression*>(node);
                auto style = ", fillcolor=gold3, style=\"filled\"";
                add_primary_edge(element, element->root());
                return fmt::format(
                    "{}[shape=record,label=\"expression_group\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::string_type: {
                auto element = dynamic_cast<string_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                for (auto fld : element->fields().as_list())
                    add_primary_edge(element, fld);
                add_primary_edge(element, element->scope());
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"string_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::namespace_e: {
                auto ns_element = dynamic_cast<namespace_element*>(node);
                auto style = ", fillcolor=yellow, style=\"filled\"";
                add_primary_edge(ns_element, ns_element->expression());
                return fmt::format(
                    "{}[shape=record,label=\"namespace\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::initializer: {
                auto initializer_element = dynamic_cast<initializer*>(node);
                auto style = ", fillcolor=darkolivegreen1, style=\"filled\"";
                add_primary_edge(initializer_element, initializer_element->expression());
                return fmt::format(
                    "{}[shape=record,label=\"initializer\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::bool_type: {
                auto element = dynamic_cast<bool_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"bool_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::numeric_type: {
                auto element = dynamic_cast<numeric_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"numeric_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::proc_instance: {
                auto element = dynamic_cast<procedure_instance*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                add_primary_edge(element, element->procedure_type());
                add_primary_edge(element, element->scope());
                return fmt::format(
                    "{}[shape=record,label=\"proc_instance\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::float_literal: {
                auto element = dynamic_cast<float_literal*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"float_literal|{}\"{}];",
                    node_vertex_name,
                    element->value(),
                    style);
            }
            case element_type_t::string_literal: {
                auto element = dynamic_cast<string_literal*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"string_literal|{}\"{}];",
                    node_vertex_name,
                    element->value(),
                    style);
            }
            case element_type_t::character_literal: {
                auto element = dynamic_cast<character_literal*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"character_literal|{}\"{}];",
                    node_vertex_name,
                    element->rune(),
                    style);
            }
            case element_type_t::module_type: {
                auto element = dynamic_cast<module_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                for (auto fld : element->fields().as_list())
                    add_primary_edge(element, fld);
                add_primary_edge(element, element->scope());
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"module_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::tuple_type: {
                auto element = dynamic_cast<tuple_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                for (auto fld : element->fields().as_list())
                    add_primary_edge(element, fld);
                add_primary_edge(element, element->scope());
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"tuple_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::composite_type: {
                auto element = dynamic_cast<composite_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                for (auto fld : element->fields().as_list())
                    add_primary_edge(element, fld);
                add_primary_edge(element, element->scope());
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"composite_type|{}|{}\"{}];",
                    node_vertex_name,
                    composite_type_name(element->type()),
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::label_reference: {
                auto label_ref = dynamic_cast<label_reference*>(node);
                auto style = ", fillcolor=slateblue2, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"label_reference|{}\"{}];",
                    node_vertex_name,
                    label_ref->label(),
                    style);
                break;
            }
            case element_type_t::spread_operator: {
                auto spread_element = dynamic_cast<spread_operator*>(node);
                auto style = ", fillcolor=slateblue2, style=\"filled\"";
                if (spread_element->expr() != nullptr)
                    add_primary_edge(spread_element, spread_element->expr());
                return fmt::format(
                    "{}[shape=record,label=\"spread_operator\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::unary_operator: {
                auto element = dynamic_cast<unary_operator*>(node);
                auto style = ", fillcolor=slateblue1, style=\"filled\"";
                add_primary_edge(element, element->rhs());
                return fmt::format(
                    "{}[shape=record,label=\"unary_operator|{}\"{}];",
                    node_vertex_name,
                    operator_type_name(element->operator_type()),
                    style);
            }
            case element_type_t::nil_literal: {
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"nil_literal|{}\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::boolean_literal: {
                auto element = dynamic_cast<boolean_literal*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"boolean_literal|{}\"{}];",
                    node_vertex_name,
                    element->value(),
                    style);
            }
            case element_type_t::integer_literal: {
                auto element = dynamic_cast<integer_literal*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"integer_literal|{}\"{}];",
                    node_vertex_name,
                    element->value(),
                    style);
            }
            case element_type_t::binary_operator: {
                auto element = dynamic_cast<binary_operator*>(node);
                auto style = ", fillcolor=slateblue1, style=\"filled\"";
                add_primary_edge(element, element->lhs(), "lhs");
                add_primary_edge(element, element->rhs(), "rhs");
                return fmt::format(
                    "{}[shape=record,label=\"binary_operator|{}\"{}];",
                    node_vertex_name,
                    operator_type_name(element->operator_type()),
                    style);
            }
            case element_type_t::namespace_type: {
                auto element = dynamic_cast<namespace_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"namespace_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::element:
            case element_type_t::unknown_identifier: {
                break;
            }
            case element_type_t::with: {
                auto with_element = dynamic_cast<with*>(node);
                auto style = ", fillcolor=yellow, style=\"filled\"";
                add_primary_edge(with_element, with_element->body());
                add_primary_edge(with_element, with_element->expr());
                return fmt::format(
                    "{}[shape=record,label=\"with\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::for_e: {
                auto for_e = dynamic_cast<for_element*>(node);
                auto style = ", fillcolor=yellow, style=\"filled\"";
                add_primary_edge(for_e, for_e->body());
                add_primary_edge(for_e, for_e->expression());
                add_primary_edge(for_e, for_e->induction_decl());
                return fmt::format(
                    "{}[shape=record,label=\"for\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::defer: {
                auto defer_e = dynamic_cast<defer_element*>(node);
                auto style = ", fillcolor=yellow, style=\"filled\"";
                add_primary_edge(defer_e, defer_e->expression());
                return fmt::format(
                    "{}[shape=record,label=\"defer\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::map_type: {
                auto element = dynamic_cast<map_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                for (auto fld : element->fields().as_list())
                    add_primary_edge(element, fld);
                add_primary_edge(element, element->scope());
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"map_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::intrinsic: {
                auto element = dynamic_cast<intrinsic*>(node);
                auto style = ", fillcolor=darkorchid1, style=\"filled\"";
                add_primary_edge(element, element->arguments());
                return fmt::format(
                    "{}[shape=record,label=\"intrinsic\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::type_literal: {
                auto type_lit = dynamic_cast<type_literal*>(node);
                auto style = ", fillcolor=yellow, style=\"filled\"";
                add_primary_edge(type_lit, type_lit->args());
                return fmt::format(
                    "{}[shape=record,label=\"type_literal\"{}];",
                    node_vertex_name,
                    style);
            }
            case element_type_t::unknown_type: {
                auto element = dynamic_cast<unknown_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"unknown_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::generic_type: {
                auto element = dynamic_cast<generic_type*>(node);
                auto style = ", fillcolor=gainsboro, style=\"filled\"";
                add_primary_edge(element, element->symbol());
                return fmt::format(
                    "{}[shape=record,label=\"generic_type|{}\"{}];",
                    node_vertex_name,
                    element->symbol()->name(),
                    style);
            }
            case element_type_t::assembly_label: {
                auto element = dynamic_cast<assembly_label*>(node);
                auto style = ", fillcolor=pink, style=\"filled\"";
                return fmt::format(
                    "{}[shape=record,label=\"assembly_label|{}\"{}];",
                    node_vertex_name,
                    element->name(),
                    style);
            }
        }

        return "";
    }

    void code_dom_formatter::format(const std::string& title) {
        fmt::print(_file, "digraph {{\n");
        fmt::print(_file, "rankdir=LR\n");
        fmt::print(_file, "graph [ fontsize=22 ];\n");
        fmt::print(_file, "labelloc=\"t\";\n");
        fmt::print(_file, "label=\"{}\";\n", title);

        _nodes.clear();
        _edges.clear();

        auto non_const_program = const_cast<compiler::program*>(&_session.program());
        _nodes.insert(format_node(non_const_program));

        for (const auto& pair : _session.elements()) {
            auto node_def = format_node(pair.second);
            if (node_def.empty())
                continue;

            _nodes.insert(node_def);

            add_secondary_edge(pair.second->parent_scope(), pair.second);
        }

        for (const auto& node : _nodes)
            fmt::print(_file, "\t{}\n", node);
        for (const auto& edge : _edges)
            fmt::print(_file, "\t{}\n", edge);

        fmt::print(_file, "}}\n");
    }

    std::string code_dom_formatter::get_vertex_name(element* node) const {
        return fmt::format(
            "{}_{}",
            element_type_name(node->element_type()),
            node->id());
    }

};