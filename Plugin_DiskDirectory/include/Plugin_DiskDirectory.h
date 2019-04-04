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

LIBNAO_PLUGIN_CALL LIBNAO_PLUGIN_DECL NaoPlugin* GetNaoPlugin();

class Plugin_DiskDirectory final : public NaoPlugin {
    public:
    N_NODISCARD NaoString Name() const override;
    N_NODISCARD NaoString DisplayName() const override;
    N_NODISCARD NaoString PluginDescription() const override;
    N_NODISCARD NaoString VersionString() const override;

    N_NODISCARD NaoString AuthorName() const override;
    N_NODISCARD NaoString AuthorDescription() const override;

    N_NODISCARD bool HasDescription(NaoObject* object) const override;
    N_NODISCARD bool PrioritiseDescription() const override;
    N_NODISCARD NaoString Description() const override;
    N_NODISCARD NaoString Description(NaoObject* of) const override;

    N_NODISCARD bool CanEnter(NaoObject* object) override;
    bool Enter(NaoObject* object) override;
};
