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

#pragma once

#include <unity/shell/scopes/ScopeInterface.h>

#include <scope-harness/view/preview-view.h>
#include <scope-harness/results/category.h>
#include <scope-harness/results/department.h>
#include <scope-harness/preview/preview-widget.h>
#include <scope-harness/view/settings-view.h>

#include <string>

#include <QVariantMap>

namespace scopes_ng
{
class Scopes;
}

namespace unity
{
namespace shell
{
namespace scopes
{
class CategoriesInterface;
}
}
namespace scopeharness
{
class ScopeHarness;

namespace internal
{
struct ResultsViewArguments;
}
namespace view
{
class PreviewView;

class Q_DECL_EXPORT ResultsView final: public AbstractView
{
public:
    UNITY_DEFINES_PTRS(ResultsView);

    ResultsView(const internal::ResultsViewArguments& arguments);

    ~ResultsView() = default;

    ResultsView(const ResultsView& other) = delete;

    ResultsView(ResultsView&& other) = delete;

    ResultsView& operator=(const ResultsView& other) = delete;

    ResultsView& operator=(ResultsView&& other) = delete;

    void setQuery(const std::string& searchString);

    std::string query() const;

    void setActiveScope(const std::string& id);

    std::string activeScope() const;

    void waitForResultsChange();

    bool overrideCategoryJson(std::string const& categoryId, std::string const& json);

    std::string scopeId() const;

    std::string displayName() const;

    std::string iconHint() const;

    std::string description() const;

    std::string searchHint() const;

    std::string shortcut() const;

    unity::scopes::Variant customizations() const;

    std::string sessionId() const;

    int queryId() const;

    void forceRefresh();

    results::Category::List categories();

    results::Category category(std::size_t row);

    results::Category category(const std::string& categoryId);

    unity::shell::scopes::ScopeInterface::Status status() const;

    // Navigation

    bool hasDepartments() const;

    bool hasAltDepartments() const;

    std::string departmentId() const;

    std::string altDepartmentId() const;

    results::Department browseDepartment(const std::string& id = std::string());

    results::Department browseAltDepartment(const std::string& id = std::string());

    SettingsView settings() const;

protected:
    friend ScopeHarness;
    friend preview::PreviewWidget;

    void setPreviewView(std::shared_ptr<PreviewView> previewView);

    struct _Priv;

    std::shared_ptr<_Priv> p;
};

}
}
}
