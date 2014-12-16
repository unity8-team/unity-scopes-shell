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

#pragma once

#include <QObject>
#include <QSocketNotifier>

namespace unity
{
namespace scopeharness
{
namespace internal
{

class Q_DECL_EXPORT SignalHandler: public QObject
{
Q_OBJECT

public:
	SignalHandler(QObject *parent = 0);

	~SignalHandler() = default;

	static int setupUnixSignalHandlers();

protected Q_SLOTS:
	void handleSigInt();

	void handleSigTerm();

protected:
	static void intSignalHandler(int unused);

	static void termSignalHandler(int unused);

	static int sigintFd[2];

	static int sigtermFd[2];

	QSocketNotifier *m_socketNotifierInt;

	QSocketNotifier *m_socketNotifierTerm;
};

}
}
}
