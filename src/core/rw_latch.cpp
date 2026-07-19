#include "core/rw_latch.hpp"

#include <stdx/profiler.hh>

#ifndef NDEBUG
#    include <stdx/assert.hh>
#endif

namespace mbr {

void rw_latch::lock() {
    {
        PROFILE_SCOPE("rw_latch::lock");
        mutex_.lock();
    }
    note_exclusive_acquired();
}

bool rw_latch::try_lock() {
    if (!mutex_.try_lock()) { return false; }
    note_exclusive_acquired();
    return true;
}

void rw_latch::unlock() {
    note_exclusive_released();
    mutex_.unlock();
}

void rw_latch::lock_shared() {
    {
        PROFILE_SCOPE("rw_latch::lock_shared");
        mutex_.lock_shared();
    }
    note_shared_acquired();
}

bool rw_latch::try_lock_shared() {
    if (!mutex_.try_lock_shared()) { return false; }
    note_shared_acquired();
    return true;
}

void rw_latch::unlock_shared() {
    note_shared_released();
    mutex_.unlock_shared();
}

void rw_latch::note_exclusive_acquired() noexcept {
#ifndef NDEBUG
    ASSERT(readers_.load() == 0, "exclusive lock acquired while readers present");
    ASSERT(!writer_.exchange(true), "exclusive lock acquired while already write-held");
#endif
}

void rw_latch::note_exclusive_released() noexcept {
#ifndef NDEBUG
    ASSERT(writer_.exchange(false), "exclusive unlock without a matching lock");
#endif
}

void rw_latch::note_shared_acquired() noexcept {
#ifndef NDEBUG
    ASSERT(!writer_.load(), "shared lock acquired while write-held");
    readers_.fetch_add(1);
#endif
}

void rw_latch::note_shared_released() noexcept {
#ifndef NDEBUG
    ASSERT(readers_.fetch_sub(1) > 0, "shared unlock without a matching lock");
#endif
}

} // namespace mbr
