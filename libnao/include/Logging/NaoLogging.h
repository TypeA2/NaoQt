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

#ifdef N_WINDOWS
#   define ndebug NaoLogger(NaoLogger::DEBUG_WIN, true, true)
#else
#   define ndebug NaoLogger(NaoLogger::STDERR, true, true)
#endif

#if defined(N_WINDOWS) && defined(NAO_DEBUG)
#   define nerr NaoLogger(NaoLogger::DEBUG_WIN, true, true)
#   define nwarn NaoLogger(NaoLogger::DEBUG_WIN, true, true)
#   define nlog NaoLogger(NaoLogger::DEBUG_WIN, true, true)
#else
#   define nerr NaoLogger(NaoLogger::STDERR, true, true)
#   define nwarn NaoLogger(NaoLogger::STDERR, true, true)
#   define nlog NaoLogger(NaoLogger::STDOUT, true, true)
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
        bool trailing_spaces = true, bool newline_on_destruct = false);

    // Destructor
    // ReSharper disable once bugprone-exception-escape
    ~NaoLogger();

    // Logging functions

    void putchar(char c) const;
    void putchar(wchar_t c) const;
    void puts(const char* msg, bool disable_space = false) const;
    void puts(const wchar_t* msg, bool disable_space = false) const;

    void print(const char* msg) const;
    void print(const wchar_t* msg) const;
    void print(char msg) const;
    void print(wchar_t msg) const;
    void print(const NaoString& msg) const;

    void print(signed char n) const;
    void print(unsigned char n) const;
    void print(signed short n) const;
    void print(unsigned short n) const;
    void print(signed int n) const;
    void print(unsigned int n) const;
    void print(signed long n) const;
    void print(unsigned long n) const;
    void print(signed long long n) const;
    void print(unsigned long long n) const;

    void print(double n) const;
    void print(long double n) const;

    void print(bool v) const;

    template <typename T,
        typename = std::enable_if_t<
            !std::is_same_v<T, char>
            && !std::is_same_v<T, wchar_t>>>
    void print(T* msg) const {
        puts(typeid(T).name(), true);
        puts("(0x", true);
        puts(NaoString::number(uintptr_t(msg), 16), true);
        puts(")");
    }

    template <typename T>
    NaoLogger& operator<<(T val) {
        print(val);

        return *this;
    }

    NaoLogger& operator<<(const NaoString& val) {
        print(val);

        return *this;
    }

    private:
    Destination _m_destination;
    bool _m_trailing_spaces;
    bool _m_newline_on_destruct;

};
