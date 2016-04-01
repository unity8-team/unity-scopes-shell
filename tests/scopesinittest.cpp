/*
 * Copyright (C) 2016 Canonical, Ltd.
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
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <QSignalSpy>
#include <QTest>
#include <scopes.h>
#include <scope-harness/registry/pre-existing-registry.h>
#include <scope-harness/test-utils.h>

using namespace unity::scopeharness;
using namespace unity::scopeharness::registry;
using namespace scopes_ng;

class ScopesInitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase()
    {
        m_registry.reset(new PreExistingRegistry(TEST_RUNTIME_CONFIG));
        m_registry->start();
    }

    void cleanupTestCase()
    {
        m_registry.reset();
    }

    void init()
    {
       const QStringList favs {"scope://mock-scope-filters"};
       TestUtils::setFavouriteScopes(favs);
    }

    void cleanup()
    {
        qputenv("LC_ALL", "C");
    }

    void testBrokenLocale()
    {
        qputenv("LC_ALL", "");
        qputenv("LC_MONETARY", "bad");

        QScopedPointer<Scopes> scopes(new Scopes(nullptr));

        QSignalSpy spy(scopes.data(), SIGNAL(loadedChanged()));
        QVERIFY(spy.wait());

        QCOMPARE(scopes->loaded(), true);
        QCOMPARE(scopes->count(), 0);
        QVERIFY(scopes->overviewScope() != nullptr);
    }

private:
    Registry::UPtr m_registry;
};

QTEST_GUILESS_MAIN(ScopesInitTest)
#include <scopesinittest.moc>
