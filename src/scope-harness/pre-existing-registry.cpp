/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <scope-harness/pre-existing-registry.h>
#include <scope-harness/test-utils.h>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QThread>
#include <QSettings>
#include <QSignalSpy>

namespace unity
{
namespace scopeharness
{

PreExistingRegistry::PreExistingRegistry(const std::string &runtimeConfig) :
        m_runtimeConfig(QString::fromStdString(runtimeConfig))
{
    QSettings runtimeSettings(m_runtimeConfig, QSettings::IniFormat);
    runtimeSettings.setIniCodec("UTF-8");

    QString zmqConfig = runtimeSettings.value("Runtime/Zmq.ConfigFile").toString();
    QSettings zmqSettings(zmqConfig, QSettings::IniFormat);
    zmqSettings.setIniCodec("UTF-8");

    m_endpointDir = zmqSettings.value("Zmq/EndpointDir").toString();
}

void PreExistingRegistry::start()
{
    qputenv("UNITY_SCOPES_CONFIG_DIR", m_tempDir.path().toUtf8());
    qputenv("TEST_DESKTOP_FILES_DIR", "");

    m_endpointDir.removeRecursively();
    m_endpointDir.mkpath(".hidden");

    // startup our private scope registry
    QString registryBin(SCOPESLIB_SCOPEREGISTRY_BIN);

    m_registryProcess.reset(new QProcess());
    m_registryProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    m_registryProcess->start(registryBin, QStringList() << m_runtimeConfig);
    throwIfNot(m_registryProcess->waitForStarted(), "Scope registry failed to start");

    // FIXME hard-coded path
    QProcess::startDetached(
            "/usr/lib/" DEB_HOST_MULTIARCH "/libqtdbustest/watchdog",
            QStringList() << QString::number(QCoreApplication::applicationPid())
                    << QString::number(m_registryProcess->pid()));

    qputenv("UNITY_SCOPES_TYPING_TIMEOUT_OVERRIDE", "0");
    qputenv("UNITY_SCOPES_LIST_DELAY", "5");
    qputenv("UNITY_SCOPES_RESULTS_TTL_OVERRIDE", "250");
    qputenv("UNITY_SCOPES_RUNTIME_PATH", m_runtimeConfig.toUtf8());
    qputenv("UNITY_SCOPES_NO_LOCATION", "1");
}

PreExistingRegistry::~PreExistingRegistry()
{
    if (m_registryProcess)
    {
        m_registryProcess->terminate();
        if (!m_registryProcess->waitForFinished())
        {
            m_registryProcess->kill();
        }
    }
    m_endpointDir.removeRecursively();
}

}
}
