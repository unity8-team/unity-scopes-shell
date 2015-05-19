/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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
 *
 * Authors:
 *  Pete Woods <pete.woods@canonical.com>
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <QTest>

#include <scope-harness/scope-harness.h>
#include <scope-harness/matcher/settings-matcher.h>
#include <scope-harness/matcher/settings-option-matcher.h>
#include <scope-harness/view/settings-view.h>
#include <scope-harness/test-utils.h>

namespace sh = unity::scopeharness;
namespace shm = unity::scopeharness::matcher;
namespace shr = unity::scopeharness::registry;
namespace shv = unity::scopeharness::view;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

class SettingsEndToEndTest : public QObject
{
    Q_OBJECT
private:
    sh::ScopeHarness::UPtr m_harness;

private Q_SLOTS:

    void cleanupTestCase()
    {
    }

    void init()
    {
        qputenv("UNITY_SCOPES_NO_WAIT_LOCATION", "1");
        m_harness = sh::ScopeHarness::newFromScopeList(
            shr::CustomRegistry::Parameters({
                TEST_DATA_DIR "mock-scope-departments/mock-scope-departments.ini", 
                TEST_DATA_DIR "mock-scope-double-nav/mock-scope-double-nav.ini", 
                TEST_DATA_DIR "mock-scope/mock-scope.ini",
            })
        );
    }

    void testBasic()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");

        auto settings = resultsView->settings();

        QVERIFY(settings.get());
        QCOMPARE(static_cast<long>(settings->count()), 5l);

        QVERIFY_MATCHRESULT(
                shm::SettingsMatcher().mode(shm::SettingsMatcher::Mode::by_id)
                    .hasAtLeast(1)
                    .option(
                            shm::SettingsOptionMatcher("distanceUnit")
                                .displayName("Distance Unit")
                                .optionType(shv::SettingsView::OptionType::List)
                                .displayValues(sc::VariantArray {sc::Variant("Kilometers"), sc::Variant("Miles")})
                                .value(sc::Variant("Miles"))
                                .defaultValue(sc::Variant("Miles"))
                        )
                        .match(settings)
                );

        QVERIFY_MATCHRESULT(
                shm::SettingsMatcher().mode(shm::SettingsMatcher::Mode::starts_with)
                    .hasAtLeast(1)
                    .option(
                        shm::SettingsOptionMatcher("location")
                            .displayName("Location")
                            .optionType(shv::SettingsView::OptionType::String)
                            .value(sc::Variant("London"))
                            .defaultValue(sc::Variant("London"))
                        )
                    .match(settings)
                );

        QVERIFY_MATCHRESULT(
                shm::SettingsMatcher().mode(shm::SettingsMatcher::Mode::all)
                    .hasExactly(5)
                    .option(
                        shm::SettingsOptionMatcher("location")
                            .displayName("Location")
                            .optionType(shv::SettingsView::OptionType::String)
                            .value(sc::Variant("London"))
                            .defaultValue(sc::Variant("London"))
                        )
                    .option(
                        shm::SettingsOptionMatcher("distanceUnit")
                            .displayName("Distance Unit")
                            .optionType(shv::SettingsView::OptionType::List)
                            .displayValues(sc::VariantArray {sc::Variant("Kilometers"), sc::Variant("Miles")})
                            .value(sc::Variant("Miles"))
                            .defaultValue(sc::Variant("Miles"))
                        )
                    .option(
                        shm::SettingsOptionMatcher("age")
                            .displayName("Age")
                            .optionType(shv::SettingsView::OptionType::Number)
                            .value(sc::Variant(23))
                            .defaultValue(sc::Variant(23))
                        )
                    .option(
                        shm::SettingsOptionMatcher("enabled")
                            .displayName("Enabled")
                            .optionType(shv::SettingsView::OptionType::Boolean)
                            .value(sc::Variant(true))
                            .defaultValue(sc::Variant(true))
                        )
                    .option(
                        shm::SettingsOptionMatcher("color")
                            .displayName("Color")
                            .optionType(shv::SettingsView::OptionType::String)
                            .value(sc::Variant())
                            .defaultValue(sc::Variant())
                        )
                    .match(settings)
                );
    }

    void verifySettingsChangedSignal()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");

        auto settings = resultsView->settings();
        settings->set("location", sc::Variant("Barcelona"));

        QVERIFY_MATCHRESULT(
                shm::SettingsMatcher().mode(shm::SettingsMatcher::Mode::starts_with)
                    .option(
                        shm::SettingsOptionMatcher("location")
                            .displayName("Location")
                            .optionType(shv::SettingsView::OptionType::String)
                            .value(sc::Variant("Barcelona"))
                            .defaultValue(sc::Variant("London"))
                        )
                    .match(settings)
                );
    }

    void testChildScopes()
    {
        auto resultsView = m_harness->resultsView();
        // make aggregator scope active
        resultsView->setActiveScope("mock-scope-departments");

        auto settings = resultsView->settings();
        QVERIFY(settings.get());
        QCOMPARE(static_cast<long>(settings->count()), 4l);

        QVERIFY_MATCHRESULT(
                shm::SettingsMatcher().mode(shm::SettingsMatcher::Mode::all)
                    .option(
                        shm::SettingsOptionMatcher("string-setting")
                            .displayName("String Setting")
                            .optionType(shv::SettingsView::OptionType::String)
                            .value(sc::Variant("Hello"))
                            .defaultValue(sc::Variant("Hello"))
                        )
                    .option(
                        shm::SettingsOptionMatcher("number-setting")
                            .displayName("Number Setting")
                            .optionType(shv::SettingsView::OptionType::Number)
                            .value(sc::Variant(13))
                            .defaultValue(sc::Variant(13))
                        )
                    .option(
                        shm::SettingsOptionMatcher("mock-scope-double-nav")
                            .displayName("Display results from mock-double-nav.DisplayName")
                            .optionType(shv::SettingsView::OptionType::Boolean)
                            .value(sc::Variant(true))
                            .defaultValue(sc::Variant())
                        )
                    .option(
                        shm::SettingsOptionMatcher("mock-scope")
                            .displayName("Display results from mock.DisplayName")
                            .optionType(shv::SettingsView::OptionType::Boolean)
                            .value(sc::Variant(true))
                            .defaultValue(sc::Variant())
                        )
                    .match(settings)
                );
    }
};

QTEST_GUILESS_MAIN(SettingsEndToEndTest)
#include <settingsendtoendtest.moc>
