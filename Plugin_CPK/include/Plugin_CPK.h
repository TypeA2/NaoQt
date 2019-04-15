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

#include <libnao.h>

#include <Plugin/NaoPlugin.h>
#include <Containers/NaoVector.h>

LIBNAO_PLUGIN_CALL LIBNAO_PLUGIN_DECL NaoPlugin* GetNaoPlugin();

class Plugin_CPK final : public NaoPlugin {
    public:
    Plugin_CPK() = default;

    N_NODISCARD NaoString Name() const override;
    N_NODISCARD NaoString DisplayName() const override;
    N_NODISCARD NaoString PluginDescription() const override;
    N_NODISCARD NaoString VersionString() const override;

    N_NODISCARD NaoString AuthorName() const override;
    N_NODISCARD NaoString AuthorDescription() const override;

    N_NODISCARD bool HasDescription(NaoObject* object) override;
    N_NODISCARD bool PrioritiseDescription() const override;
    N_NODISCARD NaoString Description() const override;

    N_NODISCARD bool CanEnter(NaoObject* object) override;
    bool Enter(NaoObject* object) override;

    N_NODISCARD bool ShouldLeave(NaoObject* object) override;
    bool Leave(NaoObject* object) override;

    N_NODISCARD bool HasContextMenu(NaoObject* object) override;
    //N_NODISCARD NaoVector<NaoAction*> ContextMenu(NaoObject* object) override;

    N_NODISCARD Event SubscribedEvents() const override;
    bool TriggerEvent(Event event, EventArgs* args) override;

    N_NODISCARD bool ProvidesNewRoot(NaoObject* from, NaoObject* to) override;
    N_NODISCARD NaoObject* NewRoot(NaoObject* from, NaoObject* to) override;

    N_NODISCARD bool ProvidesChild(const NaoString& name) override;
    N_NODISCARD NaoObject* GetChild(const NaoString& name) override;

    private:
    NaoObject* _m_root;
    bool _m_state;
    NaoVector<NaoObject*> _m_children;

    bool MoveEvent(MoveEventArgs* args);
};
