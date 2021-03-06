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
#include <QQmlEngine>
#include <QDebug>

namespace scopes_ng
{

OptionSelectorFilter::OptionSelectorFilter(unity::scopes::OptionSelectorFilter::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent)
    : unity::shell::scopes::OptionSelectorFilterInterface(parent),
    m_id(QString::fromStdString(filter->id())),
    m_title(QString::fromStdString(filter->title())),
    m_multiSelect(filter->multi_select()),
    m_label(QString::fromStdString(filter->label())),
    m_options(new OptionSelectorOptions(this, filter->options(), filter->active_options(*filterState))),
    m_filterState(filterState),
    m_filter(filter)
{
    QQmlEngine::setObjectOwnership(m_options.data(), QQmlEngine::CppOwnership);
    connect(m_options.data(), SIGNAL(optionChecked(const QString&, bool)), this, SLOT(onOptionChecked(const QString&, bool)));
}

QString OptionSelectorFilter::filterId() const
{
    return m_id;
}

QString OptionSelectorFilter::title() const
{
    return m_title;
}

unity::shell::scopes::FiltersInterface::FilterType OptionSelectorFilter::filterType() const
{
    return unity::shell::scopes::FiltersInterface::FilterType::OptionSelectorFilter;
}

QString OptionSelectorFilter::label() const
{
    return m_label;
}

bool OptionSelectorFilter::multiSelect() const
{
    return m_multiSelect;
}

void OptionSelectorFilter::reset()
{
    if (auto state = m_filterState.lock())
    {
        state->remove(m_filter->id());
        qDebug() << "Removing filter state for filter" << QString::fromStdString(m_filter->id());
        m_options->update(m_filter->active_options(*state), true);
        Q_EMIT filterStateChanged();
    }
}

void OptionSelectorFilter::onOptionChecked(const QString& id, bool checked)
{
    if (auto state = m_filterState.lock())
    {
        auto const optid = id.toStdString();
        for (auto const opt: m_filter->options())
        {
            if (opt->id() == optid)
            {
                m_filter->update_state(*state, opt, checked);
                // onOptionChecked signal is triggered by the user, but we need to updated filter
                // with new state since the state of other options may have changed if this is a single-selection filter.
                // However, we pass allow_defaults = false, so that user is able to unselect all options and they are
                // not forcefully reset to defaults if this happens.
                m_options->update(m_filter->active_options(*state), false);
                Q_EMIT filterStateChanged();
                return;
            }
        }
        qDebug() << "Removing filter state for filter" << QString::fromStdString(m_filter->id());
        state->remove(m_filter->id());
    }
}

unity::shell::scopes::OptionSelectorOptionsInterface* OptionSelectorFilter::options() const
{
    return m_options.data();
}

void OptionSelectorFilter::update(unity::scopes::FilterState::SPtr const& filterState)
{
    m_filterState = filterState;
    m_options->update(m_filter->active_options(*filterState), true);
}

void OptionSelectorFilter::update(unity::scopes::FilterBase::SCPtr const& filter)
{
    unity::scopes::OptionSelectorFilter::SCPtr optselfilter = std::dynamic_pointer_cast<unity::scopes::OptionSelectorFilter const>(filter);
    if (!optselfilter) {
        qWarning() << "OptionSelectorFilter::update(): Unexpected filter" << QString::fromStdString(filter->id()) << "of type" << QString::fromStdString(filter->filter_type());
        return;
    }

    m_filter = optselfilter;

    if (optselfilter->multi_select() != m_multiSelect)
    {
        m_multiSelect = optselfilter->multi_select();
        Q_EMIT multiSelectChanged(m_multiSelect);
    }

    if (optselfilter->title() != m_title.toStdString())
    {
        m_title = QString::fromStdString(optselfilter->title());
        Q_EMIT titleChanged();
    }

    if (QString::fromStdString(optselfilter->label()) != m_label)
    {
        m_label = QString::fromStdString(optselfilter->label());
        Q_EMIT labelChanged(m_label);
    }

    m_options->update(optselfilter->options());
    if (auto state = m_filterState.lock()) {
        m_options->update(m_filter->active_options(*state), false);
    }
}

int OptionSelectorFilter::activeFiltersCount() const
{
    if (auto state = m_filterState.lock()) {
        return static_cast<int>(m_filter->has_active_option(*state));
    }
    return 0;
}

QString OptionSelectorFilter::filterTag() const
{
    if (m_multiSelect) {
        // multi-selection filter cannot be used as primary navigation
        return "";
    }
    if (auto state = m_filterState.lock()) {
        auto selected = m_filter->active_options(*state);
        if (selected.size() > 0) {
            for (auto opt: m_filter->options()) {
                if (selected.find(opt) != selected.end()) {
                    return QString::fromStdString(opt->label()); // TODO: i18n
                }
            }
        }
    }
    return "";
}

}
