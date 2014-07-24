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
 *  Michal Hruby <michal.hruby@canonical.com>
 */

#include <QObject>
#include <QTest>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QScopedPointer>
#include <QSignalSpy>

#define SCOPES_TMP_ENDPOINT_DIR "/tmp/scopes-test-endpoints"

class RegistrySpawner
{
public:
    RegistrySpawner()
    {
        QDir endpointdir(QFileInfo(TEST_RUNTIME_CONFIG).dir());
        endpointdir.cd(QString("endpoints"));
        QFile::remove(SCOPES_TMP_ENDPOINT_DIR);
        // symlinking to workaround "File name too long" issue
        QVERIFY2(QFile::link(endpointdir.absolutePath(), SCOPES_TMP_ENDPOINT_DIR),
            "Unable to create symlink " SCOPES_TMP_ENDPOINT_DIR);
        // startup our private scope registry
        QString registryBin(TEST_SCOPEREGISTRY_BIN);
        QStringList arguments;
        arguments << TEST_RUNTIME_CONFIG;

        m_registry.reset(new QProcess(nullptr));
        m_registry->setProcessChannelMode(QProcess::ForwardedChannels);
        m_registry->start(registryBin, arguments);
        QVERIFY(m_registry->waitForStarted());

        qputenv("UNITY_SCOPES_RUNTIME_PATH", TEST_RUNTIME_CONFIG);
        qputenv("UNITY_SCOPES_NO_LOCATION", "1");
    }

    ~RegistrySpawner()
    {
        if (m_registry) {
            m_registry->terminate();
            if (!m_registry->waitForFinished()) {
                m_registry->kill();
            }
        }
        QFile::remove(SCOPES_TMP_ENDPOINT_DIR);
    }

private:
    QScopedPointer<QProcess> m_registry;
};
