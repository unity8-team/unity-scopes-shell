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

#include <scope-harness/internal/signal-handler.h>

#include <QCoreApplication>
#include <QDebug>

#include <csignal>
#include <sys/socket.h>
#include <unistd.h>

namespace unity
{
namespace scopeharness
{
namespace internal
{
int SignalHandler::sigintFd[2];

int SignalHandler::sigtermFd[2];

SignalHandler::SignalHandler(QObject *parent) :
		QObject(parent) {

	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigintFd)) {
		qFatal("Couldn't create INT socketpair");
	}
	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd)) {
		qFatal("Couldn't create TERM socketpair");
	}

	m_socketNotifierInt = new QSocketNotifier(sigintFd[1], QSocketNotifier::Read, this);
	connect(m_socketNotifierInt, &QSocketNotifier::activated, this, &SignalHandler::handleSigInt);
	m_socketNotifierTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
	connect(m_socketNotifierTerm, &QSocketNotifier::activated, this, &SignalHandler::handleSigTerm);
}

void SignalHandler::intSignalHandler(int) {
	char a = 1;
	::write(sigintFd[0], &a, sizeof(a));
}

void SignalHandler::termSignalHandler(int) {
	char a = 1;
	::write(sigtermFd[0], &a, sizeof(a));
}

int SignalHandler::setupUnixSignalHandlers() {
	struct sigaction sigint, sigterm;

	sigint.sa_handler = SignalHandler::intSignalHandler;
	sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = 0;
	sigint.sa_flags |= SA_RESTART;

	if (sigaction(SIGINT, &sigint, 0) > 0)
		return 1;

	sigterm.sa_handler = SignalHandler::termSignalHandler;
	sigemptyset(&sigterm.sa_mask);
	sigterm.sa_flags |= SA_RESTART;

	if (sigaction(SIGTERM, &sigterm, 0) > 0)
		return 2;

	return 0;
}

void SignalHandler::handleSigTerm() {
	m_socketNotifierTerm->setEnabled(false);
	char tmp;
	::read(sigtermFd[1], &tmp, sizeof(tmp));

	QCoreApplication::exit(0);

	m_socketNotifierTerm->setEnabled(true);
}

void SignalHandler::handleSigInt() {
	m_socketNotifierInt->setEnabled(false);
	char tmp;
	::read(sigintFd[1], &tmp, sizeof(tmp));

	QCoreApplication::exit(0);

	m_socketNotifierInt->setEnabled(true);
}

}
}
}
