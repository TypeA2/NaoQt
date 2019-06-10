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

#include <QtTest/QtTest>

class TestNaoString : public QObject {
    Q_OBJECT

    private slots:
    void wstring();

    void constructors();
    void comparison_operators();
    void assignment_operators();
    void conversion_operators();
    void append();
    void erase();
    void starts_ends_with();
    void extra_utility();
    void memory();
    void iterators();
    void utility();
    void statics();
    void stl();
    void operators();
};
