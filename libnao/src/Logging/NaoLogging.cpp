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

//#include "libnao.h"

#include "Logging/NaoLogging.h"

#include <vector>
#include <iostream>

#ifdef NAO_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#undef VC_EXTRALEAN
#undef WIN32_LEAN_AND_MEAN
#endif

//// Public

// Constructors
NaoLogger::NaoLogger(Destination dest, bool trailing_spaces, bool newline_on_destruct, bool disable_quote)
    : _m_destination(dest)
    , _m_trailing_spaces(trailing_spaces)
    , _m_newline_on_destruct(newline_on_destruct)
    , _m_disable_quote(disable_quote) {
    
}

// Destructor
// ReSharper disable bugprone-exception-escape
NaoLogger::~NaoLogger() {
    if (_m_newline_on_destruct) {
        _putchar('\n');
    }
}

//// Private

void NaoLogger::_putchar(char c) const {
    switch (_m_destination) {
        case STDOUT:
            std::cout.put(c);
            break;

        case STDERR:
            std::cerr.put(c);

        case DEBUG_WIN: {
            std::vector<char> vec({ c, '\0' });
            std::string str(vec.data());
#ifdef UNICODE
            std::wstring wstr(std::begin(str), std::end(str));
            OutputDebugStringW(wstr.data());
#else
            OutputDebugStringA(str.data());
#endif
        }
    }
}

void NaoLogger::_putchar(wchar_t c) const {
    switch (_m_destination) {
        case STDOUT:
            std::wcout.put(c);
            break;

        case STDERR:
            std::wcerr.put(c);

        case DEBUG_WIN: {
            std::vector<wchar_t> vec({ c, L'\0' });
            std::wstring str(vec.data());

            OutputDebugStringW(str.data());
        }
    }
}
