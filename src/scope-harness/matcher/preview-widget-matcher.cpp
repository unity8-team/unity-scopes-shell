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

#include <scope-harness/matcher/preview-widget-matcher.h>
#include <scope-harness/preview/preview-widget.h>

#include <unity/scopes/Variant.h>

#include <boost/optional.hpp>

using namespace std;
using namespace boost;
namespace sc = unity::scopes;

namespace unity
{
namespace scopeharness
{
namespace matcher
{
namespace
{

static void check_string(MatchResult& matchResult, const preview::PreviewWidget& previewWidget,
             const string& name, const string& actualValue,
             const string& expectedValue)
{
    if (actualValue != expectedValue)
    {
        matchResult.failure(
                "PreviewWidget with ID '" + previewWidget.id() + "' has '" + name
                        + "' == '" + actualValue + "' but expected '"
                        + expectedValue + "'");
    }
}

static void check_variant(MatchResult& matchResult, const preview::PreviewWidget& previewWidget,
    const string& name, const sc::Variant& actualValue, const sc::Variant& expectedValue)
{
    if (!(actualValue == expectedValue))
    {
        auto actualValueString = actualValue.serialize_json();
        auto expectedValueString = expectedValue.serialize_json();
        // serialize_json includes a trailing carriage return
        actualValueString.pop_back();
        expectedValueString.pop_back();

        matchResult.failure(
                "PreviewWidget with ID '" + previewWidget.id() + "' has '" + name
                        + "' == '" + actualValueString + "' but expected '"
                        + expectedValueString + "'");
    }
}

}

struct PreviewWidgetMatcher::Priv
{
    string m_id;

    optional<string> m_type;

    optional<sc::Variant> m_data;
};

PreviewWidgetMatcher::PreviewWidgetMatcher(const string& id) :
        p(new Priv)
{
    p->m_id = id;
}

PreviewWidgetMatcher::PreviewWidgetMatcher(const PreviewWidgetMatcher& other) :
        p(new Priv)
{
    *this = other;
}

PreviewWidgetMatcher::PreviewWidgetMatcher(PreviewWidgetMatcher&& other)
{
    *this = move(other);
}

PreviewWidgetMatcher& PreviewWidgetMatcher::operator=(const PreviewWidgetMatcher& other)
{
    p->m_id = other.p->m_id;
    p->m_type = other.p->m_type;
    p->m_data = other.p->m_data;
    return *this;
}

PreviewWidgetMatcher& PreviewWidgetMatcher::operator=(PreviewWidgetMatcher&& other)
{
    p = move(other.p);
    return *this;
}

PreviewWidgetMatcher::~PreviewWidgetMatcher()
{
}

PreviewWidgetMatcher& PreviewWidgetMatcher::type(const string& type)
{
    p->m_type = type;
    return *this;
}

PreviewWidgetMatcher& PreviewWidgetMatcher::data(const sc::Variant& data)
{
    p->m_data = data;
    return *this;
}

PreviewWidgetMatcher& PreviewWidgetMatcher::data(sc::Variant&& data)
{
    p->m_data = move(data);
    return *this;
}

MatchResult PreviewWidgetMatcher::match(const preview::PreviewWidget& previewWidget) const
{
    MatchResult matchResult;
    match(matchResult, previewWidget);
    return matchResult;
}

void PreviewWidgetMatcher::match(MatchResult& matchResult, const preview::PreviewWidget& previewWidget) const
{
    check_string(matchResult, previewWidget, "id", previewWidget.id(), p->m_id);

    if (p->m_type)
    {
        check_string(matchResult, previewWidget, "type", previewWidget.type(), p->m_type.get());
    }

    if (p->m_data)
    {
        auto dict = previewWidget.data().get_dict();
        // No point comparing this transitory field
        dict.erase("session-id");
        check_variant(matchResult, previewWidget, "data", sc::Variant(dict), p->m_data.get());
    }
}

}
}
}
