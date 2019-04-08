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

#include "Plugin/NaoPlugin.h"

bool NaoPlugin::PrioritiseDescription() const {
    return false;
}

NaoString NaoPlugin::Description() const {
    return "No description given";
}

NaoString NaoPlugin::Description(NaoObject* of) {
    return Description();
}

bool NaoPlugin::CanEnter(NaoObject* object) {
    return false;
}

bool NaoPlugin::Enter(NaoObject* object) {
    return false;
}

bool NaoPlugin::ShouldLeave(NaoObject* object) {
    return false;
}

bool NaoPlugin::Leave(NaoObject* object) {
    return false;
}

bool NaoPlugin::CanDecode(NaoObject* object) {
    return false;
}

bool NaoPlugin::Decode(NaoObject* object, NaoIO* output) {
    return false;
}

bool NaoPlugin::HasContextMenu(NaoObject* object) {
    return false;
}

NaoVector<NaoAction*> NaoPlugin::ContextMenu(NaoObject* object) {
    return { };
}

#pragma region NaoAction

NaoAction::NaoAction(NaoPlugin* parent)
    : _m_parent(parent) { }

NaoPlugin* NaoAction::parent() const {
    return _m_parent;
}

#pragma endregion
