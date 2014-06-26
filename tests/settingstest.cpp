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
        settings.reset(
                new SettingsModel(tempDir->path(), id, definitions, 0, 0));
    }

    void verifyData(int index, const QString& id, const QString& displayName,
            const QString& type, const QVariant& properties)
    {
        QCOMPARE(
                settings->data(settings->index(index),
                        SettingsModelInterface::RoleSettingId), QVariant(id));
        QCOMPARE(
                settings->data(settings->index(index),
                        SettingsModelInterface::RoleDisplayName),
                QVariant(displayName));
        QCOMPARE(
                settings->data(settings->index(index),
                        SettingsModelInterface::RoleType), QVariant(type));
        QCOMPARE(
                settings->data(settings->index(index),
                        SettingsModelInterface::RoleProperties), properties);
    }

    void verifyValue(int index, const QVariant& value)
    {
        // Using this "TRY" macro repeatedly attempts the comparison until a timeout
        QTRY_COMPARE(
                settings->data(settings->index(index),
                        SettingsModelInterface::RoleValue), value);
    }

    void setValue(int index, const QVariant& value)
    {
        settings->setData(settings->index(index), value,
                SettingsModelInterface::RoleValue);
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
        QCOMPARE(settings->rowCount(), 1);

        // Check the various properties make it through
        QVariantMap properties;
        properties["defaultValue"] = true;
        verifyData(0, "enabledSetting", "Enabled", "boolean", properties);

        verifyValue(0, true);
    }

    void testListDefinition()
    {
        newSettingsModel("list", LIST_DEFINITION);
        QCOMPARE(settings->rowCount(), 1);

        // Check the various properties make it through
        QVariantMap properties;
        properties["defaultValue"] = 1;
        properties["values"] = QVariantList() << "Celcius" << "Fahrenheit";
        verifyData(0, "unitTempSetting", "Temperature Units", "list",
                properties);

        // Check the default value
        verifyValue(0, 1);
    }

    void testNumberDefinition()
    {
        newSettingsModel("number", NUMBER_DEFINITION);
        QCOMPARE(settings->rowCount(), 1);

        // Check the various properties make it through
        QVariantMap properties;
        properties["defaultValue"] = 23;
        verifyData(0, "ageSetting", "Age", "number", properties);

        // Check the default value
        verifyValue(0, 23);
    }

    void testStringDefinition()
    {
        newSettingsModel("string", STRING_DEFINITION);
        QCOMPARE(settings->rowCount(), 1);

        // Check the various properties make it through
        QVariantMap properties;
        properties["defaultValue"] = "London";
        verifyData(0, "locationSetting", "Location", "string", properties);

        // Check the default value
        verifyValue(0, "London");
    }

    void testMixedDefinition()
    {
        newSettingsModel("mixed", MIXED_DEFINITION);
        QCOMPARE(settings->rowCount(), 4);

        {
            QVariantMap properties;
            properties["defaultValue"] = "London";
            verifyData(0, "locationSetting", "Location", "string", properties);
            verifyValue(0, "London");
        }
        {
            QVariantMap properties;
            properties["defaultValue"] = 1;
            properties["values"] = QVariantList() << "Celcius" << "Fahrenheit";
            verifyData(1, "unitTempSetting", "Temperature Units", "list",
                    properties);
            verifyValue(1, 1);
        }
        {
            QVariantMap properties;
            properties["defaultValue"] = 23;
            verifyData(2, "ageSetting", "Age", "number", properties);
            verifyValue(2, 23);
        }
        {
            QVariantMap properties;
            properties["defaultValue"] = true;
            verifyData(3, "enabledSetting", "Enabled", "boolean", properties);
            verifyValue(3, true);
        }
    }

    void testUpdateValue()
    {
        // Create an initial settings model
        newSettingsModel("update", MIXED_DEFINITION);

        // Verify the initial values
        verifyValue(0, "London");
        verifyValue(1, 1);
        verifyValue(2, 23);
        verifyValue(3, true);

        // Update the string value
        setValue(0, "Banana");
        verifyValue(0, "Banana");

        // Update the list value
        setValue(1, 0);
        verifyValue(1, 0);

        // Update the number value
        setValue(2, 123);
        verifyValue(2, 123);

        // Update the boolean value
        setValue(3, false);
        verifyValue(3, false);

        // Make a new settings model to check the settings were saved to disk
        newSettingsModel("update", MIXED_DEFINITION);
        verifyValue(0, "Banana");
        verifyValue(1, 0);
        verifyValue(2, 123);
        verifyValue(3, false);
    }
};

}
QTEST_GUILESS_MAIN(SettingsTest)
#include <settingstest.moc>
