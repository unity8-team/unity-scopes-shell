/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */

#ifndef SCOPE_TEST_INTERFACE_H_
#define SCOPE_TEST_INTERFACE_H_

#include <QObject>

class TestAdaptor;

class ScopeTestInterface: public QObject
{
Q_OBJECT
public:
    ScopeTestInterface();

    virtual ~ScopeTestInterface();

Q_SIGNALS:
    void Preview();
    void Search(const QString &query);
    void Start();
    void Stop();

protected:
    QScopedPointer<TestAdaptor> m_adaptor;
};

#endif /* SCOPE_TEST_INTERFACE_H_ */
