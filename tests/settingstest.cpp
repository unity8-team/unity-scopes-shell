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

#include <QJsonDocument>
#include <QObject>
#include <QTemporaryDir>
#include <QTest>

#include <settingsmodel.h>

using namespace scopes_ng;
using namespace unity::shell::scopes;

namespace
{

const static QByteArray BOOLEAN_DEFINITION =
        R"(
[
    {
        "id": "enabledSetting",
        "displayName": "Enabled",
        "type": "boolean",
        "parameters": {
            "defaultValue": true
        }
    }
]
)";

const static QByteArray LIST_DEFINITION =
        R"(
[
    {
        "id": "unitTempSetting",
        "displayName": "Temperature Units",
        "type": "list",
        "parameters": {
            "defaultValue": 1,
            "values": ["Celcius", "Fahrenheit"]
        }
    }
]
)";

const static QByteArray NUMBER_DEFINITION =
        R"(
[
    {
        "id": "ageSetting",
        "displayName": "Age",
        "type": "number",
        "parameters": {
            "defaultValue": 23
        }
    }
]
)";

const static QByteArray STRING_DEFINITION =
        R"(
[
    {
        "id": "locationSetting",
        "displayName": "Location",
        "type": "string",
        "parameters": {
            "defaultValue": "London"
        }
    }
]
)";

const static QByteArray MIXED_DEFINITION =
        R"(
[
    {
        "id": "locationSetting",
        "displayName": "Location",
        "type": "string",
        "parameters": {
            "defaultValue": "London"
        }
    },
    {
        "id": "unitTempSetting",
        "displayName": "Temperature Units",
        "type": "list",
        "parameters": {
            "defaultValue": 1,
            "values": ["Celcius", "Fahrenheit"]
        }
    },
    {
        "id": "ageSetting",
        "displayName": "Age",
        "type": "number",
        "parameters": {
            "defaultValue": 23
        }
    },
    {
        "id": "enabledSetting",
        "displayName": "Enabled",
        "type": "boolean",
        "parameters": {
            "defaultValue": true
        }
    }
]
)";

class SettingsTest: public QObject
{
Q_OBJECT
private:
    QScopedPointer<QTemporaryDir> tempDir;

    QSharedPointer<SettingsModelInterface> settings;

    void newSettingsModel(const QString& id, const QByteArray& json)
    {
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QVariant definitions = doc.toVariant();
        settings.reset(new SettingsModel(tempDir->path(), id, definitions));
    }

private Q_SLOTS:
    void init()
    {
        tempDir.reset(new QTemporaryDir);
    }

    void cleanup()
    {
    }

    void testBooleanDefinition()
    {
        newSettingsModel("boolean", BOOLEAN_DEFINITION);
        QVERIFY(settings->rowCount() == 1);
    }

    void testListDefinition()
    {
        newSettingsModel("list", LIST_DEFINITION);
        QVERIFY(settings->rowCount() == 1);
    }

    void testNumberDefinition()
    {
        newSettingsModel("number", NUMBER_DEFINITION);
        QVERIFY(settings->rowCount() == 1);
    }

    void testStringDefinition()
    {
        newSettingsModel("string", STRING_DEFINITION);
        QVERIFY(settings->rowCount() == 1);
    }

    void testMixedDefinition()
    {
        newSettingsModel("mixed", MIXED_DEFINITION);
        QVERIFY(settings->rowCount() == 4);
    }
};

}
QTEST_GUILESS_MAIN(SettingsTest)
#include <settingstest.moc>
