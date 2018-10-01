#include <gtest/gtest.h>

#include <chrono>
#include <exception>
#include <stdexcept>

#include <engine/async.hpp>
#include <engine/condition_variable.hpp>
#include <engine/mutex.hpp>
#include <engine/sleep.hpp>
#include <utest/utest.hpp>

namespace {

class TestException : public std::exception {};

}  // namespace

TEST(CxxabiEhGlobals, UncaughtIsCoroLocal) {
  TestInCoro([] {
    try {
      engine::Mutex mutex;
      engine::ConditionVariable cv;
      std::unique_lock<engine::Mutex> lock(mutex);

      auto subtask = engine::Async([&cv, &mutex] {
        {
          std::unique_lock<engine::Mutex> lock(mutex);
          cv.NotifyOne();
        }
        engine::SleepFor(std::chrono::seconds(1));

        // if we got here, subtask wasn't cancelled while it should've been
        // one of possible reasons is uncaught exception leaked via thread local
        ASSERT_FALSE(std::uncaught_exception());
        FAIL() << "Subtask wasn't cancelled";
      });
      cv.Wait(lock);

      // we'll switch to subtask during stack unwinding (in its dtor)
      throw TestException{};
    } catch (const TestException&) {
      return;
    }
    FAIL() << "Exception has been lost";
  });
}

TEST(CxxabiEhGlobals, ActiveIsCoroLocal) {
  TestInCoro([] {
    engine::Mutex mutex;
    engine::ConditionVariable cv;
    engine::ConditionVariable sub_cv;
    std::unique_lock<engine::Mutex> lock(mutex);

    auto subtask = engine::Async([&cv, &mutex, &sub_cv] {
      std::unique_lock<engine::Mutex> lock(mutex);
      cv.NotifyOne();
      sub_cv.Wait(lock);

      // this coro shouldn't have an active exception
      ASSERT_FALSE(std::current_exception());
      cv.NotifyOne();
    });
    cv.Wait(lock);

    try {
      throw TestException{};
    } catch (const TestException&) {
      sub_cv.NotifyOne();
      cv.WaitFor(lock, std::chrono::seconds(1));

      // this coro didn't lose an active exception
      ASSERT_TRUE(std::current_exception());
    }
  });
}
