#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>

#include <gsl/util>
#include <tinyfiledialogs.h>

#include <fmt/format.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>

#include "core/log.hpp"

#include "app/assets/images/image_buttons.hpp"
#include "app/pages/utils.hpp"
#include "app/pages/view.hpp"
#include "app/style.hpp"

using namespace std::chrono_literals;

namespace mbr::pages {

static constexpr size_t MAX_QUEUE_SIZE = 10;

const char* view_page::data_type_string(data_view_t type) {
    switch (type) {
    case data_view_t::ALL:       return "All Data Shown";
    case data_view_t::RPMDATA:   return "RPM Data Shown";
    case data_view_t::SHOCKDATA: return "Shock Data Shown";
    default:                     return "Unknown";
    }
}

view_page::view_page(const std::shared_ptr<app_context>& ctx)
    : page_base{ctx}, is_alive_{std::make_shared<bool>(true)},
      play_button_{assets::PLAY_BUTTON_PNG}, pause_button_{assets::PAUSE_BUTTON_PNG},
      step_button_{assets::STEP_BUTTON_PNG} {}

view_page::~view_page() {
    *is_alive_ = false;
    cleanup();
    LOG_INFO("Destroyed ViewPage");
}

void view_page::on_enter() { LOG_INFO("Entered ViewPage"); }

void view_page::on_exit() {
    cleanup();
    LOG_INFO("Exited ViewPage");
}

void view_page::update() {
    if (ImGui::BeginTable(
            "##viewsplt", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable)) {
        const auto cleanup_table{gsl::finally(ImGui::EndTable)};
        ImGui::TableNextColumn();
        draw_lhs();
        ImGui::TableNextColumn();
        draw_rhs();
    }
}

void view_page::cleanup() {
    stop_decoding_thread();
    try_cleanup_sokol_resources();
}

void view_page::draw_lhs() {
    if (ImGui::BeginChild("##video")) {
        const auto cleanup_video{gsl::finally(ImGui::EndChild)};
        draw_open_video();
        if (video_loaded_ && !video_path_.empty()) {
            ImGui::SameLine();
            const std::filesystem::path path{video_path_};
            const auto                  filename_str = path.filename().string();
            BOLD_DEFAULT(ImGui::TextUnformatted("Current File: "));
            ImGui::SameLine();
            ImGui::TextUnformatted(filename_str.c_str());
        }
        ImGui::SameLine();

        // Clear Video button is right aligned
        const char*  label       = "Clear Video";
        const ImVec2 button_size = ImGui::CalcTextSize(label);
        const float  padding     = ImGui::GetStyle().FramePadding.x * 2.0F;
        const float  right_x     = ImGui::GetWindowContentRegionMax().x - (button_size.x + padding);

        ImGui::SetCursorPosX(right_x);
        if (ImGui::Button(label)) {
            stop_decoding_thread();
            is_playing_ = false;
            video_path_ = {};
            try_cleanup_sokol_resources();
            return;
        }

        ImGui::Separator();

        // Playback logic and frame skipping can be ignored if paused
        bool is_timer_tick = false;
        if (is_playing_ && video_fps_ > 0.0F) {
            time_accumulator_ += ImGui::GetIO().DeltaTime;
            const auto frames_to_advance = static_cast<int>(time_accumulator_ / frame_duration_);

            // We have to handle skipped frames gracefully
            if (frames_to_advance > 0) {
                time_accumulator_ -= (frames_to_advance * frame_duration_);

                if (frames_to_advance > 1) {
                    const std::scoped_lock<std::mutex> lock{frame_mutex_};

                    const auto recoverable_frames =
                        std::min(static_cast<int>(frame_queue_.size()) - 1, frames_to_advance - 1);
                    for (auto i = 0; i < recoverable_frames; i++) { frame_queue_.pop_front(); }
                }

                is_timer_tick = true;
            }
        }

        update_texture(is_timer_tick);

        if (video_texture_.id != SG_INVALID_ID && texture_width_ > 0) {
            const float aspect =
                static_cast<float>(texture_width_) / static_cast<float>(texture_height_);
            const float avail_w = ImGui::GetContentRegionAvail().x;
            const float h       = avail_w / aspect;

            ImGui::Image(video_texture_id_, {avail_w, h});
            video_hovered_ = ImGui::IsItemHovered();
        }

        if (thread_running_) {
            ImGui::Separator();
            draw_lhs_controls();
        }
    }
}

void view_page::draw_lhs_controls() {
    // Slider
    const auto current_timestamp_min =
        (static_cast<double>(current_frame_ui) / total_frames_) * video_length_minutes_;
    const auto current_timestamp = local_time::from_minutes(current_timestamp_min);
    const auto formatted_timestamp =
        current_timestamp.value_or(local_time::zero()).to_string(false);
    ImGui::Text("%s / %s", formatted_timestamp.c_str(), video_length_formatted_.c_str());
    ImGui::SameLine();

    {
        ImGui::PushItemWidth(-1);
        const auto cleanup_width{gsl::finally(ImGui::PopItemWidth)};

        int slider_pos = current_frame_ui;
        if (ImGui::SliderInt(
                "##scrub", &slider_pos, 0, total_frames_, "", ImGuiSliderFlags_NoInput)) {
            is_playing_.exchange(false);
            request_seek(slider_pos);
        }
    }

    // Loop checkbox
    bool looping = is_looping_;
    if (ImGui::Checkbox("Looping", &looping)) { is_looping_ = looping; }
    ImGui::SameLine();

    // Play/Pause
    const auto is_playing = is_playing_.load();
    const auto tint_color = context_->style.dark_mode ? ImVec4{1, 1, 1, 1} : ImVec4{-1, -1, -1, 1};
    ImGui::SameLine();
    if (ImGui::ImageButton("##stepback",
                           step_button_.get_id(),
                           button_size_,
                           {1, 0},
                           {0, 1},
                           {0, 0, 0, 0},
                           tint_color)) {
        request_seek(current_frame_ui - 5);
        is_playing_.exchange(false);
    }
    ImGui::SameLine();
    if (ImGui::ImageButton("##playpause",
                           is_playing ? pause_button_.get_id() : play_button_.get_id(),
                           button_size_,
                           {0, 0},
                           {1, 1},
                           {0, 0, 0, 0},
                           tint_color)) {
        is_playing_.exchange(!is_playing);
        time_accumulator_ = 0.0;
    }
    ImGui::SameLine();
    if (ImGui::ImageButton("##stepforward",
                           step_button_.get_id(),
                           button_size_,
                           {0, 0},
                           {1, 1},
                           {0, 0, 0, 0},
                           tint_color)) {
        request_seek(current_frame_ui + 5);
        is_playing_.exchange(false);
    }

    // Keyboard shortcuts
    if (video_hovered_ && !timestamp_input_focused_ && !context_->is_cmd_input_focused) {
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            is_playing_.exchange(!is_playing);
            time_accumulator_ = 0.0;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false)) {
            request_seek(current_frame_ui - 1);
            is_playing_.exchange(false);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, false)) {
            request_seek(current_frame_ui - 10);
            is_playing_.exchange(false);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false)) {
            request_seek(current_frame_ui + 1);
            is_playing_.exchange(false);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, false)) {
            request_seek(current_frame_ui + 10);
            is_playing_.exchange(false);
        }
    }
}

