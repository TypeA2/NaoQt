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

#include <memory>

#define NFSM NaoFileSystemManager::global_instance()

class NaoString;
class NaoPlugin;

class NTreeNode;
class NFSMPrivate;

/**
 * \ingroup libnao
 *
 * \brief Singleton class which keeps track of a filesystem tree.
 */
class NaoFileSystemManager {
    public:
    /**
     * \brief Access the singleton instance.
     * \return Reference to the global instance of the NaoFileSystemManager class.
     */
    N_NODISCARD LIBNAO_API static NaoFileSystemManager& global_instance();

    /**
     * \brief Initialise the NaoFileSystemManager with the specified starting directory.
     * \param[in] start_dir The initial directory.
     * \return Whether the operation succeeded.
     */
    LIBNAO_API bool init(const NaoString& start_dir) const;

    /**
     * \brief Creates a node (and all it's parent nodes if needed).
     * \param[in] path The path of the node to create.
     * \return Pointer to the newly created node.
     */
    N_NODISCARD LIBNAO_API NTreeNode* retrieve_node(const NaoString& path) const;

    /**
     * \brief Moves the current node to the new path.
     * \param[in] path The path of the new position.
     * \return Whether the operation succeeded.
     */
    LIBNAO_API bool move(const NaoString& path) const;

    /**
     * \return Pointer to the currently active node.
     */
    N_NODISCARD LIBNAO_API NTreeNode* current() const;

    /**
     * \brief Get a node's plugin-supplied description.
     * \param[in] node The node to fetch the description for.
     * \return The description for the specified note.
     */
    N_NODISCARD LIBNAO_API NaoString description(NTreeNode* node) const;

    //LIBNAO_API bool move(const NaoString& target);

    //N_NODISCARD LIBNAO_API NaoObject* current_object() const;
    //N_NODISCARD LIBNAO_API const NaoString& current_path() const;
    //N_NODISCARD LIBNAO_API NaoPlugin* current_plugin() const;

    //N_NODISCARD LIBNAO_API const NaoString& last_error() const;



    private:

    /**
     * \brief Constructor to initialise d_ptr
     */
    NaoFileSystemManager();

    // Opaque pointer
    std::unique_ptr<NFSMPrivate> d_ptr;
};
