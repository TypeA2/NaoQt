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

bool plugin_complete(const NaoPlugin& plugin) {
    return plugin.plugin
        && plugin.plugin_name
        && plugin.plugin_desc
        && plugin.plugin_version
        && plugin.author_name
        && plugin.author_text_plain
        && plugin.author_text_rich
        && plugin.error
        && plugin.supports
        && plugin.populatable
        && plugin.decodable
        && plugin.description
        && plugin.populate
        && plugin.decode;
}