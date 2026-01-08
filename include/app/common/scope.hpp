#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

template <auto EndFn> class RenderScope {
  public:
    template <typename BeginFn, typename... Args>
    explicit RenderScope(BeginFn&& begin, Args&&... args) {
        std::forward<BeginFn>(begin)(std::forward<Args>(args)...);
    }
    ~RenderScope() { EndFn(); }

    RenderScope(const RenderScope&)            = delete;
    RenderScope(const RenderScope&&)           = delete;
    RenderScope& operator=(const RenderScope&) = delete;
};

static constexpr int64_t NO_ARG                 = 0xdeadbeef;
static constexpr int64_t REQUIRE_ALIVE_FOR_DTOR = 0xc0ffee ^ NO_ARG;

template <auto EndFn, int64_t Arg = NO_ARG> class ImGuiScope {
  public:
    template <typename BeginFn, typename... Args>
    explicit ImGuiScope(BeginFn&& begin, Args&&... args) {
        if constexpr (std::is_same_v<decltype(std::forward<BeginFn>(begin)(
                                         std::forward<Args>(args)...)),
                                     bool>) {
            m_Active = std::forward<BeginFn>(begin)(std::forward<Args>(args)...);
        } else {
            std::forward<BeginFn>(begin)(std::forward<Args>(args)...);
            m_Active = true;
        }
    }

    // I really don't wanna talk about this
    ~ImGuiScope() {
        if constexpr (Arg == REQUIRE_ALIVE_FOR_DTOR) {
            if (!m_Active) { return; }
        }

        if constexpr ((Arg ^ 0xc0ffee) != 0xdeadbeef && Arg != 0xdeadbeef) {
            EndFn(Arg);
        } else {
            EndFn();
        }
    }

    ImGuiScope(const ImGuiScope&)            = delete;
    ImGuiScope(const ImGuiScope&&)           = delete;
    ImGuiScope& operator=(const ImGuiScope&) = delete;

    explicit operator bool() const { return m_Active; }

  private:
    bool m_Active = true;
};

#define IMSCOPE_FN(begin) [&] { return (begin); }
