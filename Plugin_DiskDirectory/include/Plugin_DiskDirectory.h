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
    N_NODISCARD NaoString name() const override;
    N_NODISCARD NaoString display_name() const override;
    N_NODISCARD NaoString plugin_description() const override;
    N_NODISCARD NaoString version_string() const override;

    N_NODISCARD NaoString author_name() const override;
    N_NODISCARD NaoString author_description() const override;

    N_NODISCARD bool can_populate(NTreeNode* node) override;
    bool populate(NTreeNode* node) override;
};
