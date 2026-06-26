#pragma once

#include <volk.h>
#include "../Core/Widget.hpp"

namespace HouseEngine::UI {

class Image : public Widget {
public:
    Image(VkDescriptorSet textureId = VK_NULL_HANDLE);
    virtual ~Image() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    void SetTexture(VkDescriptorSet textureId) { m_TextureId = textureId; }
    VkDescriptorSet GetTexture() const { return m_TextureId; }

private:
    VkDescriptorSet m_TextureId = VK_NULL_HANDLE;
};

} // namespace HouseEngine::UI
