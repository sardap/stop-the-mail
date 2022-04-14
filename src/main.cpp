#include <nds.h>
#include <stdio.h>

#include <defence_scene.hpp>
#include <enemies.hpp>
#include <generated/greenMap.hpp>

typedef struct {
    int x, y;  // x/y lcoation
    u16* gfx;  // oam GFX
    u8* gfx_frame;

    int state;  // sprite walk state
} Character;

#pragma GCC push_options
#pragma GCC optimize("O0")
void init() {
    // Initialize the top screen engine
    videoSetMode(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_SPRITE);
    oamInit(&oamMain, SpriteMapping_1D_128, false);

    // Initialize the bot screen engine
    videoSetModeSub(MODE_0_2D);
    vramSetBankD(VRAM_D_SUB_SPRITE);
    oamInit(&oamSub, SpriteMapping_1D_128, false);
}
#pragma GCC pop_options

int main(int argc, char** argv) {
    init();

    auto scene = sm::DefenceScene();
    sm::SceneArgs args = sm::DefenceSceneArgs{.level = greenMapLevel};
    scene.load(args);

    // iprintf("      Hello DS dev'rs\n");
    // iprintf("     \x1b[32mwww.devkitpro.org\n");
    // iprintf("   \x1b[32;1mwww.drunkencoders.com\x1b[39m");

    while (1) {
        swiWaitForVBlank();
        sm::globals::current_frame++;
        sm::globals::last_touch_position = sm::globals::touch_position;
        touchRead(&sm::globals::touch_position);

        scene.update();

        // print at using ansi escape sequence \x1b[line;columnH
        // iprintf("current frame:%ld\n", sm::globals::current_frame);
        // iprintf("\x1b[16;0HTouch x = %04X, %04X\n", touchXY.rawx,
        // touchXY.px); iprintf("Touch y = %04X, %04X\n", touchXY.rawy,
        // touchXY.py);
    }

    return 0;
}
