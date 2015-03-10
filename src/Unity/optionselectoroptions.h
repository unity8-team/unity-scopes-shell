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

#ifndef NG_OPTIONSELECTOROPTIONS_H
#define NG_OPTIONSELECTOROPTIONS_H

#include "OptionSelectorOptionsInterface.h"
#include "modelupdate.h"
#include <QSharedPointer>
#include <QList>
#include <list>
#include <functional>

namespace scopes_ng
{

class OptionSelectorFilter;

class Q_DECL_EXPORT OptionSelectorOption : public unity::shell::scopes::OptionSelectorOptionInterface
{
    Q_OBJECT

public:
    OptionSelectorOption(const QString& id, const QString &label);

    QString id() const override;
    QString label() const override;
    bool checked() const override;
    void setChecked(bool checked) override;

private:
    QString m_id;
    QString m_label;
    bool m_checked;
};

class Q_DECL_EXPORT OptionSelectorOptions :
    public ModelUpdate<unity::shell::scopes::OptionSelectorOptionsInterface,
                       const std::list<unity::scopes::FilterOption::SCPtr>&,
                       QList<QSharedPointer<OptionSelectorOption>>,
                       std::function<QString(unity::scopes::FilterOption::SCPtr const&)>,
                       std::function<QString(QSharedPointer<OptionSelectorOption> const&)>,
                       std::function<QSharedPointer<OptionSelectorOption>(unity::scopes::FilterOption::SCPtr const&)>>
{
    Q_OBJECT

public:
    explicit OptionSelectorOptions(OptionSelectorFilter *parent = nullptr);
    void update(const std::list<unity::scopes::FilterOption::SCPtr>& options, unity::scopes::FilterState::SPtr const& filterState) override;
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void optionChecked(const QString&, bool);

protected Q_SLOTS:
    void onOptionChecked(bool checked);

private:
    QList<QSharedPointer<OptionSelectorOption>> m_options;
};

} // namespace scopes_ng

#endif
