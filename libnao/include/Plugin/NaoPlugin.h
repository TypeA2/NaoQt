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
#include "Containers/NaoVector.h"
#include "Containers/NaoPair.h"

#include <cstdint>

class NaoObject;
class NaoIO;

class LIBNAO_API NaoPlugin;

using PluginFunc = NaoPlugin*(*)();

class LIBNAO_API NaoPlugin {
    public:
    virtual ~NaoPlugin();

    // Plugin info
    N_NODISCARD virtual NaoString Name() const = 0;
    N_NODISCARD virtual NaoString DisplayName() const = 0;
    N_NODISCARD virtual NaoString PluginDescription() const = 0;
    N_NODISCARD virtual NaoString VersionString() const = 0;

    // Author info
    N_NODISCARD virtual NaoString AuthorName() const = 0;
    N_NODISCARD virtual NaoString AuthorDescription() const = 0;

    // Supported file description
    N_NODISCARD virtual bool Prioritise() const = 0;
    N_NODISCARD virtual NaoString Description() const = 0;
    N_NODISCARD virtual NaoString Description(NaoObject* of) const = 0;

    // Entering
    N_NODISCARD virtual bool CanEnter(NaoObject* object) = 0;
    virtual bool Enter(NaoObject* object) = 0;

    // Leaving
    N_NODISCARD virtual bool ShouldLeave(NaoObject* object) = 0;
    virtual bool Leave(NaoObject* object) = 0;

    // Moving
    N_NODISCARD virtual bool CanMove(NaoObject* object) = 0;
    virtual bool Move(NaoObject* object) = 0;

    // Decoding
    N_NODISCARD virtual bool CanDecode(NaoObject* object) = 0;
    virtual bool Decode(NaoObject* object, NaoIO* output) = 0;

    // Context menu
    class NaoContextMenuEntry {
        public:
        virtual ~NaoContextMenuEntry() = 0;

        N_NODISCARD virtual NaoString EntryName() = 0;
        virtual bool Execute(NaoObject* object) = 0;
    };

    N_NODISCARD virtual bool HasContextMenu(NaoObject* object) = 0;
    N_NODISCARD virtual NaoVector<NaoContextMenuEntry> ContextMenu(NaoObject* object) = 0;
};
