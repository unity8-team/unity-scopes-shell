/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Pete Woods <pete.woods@canonical.com>
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
 */

#include <ubuntulocationservice.h>

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <iostream>

using namespace std;
using namespace scopes_ng;

class LocationPrinter: public QObject {
Q_OBJECT

public:
    LocationPrinter() {
        locationService.activate();
        connect(&locationService, &LocationService::locationChanged, this, &LocationPrinter::positionChanged);
    }

public Q_SLOTS:
    void positionChanged() {
        cout << "position:" << locationService.location().serialize_json() << endl;
    }

protected:
    UbuntuLocationService locationService;
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTimer::singleShot(3000, &app, SLOT(quit()));
    LocationPrinter printer;
    return app.exec();
}

#include "main.moc"
