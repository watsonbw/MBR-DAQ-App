#pragma once

#include <gsl/pointers>
#include <gsl/span>
#include <imgui.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_imgui.h>

namespace mbr::assets {

class button_texture {
  public:
    button_texture(gsl::span<const unsigned char> data);
    ~button_texture();

    button_texture(const button_texture&)                = delete;
    button_texture& operator=(const button_texture&)     = delete;
    button_texture(button_texture&&) noexcept            = default;
    button_texture& operator=(button_texture&&) noexcept = default;

    [[nodiscard]] ImTextureID get_id() const { return im_tex_id_; }
    [[nodiscard]] bool        is_valid() const { return im_tex_id_ != 0; }

  private:
    sg_image    image_{SG_INVALID_ID};
    sg_view     view_{SG_INVALID_ID};
    ImTextureID im_tex_id_{0};
};

struct icon_texture {
    icon_texture(gsl::span<const unsigned char> data);
    ~icon_texture() = default;

    icon_texture(const icon_texture&)                = delete;
    icon_texture& operator=(const icon_texture&)     = delete;
    icon_texture(icon_texture&&) noexcept            = default;
    icon_texture& operator=(icon_texture&&) noexcept = default;

    // Free the allocated stb image.
    //
    // Never call this if you passed the texture to sokol as an app icon.
    void release();

    int                           width;
    int                           height;
    int                           comp;
    gsl::not_null<unsigned char*> pixels;
    size_t                        size;
};

} // namespace mbr::assets