void view_page::draw_open_video() {
    // Check if the file is ready
    {
        const std::scoped_lock<std::mutex> lock{video_path_mutex_};
        if (selected_video_) {
            stop_decoding_thread();
            try_cleanup_sokol_resources();
            video_path_     = selected_video_.value().first;
            selected_video_ = std::nullopt;
            video_loaded_   = true;
            start_decoding_thread();
        }
    }

    // File selection can happen at any point during playback
    const bool is_disabled_by_video = video_dialog_running_.load();
    if (is_disabled_by_video) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Open Video")) {
        video_dialog_running_ = true;
        video_loaded_         = false;
        dynamic_plotting_     = false;

        const auto  alive         = is_alive_;
        const auto& previous_file = video_path_;

        std::thread([this, alive, previous_file]() noexcept {
            try {
                const auto path = open_video_file(previous_file);
                if (*alive) {
                    const std::scoped_lock<std::mutex> lock{video_path_mutex_};
                    selected_video_       = path;
                    video_dialog_running_ = false;
                }
            } catch (const std::exception& e) {
                video_dialog_running_ = false;
                LOG_CRITICAL("Fatal error occurred while opening video dialog: {}", e.what());
            } catch (...) {
                video_dialog_running_ = false;
                LOG_CRITICAL("Unknown fatal error occurred");
            }
        }).detach();
    }
    if (is_disabled_by_video) { ImGui::EndDisabled(); }
}

