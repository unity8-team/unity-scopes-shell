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
#include <scope-harness/registry/custom-registry.h>

#include <QCoreApplication>

#include <iostream>

using namespace std;

namespace shr = unity::scopeharness::registry;
namespace shi = unity::scopeharness::internal;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        return 1;
    }

    vector<string> scopes;
    for (int i = 1; i < argc; ++i)
    {
        scopes.emplace_back(argv[i]);
    }

    QCoreApplication application(argc, argv);

    shi::SignalHandler handler;
    handler.setupUnixSignalHandlers();

    auto registry = make_shared<shr::CustomRegistry>(shr::CustomRegistry::Parameters(scopes));
    registry->start();

    return application.exec();
}
