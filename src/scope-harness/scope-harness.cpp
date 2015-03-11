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

#include <Unity/scopes.h>

#include <scope-harness/internal/results-view-arguments.h>
#include <scope-harness/registry/pre-existing-registry.h>
#include <scope-harness/registry/system-registry.h>
#include <scope-harness/scope-harness.h>
#include <scope-harness/test-utils.h>

#include <QSignalSpy>

using namespace std;
namespace ng = scopes_ng;

namespace unity
{
namespace scopeharness
{
using namespace internal;

struct ScopeHarness::_Priv
{
    registry::Registry::SPtr m_registry;

    shared_ptr<ng::Scopes> m_scopes;

    view::ResultsView::SPtr m_resultsView;

    view::PreviewView::SPtr m_previewView;
};

ScopeHarness::UPtr ScopeHarness::newFromPreExistingConfig(const std::string& directory)
{
    registry::Registry::SPtr registry = make_shared<registry::PreExistingRegistry>(directory);
    return ScopeHarness::UPtr(new ScopeHarness(registry));
}

ScopeHarness::UPtr ScopeHarness::newFromScopeList(const registry::CustomRegistry::Parameters& parameters)
{
    registry::Registry::SPtr registry = make_shared<registry::CustomRegistry>(parameters);
    return ScopeHarness::UPtr(new ScopeHarness(registry));
}

ScopeHarness::UPtr ScopeHarness::newFromSystem()
{
    registry::Registry::SPtr registry = make_shared<registry::SystemRegistry>();
    return ScopeHarness::UPtr(new ScopeHarness(registry));
}

ScopeHarness::ScopeHarness(registry::Registry::SPtr registry) :
        p(new _Priv)
{
    qputenv("UNITY_SCOPES_NO_FAVORITES", "1");
    qputenv("UNITY_SCOPES_NO_OPEN_URL", "1");

    p->m_registry = registry;
    p->m_registry->start();

    p->m_scopes = make_shared<ng::Scopes>();

    p->m_previewView = make_shared<view::PreviewView>();
    p->m_resultsView = make_shared<view::ResultsView>(internal::ResultsViewArguments{p->m_scopes});

    p->m_resultsView->setPreviewView(p->m_previewView);
    p->m_previewView->setResultsView(p->m_resultsView);

    // no scopes on startup
    TestUtils::throwIf(p->m_scopes->rowCount() != 0 || p->m_scopes->loaded(),
          "Scopes object was pre-populated");

    // wait till the registry spawns
    QSignalSpy spy(p->m_scopes.get(), SIGNAL(loadedChanged()));
    TestUtils::throwIfNot(spy.wait(), "Scopes failed to initalize");

    TestUtils::throwIf(p->m_scopes->rowCount() == 0 || !p->m_scopes->loaded(), "No scopes loaded");

    for (int i = 0; i < p->m_scopes->rowCount(); ++i)
    {
        // get scope proxy
        ng::Scope::Ptr scope = p->m_scopes->getScopeByRow(i);
        QSignalSpy spy(scope.data(), SIGNAL(searchInProgressChanged()));
        if (scope->searchInProgress())
        {
            TestUtils::throwIfNot(spy.wait(), "Search progress didn't change");
        }
    }
}

view::ResultsView::SPtr ScopeHarness::resultsView()
{
    return p->m_resultsView;
}

}
}
