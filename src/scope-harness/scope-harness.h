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

#include <string>
#include <memory>

#include <QtGlobal>

#include <unity/util/DefinesPtrs.h>

#include <scope-harness/registry/custom-registry.h>
#include <scope-harness/registry/registry.h>
#include <scope-harness/view/results-view.h>

#define QVERIFY_MATCHRESULT(statement) \
do {\
    auto result = (statement);\
    QVERIFY2(result.success(), result.concat_failures().c_str());\
} while (0)

namespace unity
{
namespace scopeharness
{

class Q_DECL_EXPORT ScopeHarness final
{
public:
    UNITY_DEFINES_PTRS(ScopeHarness);

    Q_DECL_EXPORT
    static ScopeHarness::UPtr newFromPreExistingConfig(const std::string& directory);

    Q_DECL_EXPORT
    static ScopeHarness::UPtr newFromScopeList(const registry::CustomRegistry::Parameters& parameters);

    Q_DECL_EXPORT
    static ScopeHarness::UPtr newFromSystem();

    ~ScopeHarness() = default;

    view::ResultsView::SPtr resultsView();

protected:
    ScopeHarness(registry::Registry::SPtr registry);

    struct Priv;
    std::shared_ptr<Priv> p;
};

}
}
