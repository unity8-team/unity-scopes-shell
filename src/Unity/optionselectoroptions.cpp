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
#include <QSet>
#include <QDebug>

namespace scopes_ng
{

struct OptionSelectorOption
{
    QString id;
    QString label;
    bool checked;
    bool default_checked;

    OptionSelectorOption(const QString& id, const QString &label, bool default_checked) :
        id(id),
        label(label),
        checked(false),
        default_checked(default_checked)
    {}
};

OptionSelectorOptions::OptionSelectorOptions(OptionSelectorFilter *parent, std::list<unity::scopes::FilterOption::SCPtr> const& options,
        std::set<unity::scopes::FilterOption::SCPtr> const& activeOptions)
    : ModelUpdate(parent)
{
    bool const use_defaults = (activeOptions.size() == 0);

    for (auto const& opt: options) {
        auto shellOpt = QSharedPointer<OptionSelectorOption>(new OptionSelectorOption(QString::fromStdString(opt->id()), QString::fromStdString(opt->label()),
                        opt->default_value()));
        m_options.append(shellOpt);
        if (use_defaults) {
            shellOpt->checked = shellOpt->default_checked;
        } else {
            if (activeOptions.find(opt) != activeOptions.end()) {
                shellOpt->checked = true;
            }
        }
    }
}

void OptionSelectorOptions::update(const std::set<unity::scopes::FilterOption::SCPtr>& activeOptions, bool allow_defaults)
{
    bool const use_defaults = (allow_defaults && activeOptions.size() == 0);
    if (use_defaults) {
        for (int row = 0; row<m_options.size(); row++) {
            auto &opt = m_options[row];
            if (opt->checked != opt->default_checked) {
                opt->checked = opt->default_checked;
                Q_EMIT dataChanged(index(row, 0), index(row, 0), { unity::shell::scopes::OptionSelectorOptionsInterface::Roles::RoleOptionChecked });
            }
        }
        return;
    }

    QSet<QString> actOpts;
    for (auto const& opt: activeOptions) {
        actOpts.insert(QString::fromStdString(opt->id()));
    }

    for (int row = 0; row<m_options.size(); row++) {
        auto &opt = m_options[row];
        if (actOpts.find(opt->id) != actOpts.end()) {
            if (!opt->checked) {
                opt->checked = true;
                Q_EMIT dataChanged(index(row, 0), index(row, 0), { unity::shell::scopes::OptionSelectorOptionsInterface::Roles::RoleOptionChecked });
            }
        } else {
            if (opt->checked) {
                opt->checked = false;
                Q_EMIT dataChanged(index(row, 0), index(row, 0), { unity::shell::scopes::OptionSelectorOptionsInterface::Roles::RoleOptionChecked });
            }
        }
    }
}

void OptionSelectorOptions::update(const std::list<unity::scopes::FilterOption::SCPtr>& options)
{
    QVector<int> roles;
    roles.append(unity::shell::scopes::OptionSelectorOptionsInterface::Roles::RoleOptionLabel);

    syncModel(options, m_options,
            // key function for scopes api filter option
            [](const unity::scopes::FilterOption::SCPtr& opt) -> QString { return QString::fromStdString(opt->id()); },
            // key function for shell api filter option
            [](const QSharedPointer<OptionSelectorOption>& opt) -> QString { return opt->id; },
            // factory function for creating shell filter option from scopes api filter option
            [this](const unity::scopes::FilterOption::SCPtr& opt) -> QSharedPointer<OptionSelectorOption> {
                auto optObj = QSharedPointer<OptionSelectorOption>(
                    new OptionSelectorOption(QString::fromStdString(opt->id()), QString::fromStdString(opt->label()), opt->default_value()));
                return optObj;
            },
            // filter option update function
            [&roles, this](int row, const unity::scopes::FilterOption::SCPtr& op1, const QSharedPointer<OptionSelectorOption>& op2) -> bool {
                if (op2->id != QString::fromStdString(op1->id())) {
                    return false;
                }
                if (op2->label != QString::fromStdString(op1->label())) {
                    op2->label = QString::fromStdString(op1->label());
                    Q_EMIT dataChanged(index(row, 0), index(row, 0), roles);
                }
                return true;
            });
}

int OptionSelectorOptions::rowCount(const QModelIndex& /* parent */) const
{
    return m_options.count();
}

void OptionSelectorOptions::setChecked(int row, bool checked)
{
    if (row >= 0 && row < m_options.count())
    {
        auto opt = m_options.at(row);
        if (checked != opt->checked)
        {
            Q_EMIT optionChecked(opt->id, checked);
        }
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
        case RoleOptionId:
            return QVariant(m_options.at(index.row())->id);
        case RoleOptionLabel:
            return QVariant(m_options.at(index.row())->label);
        case RoleOptionChecked:
            return QVariant(m_options.at(index.row())->checked);
        default:
            return QVariant();
    }
}

}
