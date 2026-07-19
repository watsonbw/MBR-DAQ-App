#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <imgui.h>
#include <opencv2/core/mat.hpp>
#include <sokol_gfx.h>
#include <stdx/option.hh>

#include "app/assets/texture.hpp"
#include "app/context.hpp"
#include "app/pages/page.hpp"
#include "core/time.hpp"

namespace mbr::pages {

class view_page : public page {
  public:
    using selected_video_t    = stdx::option<std::pair<std::string, stdx::option<date_time>>>;
    using selected_txt_file_t = stdx::option<std::string>;

    enum class data_view_t : uint8_t {
        ALL,
        RPMDATA,
        SHOCKDATA,
    };

    [[nodiscard]] static const char* data_type_string(data_view_t type);

  public:
    explicit view_page(const std::shared_ptr<app_context>& ctx);
    ~view_page() override;

    void on_enter() override;
    void on_exit() override;
    void update() override;

  private:
    void cleanup();

    void draw_lhs();
    void draw_lhs_controls();
    void draw_open_video();

    void draw_rhs();
    void draw_open_text();
    void draw_sync_video_buttons();

    selected_video_t open_video_file(const std::string& previous_file);
    void             request_seek(int frame_index);

    selected_txt_file_t  open_text_file(const std::string& previous_file);
    void                 load_data();
    stdx::option<size_t> sync_data_video(const std::vector<uint64_t>& micros_times);
    void                 delete_extra(size_t erase_pos);
    void                 dynamic_plot_start();
    void                 dynamic_plot_loop();

    void start_decoding_thread();
    void stop_decoding_thread();
    void update_texture(bool is_timer_tick);

    void try_cleanup_sokol_resources();

  private:
    std::shared_ptr<bool> is_alive_;

    std::string              video_path_;
    stdx::option<local_time> video_creation_ts_;
    std::string              creation_metadata_text_buf_;
    std::atomic<bool>        video_dialog_running_{false};
    std::mutex               video_path_mutex_;
    selected_video_t         selected_video_;
    std::string              input_time_;
    bool                     video_loaded_{false};
    bool                     video_hovered_{false};

    bool   dynamic_plotting_{false};
    size_t plot_percent_;
    double points_per_{0.0};
    double data_count_{0.0};
    double data_from_end_{0.0};

    std::string         txt_path_;
    std::atomic<bool>   txt_dialog_running_{false};
    std::mutex          txt_path_mutex_;
    selected_txt_file_t selected_txt_;
    bool                txt_loaded_{false};
    bool                data_and_time_sync_{false};

    int         total_frames_{0};
    double      video_fps_{0.0};
    double      video_length_minutes_{0.0};
    std::string video_length_formatted_;
    double      frame_duration_{0.0};
    double      time_accumulator_{0.0};
    int         current_frame_ui_{0};

    std::atomic<bool> is_playing_{false};
    std::atomic<bool> is_looping_{false};
    std::atomic<int>  seek_target_{-1};
    std::atomic<bool> force_update_frame_{false};
    data_view_t       data_show_{data_view_t::ALL};

    std::jthread                        decode_thread_;
    std::atomic<bool>                   thread_running_{false};
    std::mutex                          frame_mutex_;
    std::deque<std::pair<cv::Mat, int>> frame_queue_;
    std::condition_variable             queue_cv_;

    ImVec2                 button_size_{24, 24};
    assets::button_texture play_button_;
    assets::button_texture pause_button_;
    assets::button_texture step_button_;

    sg_image    video_texture_{SG_INVALID_ID};
    sg_view     video_view_{SG_INVALID_ID};
    ImTextureID video_texture_id_{0};
    int         texture_width_{0};
    int         texture_height_{0};

    bool timestamp_input_focused_{false};
};

} // namespace mbr::pages
