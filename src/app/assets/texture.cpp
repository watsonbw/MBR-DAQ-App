#include <cassert>
#include <cstddef>

#include <stb_image.h>

#include "app/assets/texture.hpp"

namespace mbr::assets {

button_texture::button_texture(gsl::span<const unsigned char> data) {
    int   width, height, comp;
    auto* pixels = stbi_load_from_memory(
        data.data(), static_cast<int>(data.size()), &width, &height, &comp, 4);
    assert(pixels);

    sg_image_desc img_desc           = {};
    img_desc.width                   = width;
    img_desc.height                  = height;
    img_desc.pixel_format            = SG_PIXELFORMAT_RGBA8;
    img_desc.num_mipmaps             = 1;
    img_desc.data.mip_levels[0].ptr  = pixels;
    img_desc.data.mip_levels[0].size = static_cast<size_t>(width * height) * comp;

    image_ = sg_make_image(&img_desc);
    stbi_image_free(pixels);
    assert(image_.id != SG_INVALID_ID);

    sg_view_desc view_desc  = {};
    view_desc.texture.image = image_;

    view_ = sg_make_view(&view_desc);
    assert(view_.id != SG_INVALID_ID);
    im_tex_id_ = simgui_imtextureid(view_);
}

button_texture::~button_texture() {
    if (view_.id != SG_INVALID_ID) {
        sg_destroy_view(view_);
        view_.id = SG_INVALID_ID;
    }

    if (image_.id != SG_INVALID_ID) {
        sg_destroy_image(image_);
        image_.id  = SG_INVALID_ID;
        im_tex_id_ = 0;
    }
}

icon_texture::icon_texture(gsl::span<const unsigned char> data)
    : pixels{stbi_load_from_memory(
          data.data(), static_cast<int>(data.size()), &width, &height, &comp, 4)} {
    size = static_cast<size_t>(width * height) * comp;
}

void icon_texture::release() {
    stbi_image_free(pixels);
    width  = 0;
    height = 0;
    comp   = 0;
    size   = 0;
}

} // namespace mbr::assets
