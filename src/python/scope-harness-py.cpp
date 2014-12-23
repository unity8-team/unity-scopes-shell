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

static void run_qt()
{
    /*std::thread t([]() {
            int argc = 0;
            char **argv;
            QCoreApplication app(argc, argv);
            std::cerr << "starting QCoreApplication\n";
            app.exec();
            });
    t.detach();
    std::this_thread::sleep_for(std::chrono::seconds(1));*/
}

class ScopeHarnessWrapper;

class Worker: public QObject
{
    Q_OBJECT
    public:
        Worker(const sh::registry::CustomRegistry::Parameters& parameters):
            parameters_(parameters)
        {
        }

        sh::ScopeHarness::UPtr get()
        {
            auto ft = promise.get_future();
            return ft.get();
        }

    public Q_SLOTS:
        void process()
        {
            promise.set_value(sh::ScopeHarness::newFromScopeList(parameters_));
        }

    private:
        const sh::registry::CustomRegistry::Parameters& parameters_;
        std::promise<sh::ScopeHarness::UPtr> promise;
};

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
            std::thread t([]() {
                int argc = 0;
                char **argv = {nullptr};
                QCoreApplication app(argc, argv);
                app.exec();
            });
            t.detach();

            QThread *fakeThread = new QThread();
            Worker *worker = new Worker(parameters);
            worker->moveToThread(fakeThread);
            QObject::connect(fakeThread, SIGNAL(started()), worker, SLOT(process()));
            fakeThread->start();
            sh::ScopeHarness::SPtr ptr = worker->get();
            return ScopeHarnessWrapper::UPtr(new ScopeHarnessWrapper(ptr));
        }
        ~ScopeHarnessWrapper() = default;

    private:
        sh::ScopeHarness::SPtr scope_harness_;
};

void export_scopeharness()
{
    def("run_qt", &run_qt);
    boost::python::register_ptr_to_python<std::shared_ptr<ScopeHarnessWrapper>>();
    class_<ScopeHarnessWrapper>("ScopeHarness", no_init)
        .add_property("results_view", &ScopeHarnessWrapper::resultsView)
        //.def("new_from_pre_existing_config", &sh::ScopeHarness::newFromPreExistingConfig).staticmethod("new_from_pre_existing_config")
        .def("new_from_scope_list", &ScopeHarnessWrapper::newFromScopeList).staticmethod("new_from_scope_list")
        //.def("new_from_system", &sh::ScopeHarness::newFromSystem).staticmethod("new_from_system")
    ;
}

#include "scope-harness-py.moc"
