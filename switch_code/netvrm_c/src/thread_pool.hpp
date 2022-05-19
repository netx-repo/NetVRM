#pragma once

#include <thread>
#include <array>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <tuple>
#include <future>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <functional>

namespace netvrm {
template<typename s_thread, int thread_num = 4>
class ThreadPool {
public:
    ThreadPool(std::function<s_thread ()>&& thread_init) {
        for (int i = 0; i < thread_num; i++) {
            threads[i] = std::thread([&]() {
                const auto state = thread_init();

                while (!completed) {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    cond_var.wait(lock, [&]() {
                        return !jobs.empty() || completed;
                    });
                    if (!jobs.empty()) {
                        auto job = jobs.front();
                        jobs.pop();
                        lock.unlock();
                        job(state);
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        completed = true;

        cond_var.notify_all();
        for (std::thread &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    template<typename R, typename ...Args>
    auto add(R&& fn, Args&&... args) -> std::future<std::result_of_t<decltype(fn)(s_thread, Args...)>> {
        using ReturnType = std::result_of_t<decltype(fn)(s_thread, Args...)>;
        std::lock_guard<std::mutex> guard(queue_mutex);
        std::function<ReturnType(s_thread)> bound_func = [fn, args...](s_thread state) { return std::bind(fn, state, args...)(); };
        std::shared_ptr<std::packaged_task<ReturnType(s_thread)>> task = std::make_shared<std::packaged_task<ReturnType(s_thread)>>(bound_func);
        std::future<ReturnType> future = task->get_future();
        std::function<void(s_thread)> func = [task](s_thread state) { (*task)(state); };
        jobs.push(std::move(func));
        cond_var.notify_one();
        return future;
    }

    /* Given a function and a list of iteratees, runs the function on each iteratee once, collects the results into one vector and returns it. */
    template<typename Iteratee, typename Fn, typename ...Args>
    auto add_by_iteratee_and_block(std::vector<Iteratee>&& iteratees, Fn&& fn, Args&&... args) -> std::vector<std::result_of_t<Fn(s_thread, Iteratee, Args...)>> {
        using ReturnType = std::result_of_t<Fn(s_thread, Iteratee, Args...)>;
        std::vector<std::future<ReturnType>> futures;
        std::transform(iteratees.begin(), iteratees.end(), std::back_inserter(futures), [&, fn, args...](Iteratee iteratee) mutable {
            return add(fn, iteratee, args...);
        });

        std::vector<ReturnType> results;
        std::transform(futures.begin(), futures.end(), std::back_inserter(results), [](auto &future) {
            future.wait();
            return future.get();
        });
        return results;
    }

private:
    std::array<std::thread, thread_num> threads;
    std::queue<std::function<void(s_thread)>> jobs;
    std::condition_variable cond_var;
    std::mutex queue_mutex;
    bool completed;
};
}
