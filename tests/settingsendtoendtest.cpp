/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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
 *  Pete Woods <pete.woods@canonical.com>
 */

#include <QTest>
#include <QScopedPointer>
#include <QSignalSpy>

#include <scopes.h>
#include <scope.h>

#include <unity/shell/scopes/SettingsModelInterface.h>

#include "scope-harness/registry-spawner.h"
#include "scope-harness/test-utils.h"

using namespace scopes_ng;
using namespace unity::shell::scopes;

class SettingsEndToEndTest : public QObject
{
    Q_OBJECT
private:
    QScopedPointer<Scopes> m_scopes;
    Scope* m_scope;
    QScopedPointer<RegistrySpawner> m_registry;

private Q_SLOTS:
    void initTestCase()
    {
        m_registry.reset(new RegistrySpawner(TEST_RUNTIME_CONFIG));
    }

    void cleanupTestCase()
    {
        m_registry.reset();
    }

    void init()
    {
        const QStringList favs {"scope://mock-scope-departments", "scope://mock-scope-double-nav", "scope://mock-scope"};
        setFavouriteScopes(favs);

        m_scopes.reset(new Scopes(nullptr));
        // no scopes on startup
        QCOMPARE(m_scopes->rowCount(), 0);
        QCOMPARE(m_scopes->loaded(), false);
        QSignalSpy spy(m_scopes.data(), SIGNAL(loadedChanged()));
        // wait till the registry spawns
        QVERIFY(spy.wait());
        QCOMPARE(m_scopes->loaded(), true);
        // should have at least one scope now
        QVERIFY(m_scopes->rowCount() > 1);

        // get scope proxy
        m_scope = m_scopes->getScopeById("mock-scope");
        QVERIFY(m_scope != nullptr);
        m_scope->setActive(true);
    }

    void cleanup()
    {
        m_scopes.reset();
        m_scope = nullptr;
    }

    void verifySetting(const SettingsModelInterface* settings, int index,
            const QString& id, const QString& displayName, const QString& type,
            const QVariantMap& properties, const QVariant& value)
    {
        QCOMPARE(id, settings->data(settings->index(index), SettingsModelInterface::RoleSettingId).toString());
        QCOMPARE(displayName, settings->data(settings->index(index), SettingsModelInterface::RoleDisplayName).toString());
        QCOMPARE(type, settings->data(settings->index(index), SettingsModelInterface::RoleType).toString());

        QCOMPARE(properties, settings->data(settings->index(index), SettingsModelInterface::RoleProperties).toMap());
        QCOMPARE(value, settings->data(settings->index(index), SettingsModelInterface::RoleValue));
    }

    void testBasic()
    {
        const auto settings = m_scope->settings();
        QVERIFY(settings);

        // Wait for the settings model to initialize
        if(settings->count() == 0)
        {
            QSignalSpy settingsCountSpy(settings, SIGNAL(countChanged()));
            QVERIFY(settingsCountSpy.wait());
        }

        QCOMPARE(settings->count(), 5);

        verifySetting(settings, 0, "location", "Location", "string", QVariantMap({ {"defaultValue", "London"} }), "London");
        verifySetting(settings, 1, "distanceUnit", "Distance Unit", "list", QVariantMap({ {"defaultValue", 1}, {"values", QStringList({"Kilometers", "Miles"})} }), 1);
        verifySetting(settings, 2, "age", "Age", "number", QVariantMap({ {"defaultValue", 23} }), 23);
        verifySetting(settings, 3, "enabled", "Enabled", "boolean", QVariantMap({ {"defaultValue", true} }), true);
        verifySetting(settings, 4, "color", "Color", "string", QVariantMap({ {"defaultValue", QVariant()} }), QVariant());
    }

    void setValue(SettingsModelInterface *settings, int index, const QVariant& value)
    {
        QVERIFY(settings);

        QVERIFY(settings->setData(settings->index(index), value,
                SettingsModelInterface::RoleValue));
    }

    void verifySettingsChangedSignal()
    {
        auto settings = m_scope->settings();
        QVERIFY(settings);

        // Wait for the settings model to initialize
        if(settings->count() == 0)
        {
            QSignalSpy settingsCountSpy(settings, SIGNAL(countChanged()));
            QVERIFY(settingsCountSpy.wait());
        }

        QSignalSpy settingChangedSpy(settings, SIGNAL(settingsChanged()));

        // Before changing any value check that the results are not marked as dirty
        // Also set the scope as inactive so we can check that it gets marked as dirty
        m_scope->setActive(false);
        QCOMPARE(m_scope->isActive(), false);
        QCOMPARE(m_scope->resultsDirty(), false);

        // change a value and wait for the signal
        setValue(settings, 0, "Barcelona");
        QVERIFY(settingChangedSpy.wait());

        // Check that the results are dirty
        QCOMPARE(m_scope->resultsDirty(), true);
    }
};

QTEST_GUILESS_MAIN(SettingsEndToEndTest)
#include <settingsendtoendtest.moc>
