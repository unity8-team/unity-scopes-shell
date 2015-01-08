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
 * Authors:
 *  Pete Woods <pete.woods@canonical.com>
 *  Michal Hruby <michal.hruby@canonical.com>
 */

#include <scope-harness/registry/custom-registry.h>
#include <scope-harness/test-utils.h>

#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QStringList>
#include <QTemporaryDir>

using namespace std;

namespace unity
{
namespace scopeharness
{
namespace registry
{
namespace
{

const static QString RUNTIME_CONFIG = R"(
[Runtime]
Registry.Identity = Registry
Registry.ConfigFile = %1
Default.Middleware = Zmq
Zmq.ConfigFile = %2
Smartscopes.Registry.Identity = %3
)";

const static QString REGISTRY_CONFIG = R"(
[Registry]
Middleware = Zmq
Zmq.ConfigFile = %1
Scoperunner.Path = %2
Scope.InstallDir = %3
Click.InstallDir = %4
OEM.InstallDir = %5
)";

const static QString MW_CONFIG = R"(
[Zmq]
EndpointDir = %1
)";

}

struct CustomRegistry::Parameters::Priv
{
    vector<string> m_scopes;

    bool m_includeSystemScopes = false;

    bool m_includeClickScopes = false;

    bool m_includeOemScopes = false;

    bool m_includeRemoteScopes = false;
};

CustomRegistry::Parameters::Parameters(vector<string> const& scopes) :
        p(new Priv)
{
    p->m_scopes = scopes;
}

CustomRegistry::Parameters::Parameters(const Parameters& other) :
        p(new Priv)
{
    *this = other;
}

CustomRegistry::Parameters::Parameters(Parameters&& other)
{
    p = move(other.p);
}

CustomRegistry::Parameters& CustomRegistry::Parameters::operator=(const Parameters& other)
{
    p->m_scopes = other.p->m_scopes;
    p->m_includeSystemScopes = other.p->m_includeSystemScopes;
    p->m_includeClickScopes = other.p->m_includeClickScopes;
    p->m_includeOemScopes = other.p->m_includeOemScopes;
    p->m_includeRemoteScopes = other.p->m_includeRemoteScopes;
    return *this;
}

CustomRegistry::Parameters& CustomRegistry::Parameters::operator=(Parameters&& other)
{
    p = move(other.p);
    return *this;
}

CustomRegistry::Parameters& CustomRegistry::Parameters::includeSystemScopes()
{
    p->m_includeSystemScopes = true;
    return *this;
}

CustomRegistry::Parameters& CustomRegistry::Parameters::includeClickScopes()
{
    p->m_includeClickScopes = true;
    return *this;
}

CustomRegistry::Parameters& CustomRegistry::Parameters::includeOemScopes()
{
    p->m_includeOemScopes = true;
    return *this;
}

CustomRegistry::Parameters& CustomRegistry::Parameters::includeRemoteScopes()
{
    p->m_includeRemoteScopes = true;
    return *this;
}

struct CustomRegistry::Priv
{
    Priv(const Parameters& parameters) :
        m_parameters(parameters)
    {
    }

    Parameters m_parameters;

    QProcess m_registryProcess;

    QTemporaryDir m_temp;
};

CustomRegistry::CustomRegistry(const Parameters& parameters):
    p(new Priv(parameters))
{
    p->m_parameters = parameters;
}

CustomRegistry::~CustomRegistry()
{
    if (p->m_registryProcess.state() != QProcess::NotRunning) {
        p->m_registryProcess.terminate();
        p->m_registryProcess.waitForFinished(5000);
        p->m_registryProcess.kill();
    }
}

