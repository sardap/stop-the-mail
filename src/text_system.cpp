#include <gfx/dsBasic.h>

#include <text_system.hpp>

namespace sm {

void init_text(TextInfo& text) {
    text.sheet_offset =
        oamAllocateGfx(text.oam, static_cast<SpriteSize>(dsBasicTilesLen),
                       SpriteColorFormat_256Color);
    dmaCopy(dsBasicTiles, text.sheet_offset, dsBasicTilesLen);

    for (size_t i = 0; i < text.oam_count; i++) {
        oamSetGfx(text.oam, i + text.oam_offset, SpriteSize_16x16,
                  SpriteColorFormat_256Color, text.sheet_offset);
        oamSetHidden(text.oam, i + text.oam_offset, true);
    }
}

void set_text(TextInfo& text_info, const TextGroup& text_group,
              const std::string& str, Position pos) {
    assert(str.length() < text_group.count);

    for (size_t i = 0; i < str.size(); i++) {
        size_t id = i + text_group.start;

        const auto& c = std::tolower(str[i]);
        if (c == ' ' || c == ':') {
            oamSetHidden(text_info.oam, id, true);
            continue;
        }

        int num_off = 0;
        if (c >= '0' && c <= '9') {
            num_off = c - '0';
        } else if (c >= 'a' && c <= 'z') {
            num_off = (c - 'a') + 10;
        }

        Fixed x = pos.x + Fixed(i) * 12;
        Fixed y = pos.y;

        u8* offset = (u8*)text_info.sheet_offset + (num_off * (16 * 16));
        oamSetGfx(text_info.oam, id, SpriteSize_16x16,
                  SpriteColorFormat_256Color, offset);
        oamSetXY(text_info.oam, id, static_cast<int>(x), static_cast<int>(y));
        oamSetHidden(text_info.oam, id, false);
    }
}

}  // namespace sm
