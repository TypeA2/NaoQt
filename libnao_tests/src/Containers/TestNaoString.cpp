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
#include <Filesystem/Filesystem.h>

void TestNaoString::wstring() {
    Q_UNUSED(this);

    NaoWStringConst str = NaoString("FooBarBaz").utf16();
    constexpr wchar_t wide[] = L"FooBarBaz";

    QVERIFY(wmemcmp(str, wide, wcslen(wide) + 1) == 0);
    QVERIFY(wmemcmp(str.data(), wide, wcslen(wide) + 1) == 0);
    QVERIFY(wmemcmp(str.utf16(), wide, wcslen(wide) + 1) == 0);
    QVERIFY(wmemcmp(str.c_str(), wide, wcslen(wide) + 1) == 0);
}

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
    QVERIFY(NaoString("Foo") != "Bar");

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
    QCOMPARE(NaoString("FooBa").append('r'), "FooBar");

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

void TestNaoString::memory() {
    NaoString str("FooBar");
    QCOMPARE(str, "FooBar");
    str.clear();
    QVERIFY(str.empty());
    QVERIFY(*str.data() == '\0');
    QVERIFY(str.capacity() == NaoString::data_alignment);
    str.reserve(NaoString::data_alignment + 1);
    QVERIFY(str.capacity() == NaoString::data_alignment * 2);
}

void TestNaoString::iterators() {
    Q_UNUSED(this);

    const NaoString str("FooBar");
    const char* begin = str.data();
    const char* end = str.data() + str.size() + 1;

    QVERIFY(begin != end);
    QVERIFY(*begin == 'F');
    QVERIFY(*end == '\0');
    QVERIFY(std::distance(begin, end) == ptrdiff_t(str.size() + 1));
    QVERIFY(str.begin() == begin);
    QVERIFY(str.cbegin() == begin);
    QCOMPARE(str.end(), end);
    QCOMPARE(str.cend(), end);
}

void TestNaoString::utility() {
    Q_UNUSED(this);

    NaoString str("FooBar");
    QVERIFY(str.copy() == "FooBar");
    QVERIFY(str.copy().data() != str.data());

    QVERIFY(str[0] == 'F');
    QVERIFY(str[3] == 'B');
    QVERIFY(&str[0] == str.data());
    QVERIFY_EXCEPTION_THROWN((void) str[-1], std::out_of_range);
    QVERIFY_EXCEPTION_THROWN((void) str[str.size() + 1], std::out_of_range);

    const NaoString str_c(str);

    QVERIFY(str_c[0] == 'F');
    QVERIFY(str_c[3] == 'B');
    QVERIFY(&str_c[0] == str_c.data());
    QVERIFY_EXCEPTION_THROWN((void) str_c[-1], std::out_of_range);
    QVERIFY_EXCEPTION_THROWN((void) str_c[str.size() + 1], std::out_of_range);

    QVERIFY_EXCEPTION_THROWN((void) str.substr(str.size()), std::out_of_range);

    NaoString str2("F");

    QVERIFY(str2.last_pos_of('F') == str2.begin());
    QVERIFY(str2.last_pos_of('Y') == str2.end());
}

void TestNaoString::statics() {
    QVERIFY(NaoString::number(123) == "123");
    QVERIFY(NaoString::number(0x123, 16) == "123");
    QVERIFY(NaoString::number(456U) == "456");
    QVERIFY(NaoString::number(789L) == "789");
    QVERIFY(NaoString::number(789UL) == "789");

    QVERIFY(NaoString::number(1.234567) == "1.23457");
    QVERIFY(NaoString::number(1.234567L, 3) == "1.23");

    QVERIFY(NaoString::bytes(0x1000000000000001) == "1 EiB");
    QVERIFY(NaoString::bytes(0x4000000000001) == "1 PiB");
    QVERIFY(NaoString::bytes(0x10000000001) == "1 TiB");
    QVERIFY(NaoString::bytes(0x40000001) == "1 GiB");
    QVERIFY(NaoString::bytes(0x100001) == "1 MiB");
    QVERIFY(NaoString::bytes(0x401) == "1 KiB");
    QVERIFY(NaoString::bytes(15) == "15 bytes");

    QVERIFY(NaoString::fromUTF8("FooBar") == "FooBar");
    QVERIFY(NaoString::fromWide(L"FooBar") == "FooBar");
    QVERIFY(NaoString::fromShiftJIS("\x46\x6F\x6F\x42\x61\x72") == "FooBar");
}

void TestNaoString::stl() {
    Q_UNUSED(this);

    QVERIFY(NaoString(std::string("Foo")) == "Foo");

    NaoString str;
    str = std::string("Bar");

    QVERIFY(str == "Bar");

    // To force std::string conversion
    auto tmp = [](const std::string& s) -> const std::string& { return s; };

    QVERIFY(tmp(str) == "Bar");

    str = NaoString(fs::path("/usr"));
    QVERIFY(str == "/usr");

    str = fs::path("/home");
    QVERIFY(str == "/home");

    // To force std::path conversion
    auto tmp2 = [](const fs::path& p) -> const fs::path & { return p; };

    QVERIFY(tmp2(str) == "/home");

    QVERIFY(NaoString("/usr\\bin").normalize_path() == "\\usr\\bin");

    NaoString dirty("/home\\:a/?b\"/cd<e>|f/");
    NaoString dirty2 = dirty;

    QVERIFY(dirty.clean_path() == "/home\\_a/_b_/cd_e__f/");
    QVERIFY(dirty2.clean_dir_name() == "/home\\_a/_b_/cd_e__f/");
}

void TestNaoString::operators() {
    Q_UNUSED(this);

    NaoString a = "Foo";
    NaoString b = "Bar";

    QVERIFY(a + b == "FooBar");
    QVERIFY(a + "Baz" == "FooBaz");
    QVERIFY(a + 'B' == "FooB");
    QVERIFY("Foo" + b == "FooBar");
    QVERIFY('F' + b == "FBar");
}
