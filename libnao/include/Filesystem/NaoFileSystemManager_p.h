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

class NTreeNode;

/**
 * \ingroup internal
 * \relates NaoFileSystemManager
 *
 * \brief Opaque pointer class for NaoFileSystemManager.
 */
class NFSMPrivate {
    public:

    /**
     * \brief Default constructor
     */
    NFSMPrivate();

    /**
     * \brief Destructor.
     *
     * Deletes the NTreeNode filesystem tree as well.
     */
    ~NFSMPrivate();

    /**
     * \brief Perform first-time setup.
     * \return Whether the operation succeded.
     */
    bool init();

    /**
     * \brief Access the root node.
     * \return Pointer to the root tree node.
     */
    NTreeNode* root();

    private:

    // Root tree node
    NTreeNode* _m_root;


#if 0
    public:

    ~NFSMPrivate();

    // Initialise in the root directory
    bool init(const NaoString& root_dir);

    // Move to the target directory (may be relative)
    bool move(const NaoString& target);

    NaoString description_for_object(NaoObject* object) const;

    // Current object
    NaoObject* m_current_object = nullptr;

    // Currently used plugin
    NaoPlugin* m_current_plugin = nullptr;

    // Latest error code
    NaoString m_last_error;

    private:

    NaoObject* _try_locate_child(const NaoString& path);
#endif
};
