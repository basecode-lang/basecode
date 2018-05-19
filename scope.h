#pragma once

#include <memory.h>
#include "symbol_table.h"

namespace basecode {

    class scope;

    using scope_list = std::vector<std::unique_ptr<scope>>;

    class scope {
    public:
        scope(
            basecode::scope* parent,
            const ast_node_shared_ptr& node);

        void clear();

        uint64_t address() const;

        basecode::scope* parent();

        void address(uint64_t value);

        ast_node_shared_ptr ast_node();

        const scope_list& children() const;

        basecode::scope* add_child_scope(const ast_node_shared_ptr& node);

    private:
        uint64_t _address;
        scope_list _children {};
        basecode::scope* _parent = nullptr;
        ast_node_shared_ptr _node = nullptr;
        basecode::symbol_table _symbol_table {};
    };

};

