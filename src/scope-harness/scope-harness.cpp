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

#include <scope-harness/registry/pre-existing-registry.h>
#include <scope-harness/scope-harness.h>
#include <scope-harness/test-utils.h>

#include <QSignalSpy>

using namespace std;
namespace ng = scopes_ng;

namespace unity
{
namespace scopeharness
{

struct ScopeHarness::Priv
{
    registry::Registry::SPtr m_registry;

    shared_ptr<ng::Scopes> m_scopes;

    view::ResultsView::SPtr m_resultsView;
};

ScopeHarness::UPtr ScopeHarness::newFromPreExistingConfig(const std::string& directory)
{
    registry::Registry::SPtr registry = make_shared<registry::PreExistingRegistry>(directory);
    return ScopeHarness::UPtr(new ScopeHarness(registry));
}

ScopeHarness::UPtr ScopeHarness::newFromSystem()
{
    throw domain_error("Not implemented");
}

ScopeHarness::ScopeHarness(registry::Registry::SPtr registry) :
        p(new Priv)
{
    qputenv("UNITY_SCOPES_NO_FAVORITES", "1");

    p->m_registry = registry;
    p->m_registry->start();

//    const QStringList favs {"scope://mock-scope", "scope://mock-scope-ttl", "scope://mock-scope-info"};
//    setFavouriteScopes(favs);

    p->m_scopes = make_shared<ng::Scopes>();
    auto previewView = make_shared<view::PreviewView>();
    p->m_resultsView = make_shared<view::ResultsView>(p->m_scopes, previewView);

    // no scopes on startup
    throwIf(p->m_scopes->rowCount() != 0 || p->m_scopes->loaded(),
          "Scopes object was pre-populated");

    // wait till the registry spawns
    QSignalSpy spy(p->m_scopes.get(), SIGNAL(loadedChanged()));
    throwIfNot(spy.wait(), "Scopes failed to initalize");

    throwIf(p->m_scopes->rowCount() == 0 || !p->m_scopes->loaded(), "No scopes loaded");

    for (int i = 0; i < p->m_scopes->rowCount(); ++i)
    {
        // get scope proxy
        ng::Scope::Ptr scope = p->m_scopes->getScopeByRow(i);
        QSignalSpy spy(scope.data(), SIGNAL(searchInProgressChanged()));
//        scope->setActive(true);
//        if (!scope->searchInProgress())
//        {
//            throwIfNot(spy.wait(), "Search progress didn't change 1");
//        }
        if (scope->searchInProgress())
        {
            throwIfNot(spy.wait(), "Search progress didn't change");
        }
//        scope->setActive(false);
    }
}

view::ResultsView::SPtr ScopeHarness::resultsView()
{
    return p->m_resultsView;
}

}
}
