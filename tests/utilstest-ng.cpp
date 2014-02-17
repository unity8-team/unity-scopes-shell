/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Michal Hruby <michal.hruby@canonical.com>
 */

#include <QObject>
#include <QTest>
#include <QFile>
#include <QScopedPointer>
#include <QSignalSpy>

#include <scopes-ng/utils.h>
#include <unity/scopes/Variant.h>

using namespace scopes_ng;
using namespace unity;

class UtilsTestNg : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testVariantConversions()
    {
        scopes::Variant v1("foo");
        QCOMPARE(scopeVariantToQVariant(v1).toString(), QString("foo"));
        scopes::Variant v2(7);
        QCOMPARE(scopeVariantToQVariant(v2).toInt(), 7);
        scopes::Variant v3(true);
        QCOMPARE(scopeVariantToQVariant(v3).toBool(), true);
        scopes::Variant v4(3.25);
        QCOMPARE(scopeVariantToQVariant(v4).toDouble(), 3.25);
        scopes::Variant v5;
        QCOMPARE(scopeVariantToQVariant(v5).isNull(), true);

        scopes::VariantArray arr;
        arr.push_back(v1);
        arr.push_back(v3);
        arr.push_back(v2);
        QVariantList list = scopeVariantToQVariant(scopes::Variant(arr)).toList();
        QCOMPARE(list.size(), 3);
        QCOMPARE(list[0].toString(), QString("foo"));
        QCOMPARE(list[1].toBool(), true);
        QCOMPARE(list[2].toInt(), 7);

        scopes::VariantMap vm;
        vm["first"] = v1;
        vm["2"] = v2;
        vm["last"] = v3;
        QVariantMap dict = scopeVariantToQVariant(scopes::Variant(vm)).toMap();
        QCOMPARE(dict.size(), 3);
        QCOMPARE(dict.value("first").toString(), QString("foo"));
        QCOMPARE(dict.value("2").toInt(), 7);
        QCOMPARE(dict.value("last").toBool(), true);
    }

    void testInvertedConversions()
    {
        scopes::Variant v1("foo");
        QVariant qv1(scopeVariantToQVariant(v1));
        QCOMPARE(qVariantToScopeVariant(qv1), v1);
        scopes::Variant v2(7);
        QVariant qv2(scopeVariantToQVariant(v2));
        QCOMPARE(qVariantToScopeVariant(qv2), v2);
        scopes::Variant v3(true);
        QVariant qv3(scopeVariantToQVariant(v3));
        QCOMPARE(qVariantToScopeVariant(qv3), v3);
        scopes::Variant v4(3.25);
        QVariant qv4(scopeVariantToQVariant(v4));
        QCOMPARE(qVariantToScopeVariant(qv4), v4);
        scopes::Variant v5;
        QVariant qv5(scopeVariantToQVariant(v5));
        QCOMPARE(qVariantToScopeVariant(qv5), v5);

        scopes::VariantArray arr;
        arr.push_back(v1);
        arr.push_back(v3);
        arr.push_back(v2);
        QVariantList list = scopeVariantToQVariant(scopes::Variant(arr)).toList();
        QCOMPARE(list.size(), 3);
        QCOMPARE(qVariantToScopeVariant(list[0]), v1);
        QCOMPARE(qVariantToScopeVariant(list[1]), v3);
        QCOMPARE(qVariantToScopeVariant(list[2]), v2);
        QCOMPARE(qVariantToScopeVariant(list), scopes::Variant(arr));

        scopes::VariantMap vm;
        vm["first"] = v1;
        vm["2"] = v2;
        vm["last"] = v3;
        QVariantMap dict = scopeVariantToQVariant(scopes::Variant(vm)).toMap();
        QCOMPARE(dict.size(), 3);
        QCOMPARE(qVariantToScopeVariant(dict.value("first")), v1);
        QCOMPARE(qVariantToScopeVariant(dict.value("2")), v2);
        QCOMPARE(qVariantToScopeVariant(dict.value("last")), v3);
        QCOMPARE(qVariantToScopeVariant(dict), scopes::Variant(vm));
    }
};

QTEST_MAIN(UtilsTestNg)
#include <utilstest-ng.moc>
