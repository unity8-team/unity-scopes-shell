/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 * Author: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <scope-harness/view/settings-view.h>
#include <unity/shell/scopes/SettingsModelInterface.h>
#include <scope-harness/internal/settings-view-arguments.h>
#include <Unity/utils.h>
#include <scope-harness/test-utils.h>
#include <QSignalSpy>

namespace ss = unity::shell::scopes;

using namespace std;

namespace unity
{
namespace scopeharness
{
namespace view
{

struct SettingsView::_Priv
{
    ng::Scope::Ptr m_scope;
};

SettingsView::SettingsView(const internal::SettingsViewArguments& args)
    : p(new _Priv)
{
    p->m_scope = args.m_scope;
}

SettingsView::Option::List SettingsView::options() const
{
    //
    // convert all current options from settings model to SettingsView::Option::List
    auto settings = p->m_scope->settings();
    Option::List opts;
    for (int i = 0; i<settings->count(); i++)
    {
        Option opt;

        auto const index = settings->index(i, 0);
        auto const optType = settings->data(index, ss::SettingsModelInterface::Roles::RoleType).toString().toStdString();

        opt.id = settings->data(index, ss::SettingsModelInterface::Roles::RoleSettingId).toString().toStdString();
        opt.value = ng::qVariantToScopeVariant(settings->data(index, ss::SettingsModelInterface::Roles::RoleValue));
        opt.displayName = settings->data(index, ss::SettingsModelInterface::Roles::RoleDisplayName).toString().toStdString();
        auto props = settings->data(index, ss::SettingsModelInterface::Roles::RoleProperties).toMap();

        if (props.contains("defaultValue"))
        {
            opt.defaultValue = ng::qVariantToScopeVariant(props["defaultValue"]);
        }
        if (props.contains("values"))
        {
            opt.displayValues = ng::qVariantToScopeVariant(props["values"]).get_array();
        }

        if (optType == "string")
        {
            opt.optionType = OptionType::String;
        }
        else if (optType == "number")
        {
            opt.optionType = OptionType::Number;
        }
        else if (optType == "boolean")
        {
            opt.optionType = OptionType::Boolean;
        }
        else if (optType == "list")
        {
            // for list type, the value is an index in the list;
            // make value keep actual option string.
            opt.optionType = OptionType::List;
            long i = opt.value.get_int();
            if (i < static_cast<long>(opt.displayValues.size()))
            {
                opt.value = opt.displayValues[i];
            }
            else
            {
                throw std::domain_error("Failed to process list option with ID '" + opt.id + "'. No value for index " + to_string(i));
            }
        }
        else
        {
            throw std::domain_error("Failed to process settings. Unknown option type for option with ID " + opt.id + "'");
        }

        opts.push_back(opt);
    }
    return opts;
}

std::size_t SettingsView::count() const
{
    return p->m_scope->settings()->count();
}

void SettingsView::set(const std::string& option_id, const unity::scopes::Variant &value)
{
    auto settings = p->m_scope->settings();
    for (auto i = 0; i<settings->count(); i++)
    {
        auto const index = settings->index(i, 0);
        auto const id = settings->data(index, ss::SettingsModelInterface::Roles::RoleSettingId).toString().toStdString();
        if (id == option_id)
        {
            QSignalSpy settingChangedSpy(settings, SIGNAL(settingsChanged()));
            settings->setData(index, ng::scopeVariantToQVariant(value), ss::SettingsModelInterface::Roles::RoleValue);
            TestUtils::throwIfNot(settingChangedSpy.wait(), "Settings update failed");
            return;
        }
    }
    throw std::domain_error("Setting update failed. No such option: '" + option_id + "'");
}

}
}
}
