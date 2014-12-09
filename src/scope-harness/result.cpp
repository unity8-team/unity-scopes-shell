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

#include <scope-harness/internal/result-arguments.h>
#include <scope-harness/preview-view.h>
#include <scope-harness/result.h>
#include <scope-harness/test-utils.h>

#include <Unity/resultsmodel.h>
#include <Unity/scope.h>
#include <Unity/utils.h>

#include <QSignalSpy>

using namespace std;

namespace ng = scopes_ng;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{

struct Result::Priv
{
    ss::ResultsModelInterface* m_resultsModel;

    ss::ScopeInterface* m_scope;

    QModelIndex m_index;

    weak_ptr<PreviewView> m_previewView;

    sc::Variant m_null;
};

Result::Result(const internal::ResultArguments& arguments) :
        p(new Priv)
{
    p->m_resultsModel = arguments.resultsModel;
    p->m_scope = arguments.scope;
    p->m_index = arguments.index;
    p->m_previewView = arguments.previewView;
}

Result::Result(const Result& other) :
        p(new Priv)
{
    *this = other;
}

Result& Result::operator=(const Result& other)
{
    p->m_resultsModel = other.p->m_resultsModel;
    p->m_index = other.p->m_index;
    return *this;
}

Result& Result::operator=(Result&& other)
{
    p = move(other.p);
    return *this;
}

string Result::uri() const noexcept
{
    return p->m_resultsModel->data(p->m_index,
                                   ss::ResultsModelInterface::Roles::RoleUri).toString().toStdString();
}

string Result::title() const noexcept
{
    return p->m_resultsModel->data(p->m_index,
                                   ss::ResultsModelInterface::Roles::RoleTitle).toString().toStdString();
}

string Result::art() const noexcept
{
    return p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleArt).toString().toStdString();
}

string Result::dnd_uri() const noexcept
{
    return p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleDndUri).toString().toStdString();
}

string Result::subtitle() const noexcept
{
    return p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleSubtitle).toString().toStdString();
}

string Result::emblem() const noexcept
{
    return p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleEmblem).toString().toStdString();
}

string Result::mascot() const noexcept
{
    return p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleMascot).toString().toStdString();
}

sc::Variant Result::attributes() const noexcept
{
    return ng::qVariantToScopeVariant(p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleAttributes));
}

sc::Variant Result::summary() const noexcept
{
    return ng::qVariantToScopeVariant(p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleSummary));
}

sc::Variant Result::background() const noexcept
{
    return ng::qVariantToScopeVariant(p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleBackground));
}

sc::Variant const& Result::operator[](string const& key) const
{
    return value(key);
}

sc::Variant const& Result::value(string const& key) const
{
    auto result = p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleResult).value<sc::Result::SPtr>();
    if (!result)
    {
        return p->m_null;
    }

    return result->value(key);
}

AbstractView::SPtr Result::activate() const
{
    auto result = p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleResult).value<sc::Result::SPtr>();
    throwIfNot(bool(result), "Couldn't get result");

    QSignalSpy spy(p->m_scope, SIGNAL(hideDash()));
    p->m_scope->activate(QVariant::fromValue(result));
    throwIfNot(spy.wait(), "Hide dash signal failed emit");

    return AbstractView::SPtr(p->m_previewView);
}

}
}

/*
    void testActivation()
    {
        QSignalSpy spy(resultsView->activeScope(), SIGNAL(hideDash()));
        resultsView->activeScope()->activate(QVariant::fromValue(result));
        QVERIFY(spy.wait());
    }

    void testScopeActivationWithQuery()
    {
        QSignalSpy spy(resultsView->activeScope(), SIGNAL(gotoScope(QString)));
        resultsView->activeScope()->activate(QVariant::fromValue(result));
        QVERIFY(spy.wait());
    }

    void testScopeActivationWithQuery2()
    {
        QSignalSpy spy(resultsView->activeScope(), SIGNAL(metadataRefreshed()));
        QSignalSpy spy2(resultsView->activeScope(), SIGNAL(gotoScope(QString)));
        QSignalSpy spy3(resultsView->activeScope(), SIGNAL(openScope(unity::shell::scopes::ScopeInterface*)));
        // this tries to activate non-existing scope
        resultsView->activeScope()->activate(QVariant::fromValue(result));
        QVERIFY(spy.wait());
        QCOMPARE(spy2.count(), 0);
        QCOMPARE(spy3.count(), 0);
    }

    void testScopeResultWithScopeUri()
    {
        QSignalSpy spy(resultsView->activeScope(), SIGNAL(searchQueryChanged()));
        resultsView->activeScope()->activate(QVariant::fromValue(result));
        // this is likely to be invoked synchronously
        if (spy.count() == 0) {
            QVERIFY(spy.wait());
        }
        QVERIFY(spy.count() > 0);
        QCOMPARE(resultsView->activeScope()->searchQuery(), QString("next-scope-query"));
    }

 */
