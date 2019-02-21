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

#ifndef N_LOG_ID
#define N_LOG_ID "Unknown"
#endif

#ifdef N_WINDOWS
#   define ndebug NaoLogger(NaoLogger::DEBUG_WIN, NaoLogger::Debug, true, true) << "[" N_LOG_ID "]"
#else
#   define ndebug NaoLogger(NaoLogger::STDERR, NaoLogger::Debug, true, true) << "[" N_LOG_ID "]"
#endif

#if defined(N_WINDOWS) && defined(NAO_DEBUG)
#   define nerr NaoLogger(NaoLogger::DEBUG_WIN, NaoLogger::Error, true, true) << "[" N_LOG_ID "]"
#   define nwarn NaoLogger(NaoLogger::DEBUG_WIN, NaoLogger::Warning, true, true) << "[" N_LOG_ID "]"
#   define nlog NaoLogger(NaoLogger::DEBUG_WIN, NaoLogger::Log, true, true) << "[" N_LOG_ID "]"
#else
#   define nerr NaoLogger(NaoLogger::STDERR, NaoLogger::Error, true, true) << "[" N_LOG_ID "]"
#   define nwarn NaoLogger(NaoLogger::STDERR, NaoLogger::Warning, true, true) << "[" N_LOG_ID "]"
#   define nlog NaoLogger(NaoLogger::STDOUT, NaoLogger::Log, true, true) << N_LOG_ID
#endif

class LIBNAO_API NaoLogger {
    public:

    enum Destination {
        STDOUT,
        STDERR,
        DEBUG_WIN
    };

    enum LogLevel {
        Log,
        Warning,
        Error,
        Debug
    };

    // Constructors
    explicit NaoLogger(Destination dest = STDOUT, LogLevel level = Error,
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
    LogLevel _m_level;
    bool _m_trailing_spaces;
    bool _m_newline_on_destruct;
};

class NaoLoggingPrivate;

class LIBNAO_API NaoLogging {
    public:
    static void set_log_file(const NaoString& target = NaoString());

    ~NaoLogging();

    private:
    friend class NaoLogger;

    NaoLogging();

    static NaoLogging& instance();

    void write(const char* str);
    void write(const wchar_t* str);
    bool flush();

    NaoLoggingPrivate* d_ptr;
};
