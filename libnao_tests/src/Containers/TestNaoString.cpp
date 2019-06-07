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

#include "Containers/TestNaoString.h"

#include <Containers/NaoString.h>

void TestNaoString::assignment_operators() {
    // Nothing should go wrong here

    NaoString str;

    constexpr char v[] = "abcdef";

    str = v;

    QCOMPARE(str, v);

    NaoString other;
    other = str;

    QCOMPARE(str, v);
}

void TestNaoString::conversion_operators() {
    // char* access and UTF-16 conversion
    // Also checks for null terminator

    constexpr char v[] = "TestNaoStringTestString";
    constexpr wchar_t v_l[] = L"TestNaoStringTestString";

    NaoString str(v);

    QVERIFY(memcmp(str, v, strlen(v) + 1) == 0);
    QVERIFY(memcmp(str.c_str(), v, strlen(v) + 1) == 0);
    QVERIFY(wmemcmp(str.utf16(), v_l, strlen(v) + 1) == 0);
}

