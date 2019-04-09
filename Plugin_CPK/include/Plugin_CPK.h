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

    private:
    NaoObject* _m_root;
};
/*
namespace Plugin {
    namespace Description {
        LIBNAO_PLUGIN_DECL bool prioritise_description();
        LIBNAO_PLUGIN_DECL NaoString description();
    }

    namespace Capabilities {
        LIBNAO_PLUGIN_DECL bool supports(NaoObject* object);
        LIBNAO_PLUGIN_DECL bool populatable(NaoObject* object);
        LIBNAO_PLUGIN_DECL bool decodable(N_UNUSED NaoObject* object);
        LIBNAO_PLUGIN_DECL bool can_move(NaoObject* from, NaoObject* to);
    }

    namespace Function {
        LIBNAO_PLUGIN_DECL bool populate(NaoObject* object);
        LIBNAO_PLUGIN_DECL bool decode(N_UNUSED NaoObject* object, N_UNUSED NaoIO* out);
        LIBNAO_PLUGIN_DECL bool move(NaoObject*& from, NaoObject* to);
    }

    namespace ContextMenu {
        LIBNAO_PLUGIN_DECL bool has_context_menu(NaoObject* object);
        LIBNAO_PLUGIN_DECL NaoPlugin::ContextMenu::type context_menu(NaoObject* object);
    }

    namespace Extraction {
        LIBNAO_PLUGIN_DECL bool extract_single_file(NaoObject* object);
        LIBNAO_PLUGIN_DECL bool extract_all_files(NaoObject* object);
    }
}
*/