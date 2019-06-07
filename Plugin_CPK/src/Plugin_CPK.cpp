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

#include "Plugin_CPK.h"

#define N_LOG_ID "Plugin_CPK"
#include <Logging/NaoLogging.h>
#include <NaoObject.h>
#include <IO/NaoIO.h>
#include <Decoding/Archives/NaoCPKReader.h>
#include <Decoding/NaoDecodingException.h>
#include <Plugin/NaoPluginManager.h>

NaoPlugin* GetNaoPlugin() {
    return new Plugin_CPK();
}

#pragma region Plugin info

NaoString Plugin_CPK::Name() const {
    return "libnao CPK plugin";
}

NaoString Plugin_CPK::DisplayName() const {
    return "libnao CPK";
}

NaoString Plugin_CPK::PluginDescription() const {
    return "Adds support for CPK archives";
}

NaoString Plugin_CPK::VersionString() const {
    return "1.1";
}

#pragma endregion 

#pragma region Author info

NaoString Plugin_CPK::AuthorName() const {
    return "TypeA2/I_Copy_Jokes";
}

NaoString Plugin_CPK::AuthorDescription() const {
    return "License: LGPLv3 or later<br>"
        "<a href=\"https://github.com/TypeA2\">Github</a><br>"
        "<a href=\"https://steamcommunity.com/id/TypeA2/\">Steam</a>";
}

#pragma endregion 

#pragma region Description

bool Plugin_CPK::HasDescription(NaoObject* object) {
    return !object->is_dir()
        && object->file_ref().io->read_singleshot(4) == NaoBytes("CPK ", 4);
}

bool Plugin_CPK::PrioritiseDescription() const {
    return true;
}

NaoString Plugin_CPK::Description() const {
    return "CPK Archive";
}

#pragma endregion 

#pragma region Actions 

bool Plugin_CPK::CanEnter(NaoObject* object) {
    if (_m_state) {
        // Is root
        if (object->name() == _m_root->name()) {
            return true;
;       }

        // Is a known child
        for (NaoObject* child : _m_children) {
            if (child->is_dir()
                && child->name() == object->name()) {
                return true;
            }
        }
    }

    // Is a CPK archive
    return !object->is_dir()
        && object->file_ref().io->read_singleshot(4) == NaoBytes("CPK ", 4);
}

bool Plugin_CPK::Enter(NaoObject* object) {
    if (!_m_state) {
        NaoIO* io = object->file_ref().io;

        NaoCPKReader* reader = nullptr;
        try {
            reader = new NaoCPKReader(io);
        } catch (const NaoDecodingException & e) {
            nerr << e.what();
            return false;
        }

        for (NaoObject* file : reader->take_files()) {
            file->set_name(object->name() + N_PATHSEP + file->name());

            // I don't even know why this check is needed
            // but everything explodes without it
            if (!std::empty(file->description())) {
                PluginManager.set_description(file);
            }

            _m_children.push_back(file);
        }

        delete reader;

        _m_root = object;
        _m_state = true;
    }
    
    const NaoString& subpath = object->name();
   
    for (NaoObject* file : _m_children) {
        if (file == object) {
            continue;
        }

        if (file->direct_child_of(subpath)) {
            object->add_child(file);
        }
    }

    return true;
}

bool Plugin_CPK::ShouldLeave(NaoObject* object) {
    return object->is_child_of(_m_root) || object == _m_root;
}

bool Plugin_CPK::Leave(NaoObject* object) {
    return true;
}

bool Plugin_CPK::HasContextMenu(NaoObject* object) {
    return false;
}

NaoPlugin::Event Plugin_CPK::SubscribedEvents() const {
    return Move;
}

bool Plugin_CPK::TriggerEvent(Event event, EventArgs* args) {
    if (!args) {
        return false;
    }

    switch (event) {
        case Move:
            return MoveEvent(static_cast<MoveEventArgs*>(args));

        default:
            return false;
    }
}

bool Plugin_CPK::MoveEvent(MoveEventArgs* args) {
    if (!args->from || !args->to) {
        return false;
    }

    auto [from, to] = *args;

    if (_m_state) {
        // Exiting this archive, free up everything
        if (!(to->name().starts_with(_m_root->name()))) {
            nlog << "Leaving archive";

            _m_state = false;

            for (NaoObject* child : _m_children) {
                if (child == from) {
                    continue;
                }
                
                delete child;
            }

            _m_children.clear();

            (void) _m_root->take_children();

            if (from != _m_root) {
                delete _m_root;
            }

            _m_state = false;

            return true;
        }
        
    }

    return true;
}

bool Plugin_CPK::ProvidesNewRoot(NaoObject* from, NaoObject* to) {
    if (!_m_state) {
        return false;
    }

    if (to->name() == _m_root->name()) {
        return true;
    }

    for (NaoObject* child : _m_children) {
        if (child->name() == to->name()) {
            return true;
        }
    }

    return false;
}

NaoObject* Plugin_CPK::NewRoot(NaoObject* from, NaoObject* to) {
    (void) from->take_children();

    if (to->name() == _m_root->name()) {
        return _m_root;
    }

    for (NaoObject* child : _m_children) {
        if (child->name() == to->name()) {
            return child;
        }
    }

    return to;
}

bool Plugin_CPK::ProvidesChild(const NaoString& name) {
    if (!_m_state) {
        return false;
    }

    if (_m_root->name() == name) {
        return true;
    }

    for (NaoObject* child : _m_children) {
        if (child->name() == name) {
            return true;
        }
    }

    return false;
}

NaoObject* Plugin_CPK::GetChild(const NaoString& name) {
    if (_m_root->name() == name) {
        return _m_root;
    }

    for (NaoObject* child : _m_children) {
        if (child->name() == name) {
            return child;
        }
    }

    return nullptr;
}

#pragma endregion
/*
namespace Plugin {

    namespace Capabilities {
        bool supports(NaoObject* object) {
            return populatable(object);
        }

        bool populatable(NaoObject* object) {
            return CPK.populatable(object);
        }

        bool decodable(N_UNUSED NaoObject* object) {
            return false;
        }

        bool can_move(NaoObject* from, NaoObject* to) {
            return CPK.can_move(from, to);
        }
    }

    namespace Function {
        bool populate(N_UNUSED NaoObject* object) {
            if (!Capabilities::populatable(object)) {
                return false;
            }

            return CPK.populate(object);
        }

        bool decode(N_UNUSED NaoObject* object, N_UNUSED NaoIO* out) {
            return false;
        }

        bool move(N_UNUSED NaoObject*& from, N_UNUSED NaoObject* to) {
            return false;
        }
    }

    namespace ContextMenu {
        bool has_context_menu(N_UNUSED NaoObject* object) {
            return false;
        }

        NaoPlugin::ContextMenu::type context_menu(N_UNUSED NaoObject* object) {
            return NaoPlugin::ContextMenu::type();
        }
    }

    namespace Extraction {
        bool extract_single_file(N_UNUSED NaoObject* object) {
            return false;
        }

        bool extract_all_files(N_UNUSED NaoObject* object) {
            return false;
        }
    }
}
*/
