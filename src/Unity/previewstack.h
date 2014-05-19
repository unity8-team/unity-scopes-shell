/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Michał Sawicz <michal.sawicz@canonical.com>
 *  Michal Hruby <michal.hruby@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef NG_PREVIEW_STACK_H
#define NG_PREVIEW_STACK_H

#include <unity/shell/scopes/PreviewStackInterface.h>

#include <QSet>
#include <QSharedPointer>
#include <QMultiMap>
#include <QPointer>

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Result.h>

#include "collectors.h"

namespace scopes_ng
{

class PreviewModel;
class Scope;

class Q_DECL_EXPORT PreviewStack : public unity::shell::scopes::PreviewStackInterface
{
    Q_OBJECT

public:
    explicit PreviewStack(QObject* parent = 0);
    virtual ~PreviewStack();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    virtual bool event(QEvent* ev) override;

    Q_INVOKABLE unity::shell::scopes::PreviewModelInterface* getPreviewModel(int index) const override;

    void loadForResult(unity::scopes::Result::SPtr const&);

    void setWidgetColumnCount(int columnCount) override;
    int widgetColumnCount() const override;
    void setAssociatedScope(scopes_ng::Scope*);

private Q_SLOTS:
    void widgetTriggered(QString const&, QString const&, QVariantMap const&);

private:
    void processActionResponse(PushEvent* pushEvent);

    void dispatchPreview(unity::scopes::Variant const& extra_data = unity::scopes::Variant());

    int m_widgetColumnCount;
    QList<PreviewModel*> m_previews;
    PreviewModel* m_activePreview;
    QPointer<scopes_ng::Scope> m_associatedScope;

    unity::scopes::QueryCtrlProxy m_lastPreviewQuery;
    QMap<PreviewModel*, std::weak_ptr<ScopeDataReceiverBase>> m_listeners;
    std::shared_ptr<ScopeDataReceiverBase> m_lastActivation;

    unity::scopes::Result::SPtr m_previewedResult;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::PreviewStack*)

#endif // NG_PREVIEW_STACK_H
