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

#include <scope-harness/test-utils.h>
#include <scope-harness/internal/result-arguments.h>
#include <scope-harness/results/result.h>
#include <scope-harness/view/preview-view.h>
#include <scope-harness/view/results-view.h>

#include <Unity/resultsmodel.h>
#include <Unity/scope.h>
#include <Unity/utils.h>

#include <QObject>
#include <QSignalSpy>
#include <QUrl>

using namespace std;

namespace ng = scopes_ng;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{
namespace results
{

struct Result::Priv: public QObject
{
    Q_OBJECT

public:
    enum ActivationResponse
    {
        failed,
        show_dash,
        hide_dash,
        goto_uri,
        preview_requested,
        goto_scope,
        open_scope
    };

    QSharedPointer<ss::ResultsModelInterface> m_resultsModel;

    QSharedPointer<ss::ScopeInterface> m_scope;

    QModelIndex m_index;

    weak_ptr<view::ResultsView> m_resultsView;

    weak_ptr<view::PreviewView> m_previewView;

    sc::Variant m_null;

    void connectSignals()
    {
        connect(m_scope.data(), SIGNAL(activationFailed(QString const&)), SLOT(activationFailed(QString const&)));
        connect(m_scope.data(), SIGNAL(showDash()), SLOT(showDash()));
        connect(m_scope.data(), SIGNAL(hideDash()), SLOT(hideDash()));
        connect(m_scope.data(), SIGNAL(gotoUri(QString const&)), SLOT(gotoUri(QString const&)));
        connect(m_scope.data(), SIGNAL(previewRequested(QVariant const&)), SLOT(previewRequested(QVariant const&)));
        connect(m_scope.data(), SIGNAL(gotoScope(QString const&)), SLOT(gotoScope(QString const&)));
        connect(m_scope.data(), SIGNAL(openScope(unity::shell::scopes::ScopeInterface*)), SLOT(openScope(unity::shell::scopes::ScopeInterface*)));
    }

public Q_SLOTS:
    void activationFailed(QString const& scopeId)
    {
        Q_EMIT activated(ActivationResponse::failed, scopeId);
    }

    void showDash()
    {
        Q_EMIT activated(ActivationResponse::show_dash);
    }

    void hideDash()
    {
        Q_EMIT activated(ActivationResponse::hide_dash);
    }

    void gotoUri(QString const& uri)
    {
        Q_EMIT activated(ActivationResponse::goto_uri, uri);
    }

    void previewRequested(QVariant const& result)
    {
        Q_EMIT activated(ActivationResponse::preview_requested, result);
    }

    void gotoScope(QString const& scopeId)
    {
        Q_EMIT activated(ActivationResponse::goto_scope, scopeId);
    }

    void openScope(unity::shell::scopes::ScopeInterface* scope)
    {
        Q_EMIT activated(ActivationResponse::open_scope, QVariant::fromValue(scope));
    }

Q_SIGNALS:
    void activated(int response, QVariant const& parameter = QVariant());
};

Result::Result(const internal::ResultArguments& arguments) :
        p(new Priv)
{
    p->m_resultsModel = arguments.resultsModel;
    p->m_scope = arguments.scope;
    p->m_index = arguments.index;
    p->m_resultsView = arguments.resultsView;
    p->m_previewView = arguments.previewView;

    p->connectSignals();
}

Result::Result(const Result& other) :
        p(new Priv)
{
    *this = other;
}

Result& Result::operator=(const Result& other)
{
    // This will disconnect all the existing Qt signal connections
    p = make_shared<Priv>();

    p->m_resultsModel = other.p->m_resultsModel;
    p->m_scope = other.p->m_scope;
    p->m_index = other.p->m_index;
    p->m_resultsView = other.p->m_resultsView;
    p->m_previewView = other.p->m_previewView;

    p->connectSignals();

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

view::AbstractView::SPtr Result::activate() const
{
    auto result = p->m_resultsModel->data(p->m_index,
                                       ss::ResultsModelInterface::Roles::RoleResult).value<sc::Result::SPtr>();
    throwIfNot(bool(result), "Couldn't get result");

    QSignalSpy spy(p.get(), SIGNAL(activated(int, const QVariant&)));
    p->m_scope->activate(QVariant::fromValue(result));
    if (spy.empty())
    {
        throwIfNot(spy.wait(), "Scope activation signal failed to emit");
    }

    QVariantList response = spy.front();
    QVariant signal = response.at(0);
    auto activationResponse = Priv::ActivationResponse(signal.toInt());
    QVariant parameter = response.at(1);

    view::AbstractView::SPtr view;
    auto resultsView = p->m_resultsView.lock();
    throwIfNot(bool(resultsView), "ResultsView not available");
    auto previewView = p->m_previewView.lock();
    throwIfNot(bool(previewView), "PreviewView not available");

    switch (activationResponse)
    {
        case Priv::ActivationResponse::failed:
        {
            view = resultsView;
            break;
        }
        case Priv::ActivationResponse::show_dash:
        {
            qDebug() << "show_dash";
            break;
        }
        case Priv::ActivationResponse::hide_dash:
        {
            qDebug() << "hide_dash";
            // TODO set scope inactive?
            view = previewView;
            break;
        }
        case Priv::ActivationResponse::goto_uri:
        {
            qDebug() << "goto_uri" << parameter;
            QUrl url(parameter.toString());
            if (url.scheme() == QLatin1String("scope"))
            {
                waitForSearchFinish(p->m_scope);
            }
            view = resultsView;
            break;
        }
        case Priv::ActivationResponse::preview_requested:
        {
            qDebug() << "preview_requested" << parameter;
            break;
        }
        case Priv::ActivationResponse::goto_scope:
        {
            qDebug() << "goto_scope" << parameter;
            resultsView->setActiveScope(parameter.toString().toStdString());
            view = resultsView;
            break;
        }
        case Priv::ActivationResponse::open_scope:
        {
            qDebug() << "open_scope" << parameter;
            break;
        }
    }

    return view;
}

}
}
}

#include "result.moc"
