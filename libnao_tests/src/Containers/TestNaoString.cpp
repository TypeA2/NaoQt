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

void TestNaoString::constructors() {
    QCOMPARE(NaoString(), "");
    QCOMPARE(NaoString("Foo"), "Foo");
    QCOMPARE(NaoString('X'), "X");

    NaoString src("Bar");
    NaoString tgt(std::move(src));

    QCOMPARE(tgt, "Bar");

    // ReSharper disable once bugprone-use-after-move
    QCOMPARE(src.data(), nullptr);
}

void TestNaoString::comparison_operators() {
    QCOMPARE(NaoString("FooBar"), "FooBar");

    QCOMPARE(NaoString("Foo"), NaoString("Foo"));

    QVERIFY(!(NaoString("Bar") == NaoString("Baz")));
    QVERIFY(!(NaoString("Foo") != NaoString("Foo")));
    QVERIFY(NaoString("Foo") != NaoString("Bar"));
    QVERIFY(!(NaoString("Y") != 'Y'));

    QCOMPARE(NaoString("A"), 'A');
}

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
    // Make ReSharper shut up
    Q_UNUSED(this);

    // char* access and UTF-16 conversion
    // Also checks for null terminator

    constexpr char v[] = "TestNaoStringTestString";
    constexpr wchar_t v_l[] = L"TestNaoStringTestString";

    NaoString str(v);

    QVERIFY(memcmp(str, v, strlen(v) + 1) == 0);
    QVERIFY(memcmp(str.c_str(), v, strlen(v) + 1) == 0);
    QVERIFY(wmemcmp(str.utf16(), v_l, strlen(v) + 1) == 0);
}

void TestNaoString::append() {
    QCOMPARE(NaoString("Foo").append(NaoString("Bar")), "FooBar");
    QCOMPARE(NaoString("Foo").append(NaoString("BarBaz"), 3), "FooBar");
    QCOMPARE(NaoString("Foo").append("Bar"), "FooBar");
    QCOMPARE(NaoString("Foo").append("BarBaz", 3), "FooBar");

    QVERIFY_EXCEPTION_THROWN(NaoString("Foo").append(NaoString("Bar"), 4), std::out_of_range);
    QVERIFY_EXCEPTION_THROWN(NaoString("Foo").append("Bar", 4), std::out_of_range);
}