void view_page::draw_rhs() {
    if (ImGui::BeginChild("##data")) {
        const auto cleanup_data{gsl::finally(ImGui::EndChild)};
        HEADER({
            draw_open_text();
            ImGui::SameLine();
            draw_sync_video_buttons();
            ImGui::Separator();
        });

        if (ImGui::BeginCombo("##DataView", data_type_string(data_show_))) {
            const auto cleanup_combo{gsl::finally(ImGui::EndCombo)};
            if (ImGui::Selectable("All Data", data_show_ == data_view_t::ALL)) {
                data_show_ = data_view_t::ALL;
            }
            if (ImGui::Selectable("RPM", data_show_ == data_view_t::RPMDATA)) {
                data_show_ = data_view_t::RPMDATA;
            }
            if (ImGui::Selectable("Shock", data_show_ == data_view_t::SHOCKDATA)) {
                data_show_ = data_view_t::SHOCKDATA;
            }
        }

        const auto data = context_->backend->pack_data();

        const auto sync_lt    = context_->backend->data.get_sync_lt();
        const auto plot_title = sync_lt
                                    ? fmt::format("Data View from {}", sync_lt.value().to_string())
                                    : "No Synced Time";

        view_page::dynamic_plot_loop();
        if (ImPlot::BeginPlot(plot_title.c_str(), {-1, -1})) {
            const auto cleanup_plot{gsl::finally(ImPlot::EndPlot)};
            pages::utils::plot_if_non_empty<double>("Wheel Speed",
                                                    data.time_minutes_normalized,
                                                    data.series.at("W"),
                                                    data_show_ == data_view_t::ALL ||
                                                        data_show_ == data_view_t::RPMDATA,
                                                    plot_percent_);
            pages::utils::plot_if_non_empty<double>("Engine Speed",
                                                    data.time_minutes_normalized,
                                                    data.series.at("E"),
                                                    data_show_ == data_view_t::ALL ||
                                                        data_show_ == data_view_t::RPMDATA,
                                                    plot_percent_);

            pages::utils::plot_if_non_empty<double>("Front Right Shock Travel",
                                                    data.time_minutes_normalized,
                                                    data.series.at("FR"),
                                                    data_show_ == data_view_t::ALL ||
                                                        data_show_ == data_view_t::SHOCKDATA,
                                                    plot_percent_);
            pages::utils::plot_if_non_empty<double>("Front Left Shock Travel",
                                                    data.time_minutes_normalized,
                                                    data.series.at("FL"),
                                                    data_show_ == data_view_t::ALL ||
                                                        data_show_ == data_view_t::SHOCKDATA,
                                                    plot_percent_);
            pages::utils::plot_if_non_empty<double>("Rear Right Shock Travel",
                                                    data.time_minutes_normalized,
                                                    data.series.at("RR"),
                                                    data_show_ == data_view_t::ALL ||
                                                        data_show_ == data_view_t::SHOCKDATA,
                                                    plot_percent_);
            pages::utils::plot_if_non_empty<double>("Rear Left Shock Travel",
                                                    data.time_minutes_normalized,
                                                    data.series.at("RL"),
                                                    data_show_ == data_view_t::ALL ||
                                                        data_show_ == data_view_t::SHOCKDATA,
                                                    plot_percent_);
        }
    }
}

