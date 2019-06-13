/*
    This file is part of libnao.

    libnao is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with libnao.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "libnao.h"
#include "Containers/NaoVector.h"
#include "Containers/NaoString.h"

/**
 * \ingroup plugin_interface
 * \relates NaoFileSystemManager
 *
 * \brief Represents a filesystem node.
 */
class LIBNAO_API NTreeNode {
    public:
    /**
     * \brief Destructor that also deletes child nodes.
     */
    ~NTreeNode();

    /**
     * \brief Default empty constructor.
     */
    NTreeNode() = default;

    /**
     * \brief Constructor which assigns a name and a parent.
     * \param[in] name The name of this node.
     * \param[in] parent Pointer to the parent element of this node.
     */
    NTreeNode(const NaoString& name, NTreeNode* parent = nullptr);

    /**
     * \brief Lock this node.
     * \return Whether the lock state was successfully changed.
     *
     * If a node is locked, none of it's descendants will be deleted automatically.
     */
    bool lock();

    /**
     * \brief Unlock this node.
     * \return Whether the lock state was successfully changed.
     */
    bool unlock();

    /**
     * \return Whether this node is locked.
     */
    N_NODISCARD bool locked() const;

    /**
     * \brief Set this node's name.
     * \param[in] name The node's new name.
     * \return Whether the name was successfully changed.
     * \note Fails if the parent node has a child node with the same name already.
     */
    bool set_name(const NaoString& name);

    /**
     * \return This node's name.
     */
    N_NODISCARD const NaoString& name() const;

    /**
     * \brief Set this node's parent.
     * \param[in] parent The new parent node.
     * \return Whether the parent was successfully changed.
     * \note Fails if the node is already a child of the new parent.
     */
    bool set_parent(NTreeNode* parent);

    /**
     * \brief Get the parent node.
     * \return This node's parent node.
     */
    N_NODISCARD NTreeNode* parent() const;

    /**
     * \brief Add a child node to this node.
     * \param[in] child The new child node to add.
     * \return Whether the child was successfully added.
     * \note Fails if `child` is already a child or there is another child with the same name.
     */
    bool add_child(NTreeNode* child);

    /**
     * \brief Get child nodes.
     * \return NaoVector containing pointers to child nodes.
     */
    N_NODISCARD const NaoVector<NTreeNode*>& children() const;

    /**
     * \brief Checks if the node has a direct child with the specified name.
     * \param[in] name The name to check for.
     * \return Whether this node has a child with the name `name`.
     */
    N_NODISCARD bool has_child(const NaoString& name) const;

    /**
     * \brief Checks if the node has the node as a child.
     * \param[in] node Pointer to the node to check for.
     * \return Whether `node` is a direct child of this node.
     */
    N_NODISCARD bool has_child(NTreeNode* node) const;

    /**
     * \brief Get a pointer to the child with the specified name, or `nullptr`.
     * \param[in] name The name of the child to retrieve.
     * \return Pointer to the child if found, else `nullptr`.
     */
    N_NODISCARD NTreeNode* get_child(const NaoString& name) const;

    private:
    NTreeNode* _m_parent;
    NaoString _m_name;

    NaoVector<NTreeNode*> _m_children;

    bool _m_locked;
};
