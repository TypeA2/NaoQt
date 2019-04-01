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

#include "CPKManager.h"

#define N_LOG_ID "Plugin_CPK/CPKManager"
#include <Logging/NaoLogging.h>
#include <Containers/NaoBytes.h>
#include <IO/NaoIO.h>
#include <NaoObject.h>

CPKManager& CPKManager::instance() {
    static CPKManager manager;
    return manager;
}

bool CPKManager::populatable(NaoObject* object) {
    if (!object->is_dir()) {
        ndebug << object->name() << object->file_ref().io->read_singleshot(4).data();
    }
    return !object->is_dir()
        && object->file_ref().io->read_singleshot(4) == NaoBytes("CPK ", 4);;
}

bool CPKManager::can_move(NaoObject* from, NaoObject* to) const {
    ndebug << to->is_child_of(_m_root)
           << populatable(from)
           << from->name().starts_with(to->name())
           << !from->name().substr(std::size(to->name()) + 1).contains(N_PATHSEP);

    return (to->is_child_of(_m_root)
        || (populatable(from)
            && from->name().starts_with(to->name())
            && !from->name().substr(std::size(to->name()) + 1).contains(N_PATHSEP)));
}

bool CPKManager::populate(NaoObject* object) {
    return false;
}

void CPKManager::_cleanup() {
    
}

