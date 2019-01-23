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

#include "libnao.h"

#include "Containers/NaoString.h"

#include <string>

#ifdef NAO_WINDOWS
#   define nDebug NaoLogger(NaoLogger::DEBUG_WIN, true, true)
#else
#   define nDebug NaoLogger(NaoLogger::STDERR, true, true)
#endif

class LIBNAO_API NaoLogger {
    public:

    enum Destination {
        STDOUT,
        STDERR,
        DEBUG_WIN
    };

    // Constructors
    explicit NaoLogger(Destination dest = STDOUT,
        bool trailing_spaces = true, bool space_on_destruct = false);

    // ReSharper disable once bugprone-exception-escape
    // Destructor
    ~NaoLogger();

    // Logging functions
    template <typename T>
    void print(T msg, NAO_UNUSED bool quote = true) const {
        std::string str = std::to_string(msg);

        print(str.c_str(), false);
    }

    // Operator overloads
    template <typename T>
    NaoLogger& operator<<(T val) {
        print(val);

        return *this;
    }

    private:

    void _putchar(char c) const;
    void _putchar(wchar_t c) const;

    // Private member variables
    Destination _m_destination;
    bool _m_trailing_spaces;
    bool _m_space_on_destruct;
};

template <>
inline void NaoLogger::print(const char* msg, bool quote) const {

    if (quote) {
        _putchar('"');
    }

    while (*msg != '\0') {
        _putchar(*msg++);
    }

    if (quote) {
        _putchar('"');
    }

    if (_m_trailing_spaces) {
        _putchar(' ');
    }
}

template <>
inline void NaoLogger::print(const wchar_t* msg, bool quote) const {
    if (quote) {
        _putchar(L'"');
    }

    while (*msg != L'\0') {
        _putchar(*msg++);
    }
    if (quote) {
        _putchar(L'"');
    }

    if (_m_trailing_spaces) {
        _putchar(L'\0');
    }
}

// ReSharper disable performance-unnecessary-value-param

template <>
inline void NaoLogger::print(std::string msg, NAO_UNUSED bool quote) const {
    print(msg.c_str());
}

template <>
inline void NaoLogger::print(std::wstring msg, NAO_UNUSED bool quote) const {
    print(msg.c_str());
}

template <>
inline void NaoLogger::print(const std::string& msg, NAO_UNUSED bool quote) const {
    print(msg.c_str());
}

template <>
inline void NaoLogger::print(const std::wstring& msg, NAO_UNUSED bool quote) const {
    print(msg.c_str());
}

template <>
inline void NaoLogger::print(NaoString msg, NAO_UNUSED bool quote) const {
    print(msg.c_str());
}

template <>
inline void NaoLogger::print(const NaoString& msg, NAO_UNUSED bool quote) const {
    print(msg.c_str());
}

template <>
inline void NaoLogger::print(bool msg, NAO_UNUSED bool quote) const {
    print(msg ? "true" : "false", false);
}

// ReSharper restore performance-unnecessary-value-param