void view_page::draw_open_text() {
    // Check if the file is ready
    {
        const std::scoped_lock<std::mutex> lock{txt_path_mutex_};
        if (selected_txt_) {
            txt_path_     = selected_txt_.value();
            selected_txt_ = std::nullopt;
            txt_loaded_   = true;
            load_data();
        }
    }

    // File selection can happen at any point during playback
    const bool is_disabled_by_txt = txt_dialog_running_.load();
    if (is_disabled_by_txt) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Open Text File")) {
        txt_dialog_running_ = true;
        txt_loaded_         = false;
        dynamic_plotting_   = false;

        const auto alive         = is_alive_;
        const auto previous_path = txt_path_;

        std::thread([this, alive, previous_path]() noexcept {
            try {
                const auto path = open_text_file(previous_path);
                if (*alive) {
                    const std::scoped_lock<std::mutex> lock{txt_path_mutex_};
                    selected_txt_                 = path;
                    txt_dialog_running_           = false;
                    context_->backend->is_logging = false;
                }
            } catch (const std::exception& e) {
                txt_dialog_running_ = false;
                LOG_CRITICAL("Fatal error occurred while opening text file dialog: {}", e.what());
            } catch (...) {
                txt_dialog_running_ = false;
                LOG_CRITICAL("Unknown fatal error occurred");
            }
        }).detach();
    }
    if (is_disabled_by_txt) { ImGui::EndDisabled(); }
}

void view_page::draw_sync_video_buttons() {
    if (ImGui::Button("Sync Data/Video")) {
        data_and_time_sync_ = false;
        dynamic_plotting_   = false;

        load_data();
        if (!txt_loaded_ || !video_loaded_) {
            LOG_ERROR("Could not sync data with video:");
            LOG_ERROR("  Text Loaded: {}", txt_loaded_);
            LOG_ERROR("  Video Loaded: {}", video_loaded_);
            return;
        }

        video_creation_ts_ = local_time::from_string(creation_metadata_text_buf_);
        if (!video_creation_ts_) {
            LOG_ERROR("Could not parse provided timestamp, or it was not provided.");
            return;
        }
        creation_metadata_text_buf_ = {};

        std::optional<size_t> sync_time_pos;
        {
            const std::scoped_lock<std::mutex> lock{context_->backend->data_mutex};
            sync_time_pos = sync_data_video(context_->backend->data.get_time_no_normal());
        }

        if (!sync_time_pos) {
            LOG_ERROR("Could not get trim position from data/video");
            return;
        }

        // This is probably mega unsafe...
        delete_extra(sync_time_pos.value());
        request_seek(0);
    }

    ImGui::SameLine();
    pages::utils::draw_input_box("##extra_view", creation_metadata_text_buf_, "HH:MM:SS", 120.0F);
    timestamp_input_focused_ = ImGui::IsItemFocused();
    ImGui::SameLine();
    if (ImGui::Checkbox("Dynamic Plotting", &dynamic_plotting_)) {
        view_page::dynamic_plot_start();
    }
}

view_page::selected_video_t view_page::open_video_file(const std::string& previous_file) {
    const char* const filters[] = {"*.mp4", "*.mov"};
    const char*       path      = tinyfd_openFileDialog(
        "Select a video file", previous_file.c_str(), std::size(filters), filters, nullptr, 0);
    if (path == nullptr) {
        LOG_WARN("No file selected");
        return std::nullopt;
    }

    const std::string real_path{path};
    LOG_INFO("Selected file: {}", real_path);

    auto dt = date_time::from_video_metadata(real_path);
    if (dt) {
        LOG_INFO("Selected video with creation timestamp: {}", dt.value().to_string());
    } else {
        LOG_WARN("Could not detect datetime metadata from selected video.");
    }

    return std::pair{path, dt};
}

view_page::selected_txt_file_ view_page::open_text_file(const std::string& previous_file) {
    const char* const filters[] = {"*.txt"};
    const char*       path      = tinyfd_openFileDialog(
        "Select a text file", previous_file.c_str(), std::size(filters), filters, nullptr, 0);
    if (path == nullptr) {
        LOG_WARN("No file selected");
        return std::nullopt;
    }

    LOG_INFO("Selected file: {}", path);
    return path;
}

