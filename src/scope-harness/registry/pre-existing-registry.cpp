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

#include <scope-harness/registry/pre-existing-registry.h>
#include <scope-harness/internal/test-utils.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>
#include <QThread>
#include <QSettings>
#include <QSignalSpy>

namespace unity
{
namespace scopeharness
{
using namespace internal;
namespace registry
{

struct PreExistingRegistry::Priv
{
    QString m_runtimeConfig;

    QDir m_endpointDir;

    QScopedPointer<QProcess> m_registryProcess;

    QTemporaryDir m_tempDir;
};

PreExistingRegistry::PreExistingRegistry(const std::string &runtimeConfig) :
        p(new Priv)
{
    p->m_runtimeConfig = QString::fromStdString(runtimeConfig);
    QSettings runtimeSettings(p->m_runtimeConfig, QSettings::IniFormat);
    runtimeSettings.setIniCodec("UTF-8");

    QString zmqConfig = runtimeSettings.value("Runtime/Zmq.ConfigFile").toString();
    QSettings zmqSettings(zmqConfig, QSettings::IniFormat);
    zmqSettings.setIniCodec("UTF-8");

    p->m_endpointDir = zmqSettings.value("Zmq/EndpointDir").toString();
}

void PreExistingRegistry::start()
{
    qputenv("UNITY_SCOPES_CONFIG_DIR", p->m_tempDir.path().toUtf8());
    qputenv("TEST_DESKTOP_FILES_DIR", "");

    p->m_endpointDir.removeRecursively();
    p->m_endpointDir.mkpath(".hidden");

    // startup our private scope registry
    QString registryBin(SCOPESLIB_SCOPEREGISTRY_BIN);

    p->m_registryProcess.reset(new QProcess());
    p->m_registryProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    p->m_registryProcess->start(registryBin, QStringList() << p->m_runtimeConfig);
    throwIfNot(p->m_registryProcess->waitForStarted(), "Scope registry failed to start");

    // FIXME hard-coded path
    QProcess::startDetached(
            "/usr/lib/" DEB_HOST_MULTIARCH "/libqtdbustest/watchdog",
            QStringList() << QString::number(QCoreApplication::applicationPid())
                    << QString::number(p->m_registryProcess->pid()));

    qputenv("UNITY_SCOPES_TYPING_TIMEOUT_OVERRIDE", "0");
    qputenv("UNITY_SCOPES_LIST_DELAY", "5");
    qputenv("UNITY_SCOPES_RESULTS_TTL_OVERRIDE", "250");
    qputenv("UNITY_SCOPES_RUNTIME_PATH", p->m_runtimeConfig.toUtf8());
    qputenv("UNITY_SCOPES_NO_LOCATION", "1");
}

PreExistingRegistry::~PreExistingRegistry()
{
    if (p->m_registryProcess)
    {
        p->m_registryProcess->terminate();
        if (!p->m_registryProcess->waitForFinished())
        {
            p->m_registryProcess->kill();
        }
    }
    p->m_endpointDir.removeRecursively();
}

}
}
}
