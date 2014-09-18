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

#include "ubuntulocationservice.h"

#include <QProcess>
#include <QSignalSpy>
#include <QTest>
#include <string>

#include <libqtdbusmock/DBusMock.h>
#include <libqtdbustest/DBusTestRunner.h>

using namespace std;
using namespace scopes_ng;
using namespace unity::scopes;
using namespace QtDBusTest;
using namespace QtDBusMock;

namespace
{

static const char* LOCATION_SERVICE_NAME = "com.ubuntu.location.Service";
static const char* LOCATION_SERVICE_PATH = "/com/ubuntu/location/Service";
static const char* LOCATION_SERVICE_INTERFACE = "com.ubuntu.location.Service";

static const char* SESSION_NAME = "com.ubuntu.location.Service.Session";
static const QString SESSION_PATH = "/com/ubuntu/location/session/%1";
static const QString SESSION_INTERFACE = "com.ubuntu.location.Service.Session";

static Variant geoip()
{
    VariantMap result;
    result["area_code"] = "0";
    result["city"] = "Accrington";
    result["country_code"] = "GB";
    result["country_name"] = "United Kingdom";
    result["horizontal_accuracy"] = 100000.0;
    result["latitude"] = 55.76540;
    result["longitude"] = -2.74670;
    result["region_code"] = "H2";
    result["region_name"] = "Lancashire";
    result["zip_postal_code"] = "BB5";
    return Variant(result);
}

static Variant gps()
{
    VariantMap result;
    result["altitude"] = 3.0;
    result["area_code"] = "0";
    result["city"] = "Accrington";
    result["country_code"] = "GB";
    result["country_name"] = "United Kingdom";
    result["horizontal_accuracy"] = 4.0;
    result["latitude"] = 1.0;
    result["longitude"] = 2.0;
    result["region_code"] = "H2";
    result["region_name"] = "Lancashire";
    result["vertical_accuracy"] = 5.0;
    result["zip_postal_code"] = "BB5";
    return Variant(result);
}

class LocationTest: public QObject
{
Q_OBJECT

public:
    LocationTest() :
            mock(dbus)
    {
    }

private:
    OrgFreedesktopDBusMockInterface& interface()
    {
        return mock.mockInterface(LOCATION_SERVICE_NAME, LOCATION_SERVICE_PATH,
                                  LOCATION_SERVICE_INTERFACE,
                                  QDBusConnection::SystemBus);
    }

    OrgFreedesktopDBusMockInterface& session(int id)
    {
        return mock.mockInterface(SESSION_NAME, SESSION_PATH.arg(id),
                                  SESSION_INTERFACE, QDBusConnection::SystemBus);
    }

    DBusTestRunner dbus;

    DBusMock mock;

    QScopedPointer<LocationService> locationService;

    QProcess geoIpServer;

    QUrl url;

private Q_SLOTS:
    void initTestCase()
    {
        DBusMock::registerMetaTypes();

        // Register the main interface object
        mock.registerCustomMock(LOCATION_SERVICE_NAME, LOCATION_SERVICE_PATH,
                                LOCATION_SERVICE_INTERFACE,
                                QDBusConnection::SystemBus);

        // Register the first session object
        mock.registerCustomMock(SESSION_NAME, SESSION_PATH.arg(0),
                                SESSION_INTERFACE,
                                QDBusConnection::SystemBus);

        dbus.startServices();

        // Set up the main interface
        {
            interface().AddMethod(LOCATION_SERVICE_INTERFACE,
                                "CreateSessionForCriteria", "bbbbdbbb", "o",
                                "ret='/com/ubuntu/location/session/0'");
        }

        // Set up the first session
        {
            session(0).AddMethod(SESSION_INTERFACE, "StartPositionUpdates", "", "", "");
            session(0).AddMethod(SESSION_INTERFACE, "StopPositionUpdates", "", "", "");
        }

        geoIpServer.setProcessChannelMode(QProcess::ForwardedErrorChannel);
        geoIpServer.start(GEOIP_SERVER_BINARY);
        QVERIFY(geoIpServer.waitForStarted());
        QVERIFY(geoIpServer.waitForReadyRead());

        url = "http://127.0.0.1:" + geoIpServer.readAllStandardOutput().trimmed() + "/lookup";
    }

    void cleanupTestCase() {
        geoIpServer.terminate();
        QVERIFY(geoIpServer.waitForFinished());
    }

    void init()
    {
        locationService.reset(new UbuntuLocationService(GeoIp::Ptr(new GeoIp(url))));
    }

    void cleanup()
    {
        locationService.reset();
    }

    void compareVariant(const Variant& expected_in, const Variant& actual_in)
    {
        VariantMap expected(expected_in.get_dict());
        VariantMap actual(actual_in.get_dict());

        QCOMPARE(expected.size(), actual.size());
        for(const auto entry: expected)
        {
            QVERIFY(actual.find(entry.first) != actual.end());

            Variant expectedVariant = entry.second;
            Variant actualVariant = actual[entry.first];

            if (expectedVariant.which() == Variant::Double)
            {
                bool comparison = qFuzzyCompare(expectedVariant.get_double(), actualVariant.get_double());
                if (!comparison)
                {
                    qWarning() << "Comparison:" << expectedVariant.get_double() << "!=" << actualVariant.get_double();
                }
                QVERIFY(comparison);
            }
            else
            {
                QCOMPARE(expectedVariant, actualVariant);
            }
        }
    }

    void testLocation()
    {
        QSignalSpy spy(locationService.data(), SIGNAL(locationChanged()));
        locationService->activate();

        // The GeoIP HTTP call should return now
        QVERIFY(spy.wait());
        compareVariant(geoip(), Variant(locationService->location().serialize()));

        // Call the object that the location service client creates
        QDBusMessage reply = QDBusConnection::systemBus().call(
                QDBusMessage::createMethodCall(":1.4", SESSION_PATH.arg(0),
                                               SESSION_INTERFACE,
                                               "UpdatePosition")
                    << 1.0 << 2.0 << true << 3.0 << true << 4.0 << true << 5.0 << qint64(1234));
        QCOMPARE(QString(), reply.errorMessage());

        // The GPS update should return now
        QVERIFY(spy.wait());
        compareVariant(gps(), Variant(locationService->location().serialize()));
    }
};

}
QTEST_GUILESS_MAIN(LocationTest)
#include <locationtest.moc>