void view_page::load_data() {
    std::ifstream file{txt_path_};
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file: {}", txt_path_);
        return;
    }

    const std::scoped_lock<std::mutex> lock{context_->backend->data_mutex};
    context_->backend->data.clear();
    std::string ident, value;
    while (file >> ident >> value) { context_->backend->data.write_data(ident, value); }
}

void view_page::request_seek(int frame_index) {
    // Prevent stepping out of bounds, though it is recoverable
    const auto clamped_frame = std::clamp(frame_index, 0, total_frames_);

    {
        const std::scoped_lock<std::mutex> lock{frame_mutex_};
        frame_queue_.clear();
    }

    seek_target_        = clamped_frame;
    force_update_frame_ = true;
    current_frame_ui    = clamped_frame;
    queue_cv_.notify_one();
}

void view_page::start_decoding_thread() {
    thread_running_ = true;
    is_playing_     = true;

    decode_thread_ = std::thread([this]() {
        cv::VideoCapture cap{video_path_};
        if (!cap.isOpened()) {
            LOG_ERROR("Failed to open video");
            thread_running_ = false;
            return;
        }

        video_fps_                 = cap.get(cv::CAP_PROP_FPS);
        frame_duration_            = 1.0 / video_fps_;
        total_frames_              = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
        video_length_minutes_      = (static_cast<double>(total_frames_) / video_fps_) / 60.0;
        const auto video_length_lt = local_time::from_minutes(video_length_minutes_);
        if (!video_length_lt) {
            LOG_ERROR("Video length could not be determined");
            thread_running_ = false;
            return;
        }
        video_length_formatted_ = video_length_lt.value().to_string(false);

        cv::Mat raw_frame, rgba_frame;
        while (thread_running_) {
            // Keep the buffer from becoming too large
            {
                std::unique_lock<std::mutex> lock{frame_mutex_};
                queue_cv_.wait(lock, [this] {
                    return frame_queue_.size() < MAX_QUEUE_SIZE || !thread_running_;
                });

                if (!thread_running_) { break; }
            }

            const int seek_req    = seek_target_.exchange(-1);
            bool      just_sought = false;

            if (seek_req != -1) {
                cap.set(cv::CAP_PROP_POS_FRAMES, seek_req);
                const std::scoped_lock<std::mutex> lock{frame_mutex_};
                frame_queue_.clear();
                just_sought = true;
            }

            // Don't decode if seeking or paused to prevent jitters
            if (!is_playing_ && !just_sought) {
                std::this_thread::sleep_for(5ms);
                continue;
            }

            auto current_frame_index = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));
            if (cap.read(raw_frame)) {
                cv::cvtColor(raw_frame, rgba_frame, cv::COLOR_BGR2RGBA);
                const cv::Mat frame_copy = rgba_frame.clone();

                const std::scoped_lock<std::mutex> lock{frame_mutex_};
                frame_queue_.emplace_back(frame_copy, current_frame_index);
            } else {
                if (is_looping_) {
                    cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                } else {
                    std::this_thread::sleep_for(10ms);
                }
            }
        }
    });
}

void view_page::stop_decoding_thread() {
    thread_running_ = false;
    queue_cv_.notify_all();
    if (decode_thread_.joinable()) { decode_thread_.join(); }

    const std::scoped_lock<std::mutex> lock{frame_mutex_};
    frame_queue_.clear();
}

