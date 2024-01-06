#include "raylib.h"
#include <stdio.h>

const int W_HEIGHT = 600;
const int W_WIDTH = 800;

const Color BACKGROUND_COLOR = (Color) { 35, 35, 35, 255 }; // Dark Gray
const Color TEXT_COLOR = (Color) { 220, 220, 220, 255 }; // Light Gray
const float TXT_SPACING = 2.0f;

// Play text coords
const Vector2 play_txt_pos = (Vector2) { (float) W_WIDTH / 2 - 175, (float) W_HEIGHT / 2 - 90 };

// Pause text coords
const Vector2 pause_txt_pos = (Vector2) { (float) W_WIDTH / 2 - 190, (float) W_HEIGHT / 2 - 30};

// Volume value
const Vector2 vol_val_txt_pos = (Vector2) { (float) W_WIDTH / 2 - 46, (float) W_HEIGHT / 2  + 30 };

// Close text coords
const Vector2 close_txt_pos = (Vector2) { (float) W_WIDTH / 2 - 86, (float) W_HEIGHT / 2  + 90 };

void draw_text(const Font font, const char * text, const Vector2 pos)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TXT_SPACING, TEXT_COLOR);
}

int main(void)
{

    InitWindow(W_WIDTH, W_HEIGHT, "Musializer");
    InitAudioDevice();

    // FPS set to 60 to stop flikering the sound
    SetTargetFPS(60);

    const Music music = LoadMusicStream("./resources/mp3/no-way.mp3");
    float curr_volume = 0.5f;

    SetMusicVolume(music, curr_volume);

    const int font_size = 28;
    const int glyph_count = 250;
    const Font noto_font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    while (! WindowShouldClose()) {
        UpdateMusicStream(music);

        if (IsKeyPressed(KEY_ENTER)) {
            StopMusicStream(music);
            PlayMusicStream(music);
        }

        if (IsKeyPressed(KEY_SPACE)) {
            if (IsMusicStreamPlaying(music))
                PauseMusicStream(music);
            else
                ResumeMusicStream(music);
        }

        if (IsKeyPressed(KEY_Q)) break;

        if (IsKeyPressed(KEY_MINUS) && curr_volume > 0.0f) {
            curr_volume -= 0.05f;
            SetMusicVolume(music, curr_volume);
        }

        if (IsKeyPressed(KEY_EQUAL) && curr_volume < 1.0f) {
            curr_volume += 0.05f;
            SetMusicVolume(music, curr_volume);
        }

        char str_volume[20];
        snprintf(str_volume, sizeof(str_volume), "Volume: %d",  (int) (curr_volume * 100));

        BeginDrawing();

        ClearBackground(BACKGROUND_COLOR);

        /* draw_text(noto_font, "|", (Vector2) { (float) W_WIDTH / 2, (float) W_HEIGHT / 2 - 40 }); // Ref to align text */

        draw_text(noto_font, "Press 'Enter' to play/restart sound.", play_txt_pos);
        draw_text(noto_font, "Press 'Space' to pause/resume sound.", pause_txt_pos);

        /* draw_text(noto_font, "|", (Vector2) { (float) W_WIDTH / 2, (float) W_HEIGHT / 2 + 40 }); // Ref to align text */

        draw_text(noto_font, str_volume, vol_val_txt_pos);
        draw_text(noto_font, "Press 'q' to exit", close_txt_pos);


        EndDrawing();
    }

    UnloadMusicStream(music);

    UnloadFont(noto_font);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
