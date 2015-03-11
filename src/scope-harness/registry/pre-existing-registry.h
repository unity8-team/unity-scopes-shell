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

#include <scope-harness/registry/registry.h>

#include <string>

namespace unity
{
namespace scopeharness
{
namespace registry
{

class Q_DECL_EXPORT PreExistingRegistry final: public Registry
{
public:
    PreExistingRegistry(const std::string &runtimeConfig);

    ~PreExistingRegistry();

    PreExistingRegistry(const PreExistingRegistry& other) = delete;

    PreExistingRegistry(PreExistingRegistry&& other) = delete;

    PreExistingRegistry& operator=(const PreExistingRegistry& other) = delete;

    PreExistingRegistry& operator=(PreExistingRegistry&& other) = delete;

    void start() override;

protected:
    struct _Priv;

    std::shared_ptr<_Priv> p;
};

}
}
}
