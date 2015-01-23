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

#pragma once

#include <scope-harness/registry/registry.h>

#include <vector>
#include <string>

namespace unity
{
namespace scopeharness
{
namespace registry
{

class Q_DECL_EXPORT CustomRegistry final: public Registry
{
public:
    UNITY_DEFINES_PTRS(CustomRegistry);

    class Q_DECL_EXPORT Parameters
    {
    public:
        Parameters(std::vector<std::string> const& scopes);

        Parameters(const Parameters& other);

        Parameters(Parameters&& other);

        Parameters& operator=(const Parameters& other);

        Parameters& operator=(Parameters&& other);

        ~Parameters() = default;

        Parameters& includeSystemScopes();

        Parameters& includeClickScopes();

        Parameters& includeOemScopes();

        Parameters& includeRemoteScopes();

    protected:
        struct Priv;

        std::shared_ptr<Priv> p;

        friend CustomRegistry;
    };

    CustomRegistry(const Parameters& parameters);

    ~CustomRegistry();

    CustomRegistry(const CustomRegistry& other) = delete;

    CustomRegistry(CustomRegistry&& other) = delete;

    CustomRegistry& operator=(const CustomRegistry& other) = delete;

    CustomRegistry& operator=(CustomRegistry&& other) = delete;

    void start() override;

protected:
    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
