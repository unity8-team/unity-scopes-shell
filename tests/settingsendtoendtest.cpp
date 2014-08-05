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

#include "registry-spawner.h"

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
        m_registry.reset(new RegistrySpawner);
    }

    void cleanupTestCase()
    {
        m_registry.reset();
    }

    void init()
    {
        m_scopes.reset(new Scopes(nullptr));
        // no scopes on startup
        QCOMPARE(m_scopes->rowCount(), 0);
        QCOMPARE(m_scopes->loaded(), false);
        QSignalSpy spy(m_scopes.data(), SIGNAL(loadedChanged()));
        // wait till the registry spawns
        QVERIFY(spy.wait());
        QCOMPARE(m_scopes->loaded(), true);
        // should have at least one scope now
        QCOMPARE(m_scopes->rowCount(), 4);

        // get scope proxy
        m_scope = qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(QString("mock-scope")));
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
};

QTEST_GUILESS_MAIN(SettingsEndToEndTest)
#include <settingsendtoendtest.moc>
