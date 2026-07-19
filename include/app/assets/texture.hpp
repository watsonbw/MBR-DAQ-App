#pragma once

#include <gsl/span>
#include <imgui.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_imgui.h>
#include <stb_image.h>
#include <stdx/types.hh>
#include <stdx/utility.hh>

namespace mbr::assets {

class button_texture {
  public:
    button_texture(gsl::span<const unsigned char> data);
    ~button_texture();
    MAKE_MOVE_ONLY(button_texture);

    [[nodiscard]] ImTextureID get_id() const { return im_tex_id_; }
    [[nodiscard]] bool        is_valid() const { return im_tex_id_ != 0; }

  private:
    sg_image    image_{SG_INVALID_ID};
    sg_view     view_{SG_INVALID_ID};
    ImTextureID im_tex_id_{0};
};

template <bool Managed = false> struct icon_texture {
    icon_texture(gsl::span<const unsigned char> data)
        : pixels{stbi_load_from_memory(
              data.data(), static_cast<i32>(data.size()), &width, &height, &comp, 4)} {
        size = static_cast<usize>(width * height) * comp;
    }
    ~icon_texture() { release(); };
    MAKE_MOVE_ONLY(icon_texture);

    // This is a noop if the texture is managed
    void release() {
        if constexpr (!Managed) {
            stbi_image_free(pixels);
            pixels = nullptr;
        }
    }

    i32            width;
    i32            height;
    i32            comp;
    unsigned char* pixels;
    usize          size;
};

} // namespace mbr::assets
