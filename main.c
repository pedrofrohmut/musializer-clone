#include "raylib.h"

void add_text(const char * text, const int x, const int y);
void add_text_with_clr(const char * text, const int x, const int y, const Color clr);

// Constants for fonts
Font global_font;
/* float global_font_size = 20.0f; */
float global_txt_spacing = 2.0f;
Color global_txt_tint = BLACK;

void draw_text(const char * text, const Vector2 pos)
{
    DrawTextEx(global_font, text, pos, (float) global_font.baseSize, global_txt_spacing, global_txt_tint);
    /* DrawTextEx(fontTtf, msg, (Vector2){ 20.0f, 100.0f }, (float)fontTtf.baseSize, 2, LIME); */
}

int main(void)
{
    const int height = 600;
    const int width = 800;

    InitWindow(width, height, "Musializer");
    InitAudioDevice();

    SetTargetFPS(60);

    Sound no_way_music = LoadSound("resources/mp3/no-way.mp3");

    // TTF font : Font data and atlas are generated directly from TTF
    // NOTE: We define a font base size of 28 pixels tall and up-to 250 characters
    global_font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", 28, 0, 250);

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
