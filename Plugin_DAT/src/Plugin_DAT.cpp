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

#include "Plugin_DAT.h"

#include <Logging/NaoLogging.h>

LIBNAO_CALL const char* NaoName() {
    return "DAT plugin bundled with NaoQt";
}

LIBNAO_CALL const char* NaoDescription() {
    return "Reads DAT archives";
}

LIBNAO_CALL uint64_t NaoVersion() {
    return 1ui64;
}

