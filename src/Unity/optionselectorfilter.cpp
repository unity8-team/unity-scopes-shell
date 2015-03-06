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

#include "optionselectorfilter.h"
#include <QDebug>

namespace scopes_ng
{

OptionSelectorFilter::OptionSelectorFilter(unity::scopes::OptionSelectorFilter::SCPtr const& filter, unity::shell::scopes::FiltersInterface *parent)
    : unity::shell::scopes::OptionSelectorFilterInterface(parent),
    m_id(QString::fromStdString(filter->id())),
    m_multiSelect(filter->multi_select()),
    m_label(QString::fromStdString(filter->label()))
{
}

QString OptionSelectorFilter::id() const
{
    return m_id;
}

QString OptionSelectorFilter::filterType() const
{
    return "option_selector";
}

QString OptionSelectorFilter::label() const
{
    return m_label;
}

bool OptionSelectorFilter::multiSelect() const
{
    return m_multiSelect;
}

int OptionSelectorFilter::count() const
{
    return m_options->count();
}

unity::shell::scopes::OptionSelectorOptionsInterface* OptionSelectorFilter::options() const
{
    return m_options.data();
}

void OptionSelectorFilter::update(unity::scopes::FilterBase::SCPtr const& filter, unity::scopes::FilterState const& filterState)
{
    unity::scopes::OptionSelectorFilter::SCPtr optselfilter = std::dynamic_pointer_cast<unity::scopes::OptionSelectorFilter const>(filter);
    if (!optselfilter) {
        qWarning() << "OptionSelectorFilter::update(): Unexpected filter" << QString::fromStdString(filter->id()) << "of type" << QString::fromStdString(filter->filter_type());
        return;
    }

    if (optselfilter->multi_select() != m_multiSelect)
    {
        m_multiSelect = optselfilter->multi_select();
        Q_EMIT multiSelectChanged(m_multiSelect);
    }

    if (QString::fromStdString(optselfilter->label()) != m_label)
    {
        m_label = QString::fromStdString(optselfilter->label());
        Q_EMIT labelChanged(m_label);
    }

    m_options->update(optselfilter->options(), filterState);
}

}
