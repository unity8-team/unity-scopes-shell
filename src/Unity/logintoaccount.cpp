/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
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

#include "logintoaccount.h"
#include <unity/scopes/OnlineAccountClient.h>
#include <QtConcurrent>

using namespace unity;

LoginToAccount::LoginToAccount(QString const& scope_id, QString const& service_name, QString const& service_type, QString const& provider_name,
        int login_passed_action, int login_failed_action, QObject *parent)
    : QObject(parent),
      m_scope_id(scope_id),
      m_service_name(service_name),
      m_service_type(service_type),
      m_provider_name(provider_name),
      m_login_passed_action(login_passed_action),
      m_login_failed_action(login_failed_action)
{
}

void LoginToAccount::onSetupFinished()
{
    // Check again whether the service was successfully enabled
    scopes::OnlineAccountClient oa_client(m_service_name.toStdString(), m_service_type.toStdString(), m_provider_name.toStdString());
    auto service_statuses = oa_client.get_service_statuses();
    for (auto const& status : service_statuses)
    {
        if (status.service_enabled)
        {
            Q_EMIT finished(true, m_login_passed_action);
            return;
        }
    }
    Q_EMIT finished(false, m_login_failed_action);
}

void LoginToAccount::loginToAccount()
{
    // Set the UNITY_SCOPES_OA_UI_POLICY environment variable here so that OnlineAccountClient knows we're
    // calling it from the shell (hence it will use the default UI policy when talking to libsignon).
    setenv("UNITY_SCOPES_OA_UI_POLICY", "1", 0);

    QFuture<bool> service_enabled_future = QtConcurrent::run([&]
    {
        // Check if at least one account has the specified service enabled
        scopes::OnlineAccountClient oa_client(m_service_name.toStdString(), m_service_type.toStdString(), m_provider_name.toStdString());
        auto service_statuses = oa_client.get_service_statuses();
        for (auto const& status : service_statuses)
        {
            if (status.service_enabled)
            {
                return true;
            }
        }
        return false;
    });
    QFutureWatcher<bool> future_watcher;
    future_watcher.setFuture(service_enabled_future);

    // Set SearchInProgress so that the loading bar animates while we waiting for the token to be issued.
    //setSearchInProgress(true);

    QEventLoop loop;
    connect(&future_watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);

    // Unset SearchInProgress to stop the loading bar animation.
    //setSearchInProgress(false);

    bool service_enabled = service_enabled_future.result();

    // Start the signon UI if no enabled services were found
    if (!service_enabled)
    {
        m_setup = new OnlineAccountsClient::Setup(this);
        connect(m_setup, &OnlineAccountsClient::Setup::finished, this, &LoginToAccount::onSetupFinished);
        m_setup->setApplicationId(m_scope_id);
        m_setup->setServiceTypeId(m_service_type);
        m_setup->setProviderId(m_provider_name);
        m_setup->exec();
        return;
    }
    else
    {
        Q_EMIT finished(true, m_login_passed_action);
    }
}

