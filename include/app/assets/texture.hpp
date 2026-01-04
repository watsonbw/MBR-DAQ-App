#pragma once

#include <imgui.h>

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_imgui.h>

class ButtonTexture {
  public:
    ButtonTexture() = delete;
    ButtonTexture(const unsigned char* data, size_t size);
    ~ButtonTexture();

    ImTextureID GetID() const { return m_ImTexID; }
    bool        IsValid() const { return m_ImTexID != 0; }

  private:
    sg_image    m_Image{SG_INVALID_ID};
    sg_view     m_View{SG_INVALID_ID};
    ImTextureID m_ImTexID{0};
};