void view_page::update_texture(bool is_timer_tick) {
    cv::Mat frame_to_upload;
    bool    frame_ready = false;
    {
        const std::scoped_lock<std::mutex> lock{frame_mutex_};
        const bool                         should_consume = is_timer_tick || force_update_frame_;

        if (should_consume && !frame_queue_.empty()) {
            const auto p     = frame_queue_.front();
            frame_to_upload  = p.first;
            current_frame_ui = p.second;

            frame_queue_.pop_front();
            frame_ready = true;
            queue_cv_.notify_one();
            force_update_frame_.exchange(false);
        }
    }

    if (frame_ready && !frame_to_upload.empty()) {
        // Initialize Texture if resolution changed or first run
        if (texture_width_ != frame_to_upload.cols || texture_height_ != frame_to_upload.rows) {
            try_cleanup_sokol_resources();

            texture_width_  = frame_to_upload.cols;
            texture_height_ = frame_to_upload.rows;

            sg_image_desc desc       = {};
            desc.width               = texture_width_;
            desc.height              = texture_height_;
            desc.pixel_format        = SG_PIXELFORMAT_RGBA8;
            desc.usage.stream_update = true;
            desc.num_mipmaps         = 1;
            video_texture_           = sg_make_image(&desc);
            assert(video_texture_.id != SG_INVALID_ID);

            sg_view_desc view_desc  = {};
            view_desc.texture.image = video_texture_;
            video_view_             = sg_make_view(&view_desc);
            video_texture_id_       = simgui_imtextureid(video_view_);
        }

        // Perform the actual upload
        sg_image_data data      = {};
        data.mip_levels[0].ptr  = frame_to_upload.data;
        data.mip_levels[0].size = frame_to_upload.total() * frame_to_upload.elemSize();
        sg_update_image(video_texture_, &data);
    }
};

void view_page::try_cleanup_sokol_resources() {
    if (video_view_.id != SG_INVALID_ID) {
        sg_destroy_view(video_view_);
        video_view_.id = SG_INVALID_ID;
    }

    if (video_texture_.id != SG_INVALID_ID) {
        sg_destroy_image(video_texture_);
        video_texture_.id = SG_INVALID_ID;
        video_texture_id_ = 0;
    }

    texture_width_  = 0;
    texture_height_ = 0;
}

std::optional<size_t> view_page::sync_data_video(const std::vector<uint64_t>& micros_times) {
    if (!video_creation_ts_) { return std::nullopt; }

    data_and_time_sync_               = true;
    const auto     creation_timestamp = video_creation_ts_.value();
    const uint64_t micros_to_sync     = creation_timestamp.micros_since_midnight();
    const auto     it                 = std::ranges::lower_bound(micros_times, micros_to_sync);
    if (it != micros_times.end()) { return std::distance(micros_times.begin(), it); }

    return std::nullopt;
}

void view_page::delete_extra(size_t erase_pos) {
    const std::scoped_lock<std::mutex> lock{context_->backend->data_mutex};
    std::vector<double>* const         data[] = {
        &context_->backend->data.time_,
        &context_->backend->data.series.at("W"),
        &context_->backend->data.series.at("E"),
        &context_->backend->data.series.at("FR"),
        &context_->backend->data.series.at("FL"),
        &context_->backend->data.series.at("RR"),
        &context_->backend->data.series.at("RL"),
    };

    for (const auto& datum : data) {
        if (datum->size() < erase_pos) { return; }
    }

    for (const auto& datum : data) {
        datum->erase(datum->begin(), datum->begin() + static_cast<ptrdiff_t>(erase_pos));
    }
}

void view_page::dynamic_plot_start() {
    if (dynamic_plotting_) {
        const std::scoped_lock<std::mutex> lock{context_->backend->data_mutex};
        const auto&                        time_vec = context_->backend->data.time_;
        if (time_vec.empty()) { return; }
        const auto begin_time_min = context_->backend->data.time_[0];

        const double target_end_time = begin_time_min + video_length_minutes_;
        const auto   end_it          = std::ranges::lower_bound(time_vec, target_end_time);

        const size_t end_idx = std::distance(time_vec.begin(), end_it);
        const size_t end_idy = std::distance(end_it, time_vec.end());
        data_from_end_       = static_cast<double>(end_idy);
        data_count_          = static_cast<double>(end_idx);
        points_per_          = static_cast<double>(end_idx) / total_frames_;
    }
}

void view_page::dynamic_plot_loop() {
    if (dynamic_plotting_) {
        plot_percent_ = static_cast<size_t>(std::max(
            data_count_ - (points_per_ * current_frame_ui) + data_from_end_, data_from_end_));
        return;
    }
    plot_percent_ = 0;
}

} // namespace mbr::pages
