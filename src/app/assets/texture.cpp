#include <cassert>
#include <cstddef>

#include <stb_image.h>

#include "app/assets/texture.hpp"

ButtonTexture::ButtonTexture(const unsigned char* data, size_t size) {
    int   width, height, comp;
    auto* pixels = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &comp, 4);
    assert(pixels);

    sg_image_desc img_desc           = {};
    img_desc.width                   = width;
    img_desc.height                  = height;
    img_desc.pixel_format            = SG_PIXELFORMAT_RGBA8;
    img_desc.num_mipmaps             = 1;
    img_desc.data.mip_levels[0].ptr  = pixels;
    img_desc.data.mip_levels[0].size = static_cast<size_t>(width * height) * comp;

    m_Image = sg_make_image(&img_desc);
    stbi_image_free(pixels);
    assert(m_Image.id != SG_INVALID_ID);

    sg_view_desc view_desc  = {};
    view_desc.texture.image = m_Image;

    m_View = sg_make_view(&view_desc);
    assert(m_View.id != SG_INVALID_ID);
    m_ImTexID = simgui_imtextureid(m_View);
}

ButtonTexture::~ButtonTexture() {
    if (m_View.id != SG_INVALID_ID) {
        sg_destroy_view(m_View);
        m_View.id = SG_INVALID_ID;
    }

    if (m_Image.id != SG_INVALID_ID) {
        sg_destroy_image(m_Image);
        m_Image.id = SG_INVALID_ID;
        m_ImTexID  = 0;
    }
}

IconTexture::IconTexture(const unsigned char* data, size_t size) {
    int   width, height, comp;
    auto* pixels = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &comp, 4);
    assert(pixels);

    Width  = width;
    Height = height;
    Pixels = pixels;
    Size   = static_cast<size_t>(width * height) * comp;
}

void IconTexture::Free() {
    stbi_image_free(Pixels);
    *this = {};
}
