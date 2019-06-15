/*
    This file is part of NaoQt.

    NaoQt is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NaoQt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with NaoQt.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "libnao.h"

#include "Containers/NaoString.h"

//class NaoObject;
//class NaoIO;

class LIBNAO_API NaoPlugin;
class LIBNAO_API NTreeNode;
//class LIBNAO_API NaoAction;

using PluginFunc = NaoPlugin*(*)();

/**
 * \ingroup libnao
 *
 * \brief Base class from which all plugins inherit.
 *
 * This class outlines the basic functionalities of every plugin.
 */
class LIBNAO_API NaoPlugin {
    public:

    /**
     * \brief Virtual destructor.
     */
    virtual ~NaoPlugin() = default;

    /**
     * \return The name of the plugin.
     */
    N_NODISCARD virtual NaoString name() const = 0;

    /**
     * \return The (possibly shorted) display name of the plugin.
     */
    N_NODISCARD virtual NaoString display_name() const = 0;

    /**
     * \return A description of the plugin.
     */
    N_NODISCARD virtual NaoString plugin_description() const = 0;

    /**
     * \return A string representing the plugin's version.
     */
    N_NODISCARD virtual NaoString version_string() const = 0;

    /**
     * \return The name of the author of the plugin.
     */
    N_NODISCARD virtual NaoString author_name() const = 0;

    /**
     * \return A custom text by the author of the plugin.
     */
    N_NODISCARD virtual NaoString author_description() const = 0;



    /**
     * \brief Whether this plugin can populate a node.
     * \param[in] node The node to check.
     * \return Whether this plugin can populate the node.
     */
    N_NODISCARD virtual bool can_populate(NTreeNode* node);

    /**
     * \brief Populates a node.
     * \param[in] node The node to populate.
     * \return Whether the operation succeeded.
     *
     * Populating a node means to fill in it's children and any possible sub-children.
     */
    virtual bool populate(NTreeNode* node) = 0;

#if 0
    enum Event : uint64_t {
        None = 0x0,
        Move = 0x1
    };

    static constexpr Event AllEvents[] = {
        Move
    };

    struct EventArgs { };

    struct MoveEventArgs : EventArgs {
        MoveEventArgs(NaoObject* from, NaoObject* to);
        NaoObject* from;
        NaoObject* to;
    };





    // Supported file description
    N_NODISCARD virtual bool HasDescription(NaoObject* object) = 0;
    N_NODISCARD virtual bool PrioritiseDescription() const;
    N_NODISCARD virtual NaoString Description() const;
    N_NODISCARD virtual NaoString Description(NaoObject* of);

    // Entering
    N_NODISCARD virtual bool CanEnter(NaoObject* object);
    virtual bool Enter(NaoObject* object);

    // Leaving
    N_NODISCARD virtual bool ShouldLeave(NaoObject* object);
    virtual bool Leave(NaoObject* object);

    // Decoding
    N_NODISCARD virtual bool CanDecode(NaoObject* object);
    virtual bool Decode(NaoObject* object, NaoIO* output);

    // Context menu
    N_NODISCARD virtual bool HasContextMenu(NaoObject* object);
    N_NODISCARD virtual NaoVector<NaoAction*> ContextMenu(NaoObject* object);

    // Event subscribers
    N_NODISCARD virtual Event SubscribedEvents() const;
    virtual bool TriggerEvent(Event event, EventArgs* args = nullptr);

    // Archives
    N_NODISCARD virtual bool ProvidesNewRoot(NaoObject* from, NaoObject* to);
    N_NODISCARD virtual NaoObject* NewRoot(NaoObject* from, NaoObject* to);

    N_NODISCARD virtual bool ProvidesChild(const NaoString& name);
    N_NODISCARD virtual NaoObject* GetChild(const NaoString& name);
#endif
};

#if 0
class LIBNAO_API NaoAction {
    public:
    NaoAction(NaoPlugin* parent);
    virtual ~NaoAction() = default;

    N_NODISCARD virtual NaoString ActionName() = 0;
    virtual bool Execute(NaoObject* object) = 0;

    N_NODISCARD NaoPlugin* parent() const;

    private:
    NaoPlugin* _m_parent;
};
#endif
