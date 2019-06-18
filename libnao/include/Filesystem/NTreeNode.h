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

class LIBNAO_API NaoIO;

class LIBNAO_API NTreeNode;

// Export NaoVector instance of nodes
template class LIBNAO_API NaoVector<NTreeNode*>;

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
     * \param[in] parent Optional pointer to the parent element of this node.
     * \param[in] io Optional pointer to an IO object. Will be deleted when this node is deleted.
     */
    NTreeNode(const NaoString& name, NTreeNode* parent = nullptr, NaoIO* io = nullptr);

    /**
     * \brief Lock this node.
     * \return Whether the lock state was successfully changed.
     *
     * If a node is locked, none of it's descendants will be deleted automatically.
     * This can be used to prevent an expensive operation to happen multiple times.
     * Locked nodes will however be deleted if they are no longer in scope.
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

    /**
     * \brief Remove all of a node's children.
     */
    void clear_children();

    /**
     * \brief Sets whether the node has been populated already, or needs to be populated again.
     * \param[in] state The new populated state of this node.
     */
    void set_populated(bool state);

    /**
     * \return Whether this node hasb een populated already.
     */
    N_NODISCARD bool populated() const;

    /**
     * \brief Constructs the node's path.
     * \return Complete path pointing to the node, using N_PATHSEP.
     */
    N_NODISCARD NaoString path() const;

    /**
     * \brief Sets this node's IO object.
     * \param[in] io The new IO object.
     */
    void set_io(NaoIO* io);

    /**
     * \return This node's IO object.
     */
    N_NODISCARD NaoIO* io() const;

    /**
     * \return Whether this node represents a directory or a file.
     * \note This function checks if an IO object is present. If so, it's considered a file.
     */
    N_NODISCARD bool is_dir() const;

    private:
    // Parent node of this node
    NTreeNode* _m_parent;

    // Name of this node
    NaoString _m_name;

    // This node's children
    NaoVector<NTreeNode*> _m_children;

    // Whether this node is locked
    bool _m_locked;

    // Whether this node has been populated already
    bool _m_populated;

    // Whether an IO object if this node represents a file, or nullptr if it's a directory
    NaoIO* _m_io;
};
