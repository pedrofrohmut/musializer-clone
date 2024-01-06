#include "raylib.h"

// Constants for fonts
Font global_font;
float global_txt_spacing = 2.0f;
Color global_txt_tint = BLACK;

void draw_text(const char * text, const Vector2 pos)
{
    DrawTextEx(global_font, text, pos, (float) global_font.baseSize, global_txt_spacing,
            global_txt_tint);
}

int main(void)
{
    const int height = 600;
    const int width = 800;

    InitWindow(width, height, "Musializer");
    InitAudioDevice();

    SetTargetFPS(60);

    Sound no_way_music = LoadSound("resources/mp3/no-way.mp3");

    const int font_size = 28;
    const int glyph_count = 250;
    global_font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    // Play text coords
    const Vector2 play_txt_pos = (Vector2) {
        (float) GetScreenWidth() / 2 - 150,
        (float) GetScreenHeight() / 2 - 80
    };

    // Close text coords
    const Vector2 close_pos = (Vector2) {
        (float) GetScreenWidth() / 2 - 93,
        (float) GetScreenHeight() / 2  + 40
    };

    while (! WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ENTER)) PlaySound(no_way_music);
        if (IsKeyPressed(KEY_Q)) break;

        BeginDrawing();

        ClearBackground(WHITE);

        draw_text("Press 'Enter' to play sound.", play_txt_pos);
        draw_text("Press 'q' to exit", close_pos);

        EndDrawing();
    }


    UnloadSound(no_way_music);
    UnloadFont(global_font);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
