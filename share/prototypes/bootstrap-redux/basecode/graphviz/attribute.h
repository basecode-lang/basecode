// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/types.h>
#include <basecode/adt/string.h>

namespace basecode::graphviz {

    using namespace std::literals;

    static constexpr uint8_t edge_flag = 0b00000001;
    static constexpr uint8_t node_flag = 0b00000010;
    static constexpr uint8_t graph_flag = 0b00000100;
    static constexpr uint8_t subgraph_flag = 0b00001000;
    static constexpr uint8_t cluster_subgraph_flag = 0b00010000;

    enum class component_type_t : uint8_t {
        edge                = edge_flag,
        node                = node_flag,
        graph               = graph_flag,
        subgraph            = subgraph_flag,
        cluster_subgraph    = cluster_subgraph_flag
    };

    std::string_view component_type_to_name(component_type_t type);

    ///////////////////////////////////////////////////////////////////////////

    enum class attribute_type_t {
        rankdir,            // G                 TB, LR, BT, RL
        fontsize,           // G                 double
        label,              // G, N, E           string
        fillcolor,          // N                 color, colorList,
        labelloc,           // G                 string
        shape,              // N                 none, record, box, polygon, ellipse, oval, circle, point, etc.
        style,              // N                 filled, invisible, diagonals, rounded, dashed, dotted, solid, bold

        background,
        arrowhead,
        arrowsize,
        arrowtail,
        bgcolor,
        center,
        charset,
        clusterrank,
        color,
        colorscheme,
        comment,
        compound,
        concentrate,
        constraint,
        decorate,
        dir,
        distortion,
        esep,
        fixedsize,
        fontcolor,
        fontname,
        fontpath,
        forcelabels,
        gradientangle,
        group,
        headclip,
        headlabel,
        headport,
        height,
        image,
        imagepath,
        imagepos,
        imagescale,
        labelangle,
        labeldistance,
        labelfloat,
        labelfontcolor,
        labelfontname,
        labelfontsize,
        labeljust,
        landscape,
        layer,
        layerlistsep,
        layers,
        layerselect,
        layersep,
        layout,
        lhead,
        ltail,
        margin,
        mclimit,
        minlen,
        newrank,
        nodesep,
        nojustify,
        normalize,
        nslimit,
        nslimit1,
        ordering,
        orientation,
        outputorder,
        overlap,
        pack,
        packmode,
        pad,
        page,
        pagedir,
        pencolor,
        penwidth,
        peripheries,
        pos,
        quantum,
        rank,
        ranksep,
        ratio,
        regular,
        remincross,
        rotate,
        samehead,
        sametail,
        samplepoints,
        scale,
        searchsize,
        sep,
        shapefile,
        showboxes,
        sides,
        size,
        skew,
        sortv,
        splines,
        tailclip,
        taillabel,
        tailport,
        viewport,
        voro_margin,
        weight,
        width,
        xlabel,
        z
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class attribute_value_type_t {
        string,
        boolean,
        integer,
        enumeration,
        floating_point,
    };

    struct attribute_value_t final {
        attribute_type_t type;
        attribute_value_type_t value_type;
        union {
            bool flag;
            int32_t integer;
            double floating_point;
            adt::string_t* string;
        } value;
    };

    struct enumeration_value_t final {
        constexpr explicit enumeration_value_t(const char* name) : name(name) {
        }
        const char* name;
    };

    using attribute_value_list_t = adt::array_t<attribute_value_t*>;

}