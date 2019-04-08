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

class NaoObject;
class NaoIO;

class LIBNAO_API NaoPlugin;
class LIBNAO_API NaoAction;

using PluginFunc = NaoPlugin*(*)();

class LIBNAO_API NaoPlugin {
    public:
    virtual ~NaoPlugin() = default;

    // Plugin info
    N_NODISCARD virtual NaoString Name() const = 0;
    N_NODISCARD virtual NaoString DisplayName() const = 0;
    N_NODISCARD virtual NaoString PluginDescription() const = 0;
    N_NODISCARD virtual NaoString VersionString() const = 0;

    // Author info
    N_NODISCARD virtual NaoString AuthorName() const = 0;
    N_NODISCARD virtual NaoString AuthorDescription() const = 0;

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
};

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
