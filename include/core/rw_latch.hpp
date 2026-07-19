#pragma once

#include <shared_mutex>

#include <stdx/utility.hh>

#ifndef NDEBUG
#    include <atomic>

#    include <stdx/types.hh>
#endif

namespace mbr {

// A thin wrapper over a shared mutex for instrumented latching
class rw_latch {
  public:
    rw_latch() noexcept = default;
    ~rw_latch()         = default;
    MAKE_PINNED(rw_latch);

    void               lock();
    [[nodiscard]] bool try_lock();
    void               unlock();

    void               lock_shared();
    [[nodiscard]] bool try_lock_shared();
    void               unlock_shared();

  private:
    void note_exclusive_acquired() noexcept;
    void note_exclusive_released() noexcept;
    void note_shared_acquired() noexcept;
    void note_shared_released() noexcept;

  private:
    std::shared_mutex mutex_;
#ifndef NDEBUG
    std::atomic<i32>  readers_{0};
    std::atomic<bool> writer_{false};
#endif
};

} // namespace mbr
