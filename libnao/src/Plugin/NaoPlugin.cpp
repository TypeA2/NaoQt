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

NaoString NaoPlugin::Description(NaoObject* of) const {
    return "No description given";
}

bool NaoPlugin::Enter(NaoObject* object) {
    return false;
}

bool NaoPlugin::Leave(NaoObject* object) {
    return false;
}

bool NaoPlugin::Move(NaoObject*& from, NaoObject* to) {
    return false;
}

bool NaoPlugin::Decode(NaoObject* object, NaoIO* output) {
    return false;
}

NaoVector<NaoPlugin::NaoContextMenuEntry*> NaoPlugin::ContextMenu(NaoObject* object) {
    return { };
}
