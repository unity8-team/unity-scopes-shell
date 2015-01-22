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
 * Author: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <boost/python.hpp>
#include <scope-harness/scope-harness.h>
#include <thread>
#include <chrono>
#include <QCoreApplication>
#include <QThread>
#include <future>
#include <iostream>

using namespace boost::python;
namespace sh = unity::scopeharness;

class ScopeHarnessWrapper
{
    public:
        UNITY_DEFINES_PTRS(ScopeHarnessWrapper);

        ScopeHarnessWrapper(const sh::ScopeHarness::SPtr& shptr)
            : scope_harness_(shptr)
        {}

        sh::view::ResultsView::SPtr resultsView()
        {
            return scope_harness_->resultsView();
        }

        static ScopeHarnessWrapper::SPtr newFromScopeList(const sh::registry::CustomRegistry::Parameters& parameters)
        {
            run_qt();
            sh::ScopeHarness::SPtr ptr = sh::ScopeHarness::newFromScopeList(parameters);
            return ScopeHarnessWrapper::UPtr(new ScopeHarnessWrapper(ptr));
        }

        static ScopeHarnessWrapper::SPtr newFromPreExistingConfig(const std::string& directory)
        {
            run_qt();
            sh::ScopeHarness::SPtr ptr = sh::ScopeHarness::newFromPreExistingConfig(directory);
            return ScopeHarnessWrapper::UPtr(new ScopeHarnessWrapper(ptr));
        }

        static ScopeHarnessWrapper::SPtr newFromSystem()
        {
            run_qt();
            sh::ScopeHarness::SPtr ptr = sh::ScopeHarness::newFromSystem();
            return ScopeHarnessWrapper::UPtr(new ScopeHarnessWrapper(ptr));
        }

        ~ScopeHarnessWrapper() = default;

    private:
        static void run_qt()
        {
            int argc = 0;
            char **argv = {nullptr}; // FIXME: pass argv from python
            static std::unique_ptr<QCoreApplication> coreApp(QCoreApplication::instance() ? nullptr
                    : new QCoreApplication(argc, argv));
        }

        sh::ScopeHarness::SPtr scope_harness_;
};

void export_scopeharness()
{
    boost::python::register_ptr_to_python<std::shared_ptr<ScopeHarnessWrapper>>();
    class_<ScopeHarnessWrapper>("ScopeHarness",
                                "This is the main class for scope harness testing. An instance of it needs to be created "
                                "using one of the static class methods (new_from_*) before any tests can be performed. "
                                "The instance of ResultsView provided by results_view property is the entry point for "
                                "invoking actual queries.",
                                no_init)
        .add_property("results_view", &ScopeHarnessWrapper::resultsView)
        .def("new_from_pre_existing_config", &ScopeHarnessWrapper::newFromPreExistingConfig,
             "Creates ScopeHarness instance from scope runtime configuration files in given directory").staticmethod("new_from_pre_existing_config")
        .def("new_from_scope_list", &ScopeHarnessWrapper::newFromScopeList,
             "Creates ScopeHarness instance from a configuration provided by an"
             " instance of CustomRegistry passed to this factory method").staticmethod("new_from_scope_list")
        .def("new_from_system", &ScopeHarnessWrapper::newFromSystem,
             "Creates ScopeHarness instance using default configuration from the system").staticmethod("new_from_system")
    ;
}
