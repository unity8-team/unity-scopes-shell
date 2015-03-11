/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
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

#include "optionselectoroptions.h"
#include "optionselectorfilter.h"
#include <QQmlEngine>
#include <QDebug>

namespace scopes_ng
{

OptionSelectorOption::OptionSelectorOption(const QString& id, const QString &label)
    : m_id(id),
      m_label(label)
{
}

QString OptionSelectorOption::id() const
{
    return m_id;
}

QString OptionSelectorOption::label() const
{
    return m_label;
}

bool OptionSelectorOption::checked() const
{
    return m_checked;
}

void OptionSelectorOption::setChecked(bool checked)
{
    if (checked != m_checked)
    {
        m_checked = checked;
        Q_EMIT checkedChanged(m_checked);
    }
}

OptionSelectorOptions::OptionSelectorOptions(OptionSelectorFilter *parent)
    : ModelUpdate(parent)
{
}

void OptionSelectorOptions::update(const std::list<unity::scopes::FilterOption::SCPtr>& options, const std::set<unity::scopes::FilterOption::SCPtr>& activeOptions)
{
    QSet<QString> actOpts;
    for (auto const& opt: activeOptions)
    {
        actOpts.insert(QString::fromStdString(opt->id()));
    }

    syncModel(options, m_options,
            // key function for scopes api filter option
            [](const unity::scopes::FilterOption::SCPtr& opt) -> QString { return QString::fromStdString(opt->id()); },
            // key function for shell api filter option
            [](const QSharedPointer<OptionSelectorOption>& opt) -> QString { return opt->id(); },
            // factory function for creating shell filter option from scopes api filter option
            [this](const unity::scopes::FilterOption::SCPtr& opt) -> QSharedPointer<OptionSelectorOption> {
                auto optObj = QSharedPointer<OptionSelectorOption>(
                    new OptionSelectorOption(QString::fromStdString(opt->id()), QString::fromStdString(opt->label())));
                QQmlEngine::setObjectOwnership(optObj.data(), QQmlEngine::CppOwnership);
                connect(optObj.data(), SIGNAL(checkedChanged(bool)), this, SLOT(onOptionChecked(bool)));
                return optObj;
            },
            // filter option update function
            [&actOpts](const unity::scopes::FilterOption::SCPtr& op1, const QSharedPointer<OptionSelectorOption>& op2) -> bool {
                if (op2->id() != QString::fromStdString(op1->id())) {
                    return false;
                }
                bool backendState = actOpts.contains(op2->id());
                if (backendState != op2->checked())
                {
                    op2->setChecked(backendState);
                }
                return true;
            });
}

int OptionSelectorOptions::rowCount(const QModelIndex& parent) const
{
    return m_options.count();
}

void OptionSelectorOptions::onOptionChecked(bool checked)
{
    OptionSelectorOption* opt = qobject_cast<scopes_ng::OptionSelectorOption*>(sender());
    if (opt)
    {
        Q_EMIT optionChecked(opt->id(), checked);
    }
}

QVariant OptionSelectorOptions::data(const QModelIndex& index, int role) const
{
    if (index.row() >= m_options.count())
    {
        return QVariant();
    }
    switch (role)
    {
        case Qt::DisplayRole:
        case RoleOption:
            return QVariant::fromValue(m_options.at(index.row()).data());
        default:
            return QVariant();
    }
}

}
