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

#ifndef LOGIN_TO_ACCOUNT_H
#define LOGIN_TO_ACCOUNT_H

#include <QObject>
#include <online-accounts-client/Setup>

class Q_DECL_EXPORT LoginToAccount: public QObject
{
    Q_OBJECT

public:
    LoginToAccount(QString const& scope_id, QString const& service_name, QString const& service_type, QString const& provider_name, int login_passed_action, int
            login_failed_action, QObject *parent);
    void loginToAccount();

Q_SIGNALS:
    void finished(bool, int);
    void searchInProgress(bool);

private Q_SLOTS:
    void onSetupFinished();

private:
    OnlineAccountsClient::Setup *m_setup;
    QString m_scope_id;
    QString m_service_name;
    QString m_service_type;
    QString m_provider_name;
    int m_login_passed_action;
    int m_login_failed_action;
};

#endif
