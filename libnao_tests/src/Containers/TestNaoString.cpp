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

void TestNaoString::erase() {
    NaoString foo("FooBarBazA");

    // Normal erase
    QCOMPARE(*foo.erase(foo.begin() + 3, foo.begin() + 6), 'A');
    QCOMPARE(foo, "FooBazA");
    foo = "FooBarBaz";

    // Erase till end
    QCOMPARE(foo.erase(foo.begin() + 3, foo.end()), foo.end());
    QCOMPARE(foo, "Foo");
    foo = "FooBarBaz";

    // Erase nothing
    QCOMPARE(foo.erase(foo.begin(), foo.begin()), foo.begin());
    QCOMPARE(foo, "FooBarBaz");

    // Erase everything
    QCOMPARE(foo.erase(foo.begin(), foo.end()), foo.end());
    QVERIFY(foo.empty());
}

void TestNaoString::starts_ends_with() {
    Q_UNUSED(this);

    NaoString foo("FooBarBaz");

    QVERIFY(foo.starts_with(NaoString("FooBar")));
    QVERIFY(foo.starts_with("FooBar"));
    QVERIFY(foo.starts_with('F'));
    QVERIFY(foo.starts_with("FooBarBaz"));
    QVERIFY(foo.starts_with(""));
    QVERIFY(foo.starts_with(NaoString()));
    QVERIFY(!foo.starts_with("FooBaz"));
    QVERIFY(!foo.starts_with('P'));
    QVERIFY(!NaoString().starts_with('\0'));

    QVERIFY(foo.ends_with(NaoString("BarBaz")));
    QVERIFY(foo.ends_with("BarBaz"));
    QVERIFY(foo.ends_with('z'));
    QVERIFY(foo.ends_with(""));
    QVERIFY(foo.ends_with(NaoString()));
    QVERIFY(foo.ends_with("FooBarBaz"));
    QVERIFY(!foo.ends_with("FooBar"));
    QVERIFY(!foo.ends_with('B'));
    QVERIFY(!NaoString().ends_with('\0'));
}

void TestNaoString::extra_utility() {
    NaoString foo("FooBarBaz");

    // Entire string
    QCOMPARE(foo.substr(0), "FooBarBaz");

    // Left
    QCOMPARE(foo.substr(0, 3), "Foo");

    // Center
    QCOMPARE(foo.substr(3, 3), "Bar");
    
    // Right
    QCOMPARE(foo.substr(6), "Baz");

    // Empty substring
    QCOMPARE(foo.substr(0, 0), "");

    QCOMPARE(foo.last_pos_of('B'), foo.begin() + 6);
    QCOMPARE(foo.last_pos_of('F'), foo.begin());
    QCOMPARE(foo.last_pos_of('Q'), foo.end());

    QCOMPARE(foo.last_index_of('B'), 6);
    QCOMPARE(foo.last_index_of('F'), 0);
    QCOMPARE(foo.last_index_of('Q'), foo.size());

    QVERIFY(foo.contains('F'));
    QVERIFY(foo.contains('z'));
    QVERIFY(!foo.contains('Q'));

    QCOMPARE(foo.replace('o', 'e'), 2);
    QCOMPARE(foo, "FeeBarBaz");

    QCOMPARE(foo.replace('l', 'q'), 0);
    QCOMPARE(foo, "FeeBarBaz");
}

