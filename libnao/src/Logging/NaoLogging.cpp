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

#ifdef N_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   define VC_EXTRALEAN
#   include <Windows.h>
#   undef VC_EXTRALEAN
#   undef WIN32_LEAN_AND_MEAN
#endif

//// Public

// Constructors
NaoLogger::NaoLogger(Destination dest, bool trailing_spaces, bool newline_on_destruct)
    : _m_destination(dest)
    , _m_trailing_spaces(trailing_spaces)
    , _m_newline_on_destruct(newline_on_destruct) {
    
}

// ReSharper disable once bugprone-exception-escape
NaoLogger::~NaoLogger() {
    if (_m_newline_on_destruct) {
        putchar('\n');
    }
}

void NaoLogger::putchar(char c) const {
    switch (_m_destination) {
        case STDOUT:
            std::cout.put(c);
            break;

        case STDERR:
            std::cerr.put(c);
            break;

        case DEBUG_WIN:
            OutputDebugStringA(std::vector<char> { c, '\0' }.data());
            break;
    }
}

void NaoLogger::putchar(wchar_t c) const {
    switch (_m_destination) {
        case STDOUT:
            std::wcout.put(c);
            break;

        case STDERR:
            std::wcerr.put(c);
            break;

        case DEBUG_WIN:
            OutputDebugStringW(std::vector<wchar_t> { c, L'\0' }.data());
            break;
    }
}

void NaoLogger::puts(const char* msg, bool disable_space) const {
    switch (_m_destination) {
        case STDOUT:
            std::cout << msg;
            break;

        case STDERR:
            std::cerr << msg;
            break;

        case DEBUG_WIN:
            OutputDebugStringA(msg);
            break;
    }

    if (!disable_space && _m_trailing_spaces) {
        switch (_m_destination) {
            case STDOUT:
                std::cout.put(' ');
                break;

            case STDERR:
                std::cerr.put(' ');
                break;

            case DEBUG_WIN:
                OutputDebugStringA(" ");
                break;
        }
    }
}

void NaoLogger::puts(const wchar_t* msg, bool disable_space) const {
    switch (_m_destination) {
        case STDOUT:
            std::wcout << msg;
            break;

        case STDERR:
            std::wcerr << msg;
            break;

        case DEBUG_WIN:
            OutputDebugStringW(msg);
            break;
    }

    if (!disable_space && _m_trailing_spaces) {
        switch (_m_destination) {
            case STDOUT:
                std::wcout.put(L' ');
                break;

            case STDERR:
                std::wcerr.put(L' ');
                break;

            case DEBUG_WIN:
                OutputDebugStringW(L" ");
                break;
        }
    }
}

#pragma region Text types

inline void NaoLogger::print(const char* msg) const {
    puts(msg);
}

inline void NaoLogger::print(const wchar_t* msg) const {
    puts(msg);
}

inline void NaoLogger::print(char msg) const {
    putchar(msg);
}

inline void NaoLogger::print(wchar_t msg) const {
    putchar(msg);
}

inline void NaoLogger::print(const NaoString& msg) const {
    puts(msg);
}


#pragma endregion

#pragma region Numeric types

inline void NaoLogger::print(signed char n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(unsigned char n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(signed short n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(unsigned short n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(signed int n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(unsigned int n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(signed long n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(unsigned long n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(signed long long n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(unsigned long long n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(double n) const {
    puts(NaoString::number(n));
}

inline void NaoLogger::print(long double n) const {
    puts(NaoString::number(n));
}

#pragma endregion

void NaoLogger::print(bool v) const {
    puts(v ? "true" : "false");
}
