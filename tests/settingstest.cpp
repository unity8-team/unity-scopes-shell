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

#include <QObject>
#include <QTest>

#include <settingsmodel.h>

using namespace scopes_ng;
using namespace unity::shell::scopes;

class SettingsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
    }

    void cleanup()
    {
    }

    void testNoDepartments()
    {
        QVariant definitions;
        QScopedPointer<SettingsModelInterface> settingsModel(
                new SettingsModel("id", definitions));
    }

};

QTEST_GUILESS_MAIN(SettingsTest)
#include <settingstest.moc>