void CustomRegistry::start()
{
    qputenv("UNITY_SCOPES_CONFIG_DIR", p->m_temp.path().toUtf8());
    qputenv("TEST_DESKTOP_FILES_DIR", "");

    QDir tmp(p->m_temp.path());

    QFile runtimeConfig(tmp.filePath("Runtime.ini"));
    QFile registryConfig(tmp.filePath("Registry.ini"));
    QFile mwConfig(tmp.filePath("Zmq.ini"));

    tmp.mkpath("endpoints");
    QDir endpointsDir(tmp.filePath("endpoints"));

    throwIf(!runtimeConfig.open(QIODevice::WriteOnly)
            || !registryConfig.open(QIODevice::WriteOnly)
            || !mwConfig.open(QIODevice::WriteOnly)
            || !endpointsDir.exists(),
            "Unable to open temporary files");

    QFile scopeRegistryBin(SCOPESLIB_SCOPEREGISTRY_BIN);
    QFile scopeRunnerBin(SCOPESLIB_SCOPERUNNER_BIN);

    QDir scopeInstallDir(SCOPESLIB_SCOPESDIR);
    if (!p->m_parameters.p->m_includeSystemScopes)
    {
        scopeInstallDir = tmp.filePath("scopes");
        throwIfNot(tmp.mkpath("scopes"), string("Unable to create directory: ") + scopeInstallDir.path().toStdString());
    }

    QDir clickInstallDir(QDir::home().filePath(".local/share/unity-scopes"));
    if (!p->m_parameters.p->m_includeClickScopes)
    {
        clickInstallDir = tmp.filePath("click");
        throwIfNot(tmp.mkpath("click"), string("Unable to create directory: ") + clickInstallDir.path().toStdString());
    }

    QDir oemInstallDir("/custom/share/unity-scopes");
    if (!p->m_parameters.p->m_includeOemScopes)
    {
        oemInstallDir = tmp.filePath("oem");
        throwIfNot(tmp.mkpath("oem"), string("Unable to create directory: ") + oemInstallDir.path().toStdString());
    }

    // FIXME: keep in sync with the SSRegistry config
    QString runtimeIni = RUNTIME_CONFIG
            .arg(registryConfig.fileName())
            .arg(mwConfig.fileName())
            .arg(p->m_parameters.p->m_includeRemoteScopes ? "SSRegistry" : "");

    QString registryIni = REGISTRY_CONFIG
            .arg(mwConfig.fileName())
            .arg(scopeRunnerBin.fileName())
            .arg(scopeInstallDir.path())
            .arg(clickInstallDir.path())
            .arg(oemInstallDir.path());

    QString mwIni = MW_CONFIG
            .arg(endpointsDir.path());

    runtimeConfig.write(runtimeIni.toUtf8());
    registryConfig.write(registryIni.toUtf8());
    mwConfig.write(mwIni.toUtf8());

    runtimeConfig.close();
    registryConfig.close();
    mwConfig.close();

    qputenv("UNITY_SCOPES_RUNTIME_PATH", runtimeConfig.fileName().toLocal8Bit());

    QStringList arguments;
    arguments << runtimeConfig.fileName();
    for (const auto& scope : p->m_parameters.p->m_scopes)
    {
        arguments << QString::fromStdString(scope);
    }

    p->m_registryProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    p->m_registryProcess.start(scopeRegistryBin.fileName(), arguments);
    throwIfNot(p->m_registryProcess.waitForStarted(), "Scope registry failed to start");

    // FIXME hard-coded path
    QProcess::startDetached(
            "/usr/lib/" DEB_HOST_MULTIARCH "/libqtdbustest/watchdog",
            QStringList() << QString::number(QCoreApplication::applicationPid())
                    << QString::number(p->m_registryProcess.pid()));

    qputenv("UNITY_SCOPES_TYPING_TIMEOUT_OVERRIDE", "0");
    qputenv("UNITY_SCOPES_LIST_DELAY", "5");
    qputenv("UNITY_SCOPES_RESULTS_TTL_OVERRIDE", "250");
    qputenv("UNITY_SCOPES_NO_LOCATION", "1");
}

}
}
}
