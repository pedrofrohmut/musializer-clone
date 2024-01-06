#include "raylib.h"

const Color BACKGROUND_COLOR = (Color) { 35, 35, 35, 255 }; // Dark Gray
const Color TEXT_COLOR = (Color) { 220, 220, 220, 255 }; // Light Gray
const float TXT_SPACING = 2.0f;

void draw_text(const Font font, const char * text, const Vector2 pos)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TXT_SPACING, TEXT_COLOR);
}

int main(void)
{
    const int height = 600;
    const int width = 800;

    InitWindow(width, height, "Musializer");
    InitAudioDevice();

    // FPS set to 60 to stop flikering the sound
    SetTargetFPS(60);

    Music music = LoadMusicStream("./resources/mp3/no-way.mp3");

    const int font_size = 28;
    const int glyph_count = 250;
    Font noto_font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    // Play text coords
    const Vector2 play_txt_pos = (Vector2) {
        (float) GetScreenWidth() / 2 - 175,
        (float) GetScreenHeight() / 2 - 80
    };

    // Close text coords
    const Vector2 close_pos = (Vector2) {
        (float) GetScreenWidth() / 2 - 86,
        (float) GetScreenHeight() / 2  + 40
    };

    while (! WindowShouldClose()) {
        UpdateMusicStream(music);

        if (IsKeyPressed(KEY_ENTER)) {
            StopMusicStream(music);
            PlayMusicStream(music);
        }

        if (IsKeyPressed(KEY_Q)) break;

        BeginDrawing();

        ClearBackground(BACKGROUND_COLOR);

        /* draw_text(noto_font, "|", (Vector2) { (float) width / 2, (float) height / 2 - 50 }); */

        draw_text(noto_font, "Press 'Enter' to play/restart sound.", play_txt_pos);
        draw_text(noto_font, "Press 'q' to exit", close_pos);

        /* draw_text(noto_font, "|", (Vector2) { (float) width / 2, (float) height / 2 + 50 }); */

        EndDrawing();
    }

    UnloadMusicStream(music);

    UnloadFont(noto_font);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
