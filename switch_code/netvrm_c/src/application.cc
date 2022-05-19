#include "application.hpp"
#include "options.hpp"
#include "util.hpp"

#include <random>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <memory>

namespace netvrm {
const Application::ApplicationVec Application::generate_applications(const int num, const SceneType scene_type, const int seed, const int app_duration,
    const int app_len_mean, const int app_len_spread, const int max_concurrent_app, const double input_slo) noexcept {
    std::mt19937 randomizer(seed);
    auto app_duration_distribution = std::uniform_int_distribution<int>(app_len_mean - app_len_spread, app_len_mean + app_len_spread);
    std::vector<int> app_durations(num);
    std::generate(
        app_durations.begin(),
        app_durations.end(),
        [&]() {
            return app_duration_distribution(randomizer);
        }
    );
    std::cout << "generate app duration finish..." << std::endl;
    auto arrival_ts_distribution = std::poisson_distribution<int>(static_cast<double>(app_duration) / num);
    std::vector<int> arrival_ts(num);
    const auto verify_arrival_ts = [&]() {
        if (arrival_ts.back() > app_duration) {
            // Makes sure that the last app arrives within the arriv_duration.
            return false;
        }

        for (int i = 0; i < max_concurrent_app; i++) {
            int j = i + max_concurrent_app;
            while (j < arrival_ts.size()) {
                if (arrival_ts[j] - arrival_ts[j - max_concurrent_app] < app_durations[j - max_concurrent_app]) {
                    return false;
                }
                j += max_concurrent_app;
            }
        }

        return true;
    };

    do {
        std::vector<int> arriv_interval(num);
        std::generate(
            arriv_interval.begin() + 1,     // Leaves the first element as 0.
            arriv_interval.end(),
            [&randomizer, &arrival_ts_distribution]() { return arrival_ts_distribution(randomizer); }
        );

        std::partial_sum(arriv_interval.begin(), arriv_interval.end(), arrival_ts.begin());
    } while (!verify_arrival_ts());
    std::cout << "generate arrival_ts finish..." << std::endl;

    std::vector<ApplicationType> application_types(max_concurrent_app);
    switch (scene_type) {
        case SceneType::HEAVY_HITTER:
            std::fill(application_types.begin(), application_types.end(), ApplicationType::HEAVY_HITTER);
            break;

        case SceneType::SUPERSPREADER:
            std::fill(application_types.begin(), application_types.end(), ApplicationType::SUPERSPREADER);
            break;

        case SceneType::TCP_CONNECTION:
            std::fill(application_types.begin(), application_types.end(), ApplicationType::TCP_CONNECTION);
            break;

        case SceneType::MIXED:
            std::generate(
                application_types.begin(),
                application_types.end(),
                [ &randomizer, uniform_distribution = std::uniform_int_distribution<int>(0, 2) ] () mutable {
                    return static_cast<ApplicationType>(uniform_distribution(randomizer));
                }
            );
    }

    ApplicationVec applications;
    for (int n = 0; n < num; n++) {
        applications.push_back(std::make_shared<Application>(
            n,
            n % max_concurrent_app,
            application_types[n % max_concurrent_app],
            n % max_concurrent_app,
            arrival_ts[n],
            app_durations[n],
            input_slo
        ));
    };

    return applications;
}

const Application::ApplicationVec Application::get_active_applications(Application::ApplicationVec& apps, const int second_idx) noexcept {
    if (second_idx < 0) {
        return {};
    }

    ApplicationVec results;
    std::for_each(std::begin(apps), std::end(apps), [&](value_type_t<decltype(apps)> app) {
        if (app->is_running(second_idx) && !app->rejected && !app->dropped) {
            results.push_back(app);
        }
    });

    return results;
}

const std::array<Application::ApplicationVec, 3> Application::analyze_application_activity_changes(ApplicationVec& prev_sec_apps, ApplicationVec& curr_sec_apps) noexcept {
    // Apps that becomes inactive at this new second.
    ApplicationVec inactive_apps;
    std::set_difference(
        prev_sec_apps.begin(),
        prev_sec_apps.end(),
        curr_sec_apps.begin(),
        curr_sec_apps.end(),
        std::back_inserter(inactive_apps),
        [](value_type_t<decltype(inactive_apps)> first, value_type_t<decltype(inactive_apps)> second) {
            return first->uid < second->uid;
        }
    );

    // Apps that becomes active at this new second.
    ApplicationVec new_active_apps;
    std::set_difference(
        curr_sec_apps.begin(),
        curr_sec_apps.end(),
        prev_sec_apps.begin(),
        prev_sec_apps.end(),
        std::back_inserter(new_active_apps),
        [](value_type_t<decltype(new_active_apps)> first, value_type_t<decltype(new_active_apps)> second) {
            return first->uid < second->uid;
        }
    );

    // Apps that were active previously and are also active now.
    ApplicationVec old_active_apps;
    std::set_intersection(
        curr_sec_apps.begin(),
        curr_sec_apps.end(),
        prev_sec_apps.begin(),
        prev_sec_apps.end(),
        std::back_inserter(old_active_apps),
        [](value_type_t<decltype(old_active_apps)> first, value_type_t<decltype(old_active_apps)> second) {
            return first->uid < second->uid;
        }
    );

    return {inactive_apps, new_active_apps, old_active_apps};
}
}
