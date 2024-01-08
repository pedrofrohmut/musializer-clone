#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "raylib.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

#define C_DARK_GRAY    CLITERAL(Color){ 0x23, 0x23, 0x23, 0xFF } // Dark  Gray
#define C_LIGHT_GRAY   CLITERAL(Color){ 0xCC, 0xCC, 0xCC, 0xFF } // Light Gray
#define C_MATRIX_GREEN CLITERAL(Color){ 0x66, 0xFF, 0x33, 0xFF } // Matrix Green
#define C_MATRIX_PURPLE CLITERAL(Color){ 0x99, 0x00, 0xCC, 0xFF } // Matrix Purple

const int W_WIDTH = 800; // 800
const int W_HEIGHT = 600; // 600

const Color BACKGROUND_COLOR = C_DARK_GRAY;
const Color TEXT_COLOR = C_LIGHT_GRAY;
const Color RECT_COLOR = C_MATRIX_GREEN;
const Color RECT_NEG_COLOR = C_MATRIX_PURPLE;

const float TXT_SPACING = 2.0f;

typedef struct {
    float left;
    float right;
} Frame;

Frame global_frames[512] = {0};
size_t global_frames_count = 0;

void callback(void * buffer_data, unsigned int frames)
{
    float * b = (float *) buffer_data;
    size_t count = 0;
    for (unsigned int i = 0; i < frames * 2; i = i + 2)
    {
        float left = b[i]; float right = b[i+1];
        global_frames[count] = (Frame) { left, right };
        count++;
    }
    global_frames_count = count;
}

void draw_text(const Font font, const char * text, const Vector2 pos)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TXT_SPACING, TEXT_COLOR);
}

int main(void)
{

    InitWindow(W_WIDTH, W_HEIGHT, "Musializer");
    InitAudioDevice();
    SetTargetFPS(30); // FPS set to 60 to stop flikering the sound

    // Load font
    const int font_size = 28;
    const int glyph_count = 250;
    const Font noto_font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    const char * music_path = "./resources/mp3/no-way.mp3";

    // Check music exists
    const int music_exists = access(music_path, F_OK);
    if (music_exists == -1) {
        printf("ERROR: Music file not found in the path provided\n");
        return 1;
    }
    assert(music_exists != -1);

    const Music music = LoadMusicStream(music_path);

    printf("music.frameCount: %d\n", music.frameCount);
    printf("music.stream.sampleRate: %d\n", music.stream.sampleRate);
    printf("music.stream.sampleSize: %d\n", music.stream.sampleSize);
    printf("music.stream.channels: %d\n", music.stream.channels);
    assert(music.stream.sampleSize == 32);
    assert(music.stream.channels == 2);

    float curr_volume = 0.5f;
    SetMusicVolume(music, curr_volume);

    AttachAudioStreamProcessor(music.stream, callback);

    const int music_time_length = GetMusicTimeLength(music);

    PlayMusicStream(music); // For testing can remove later

    const int middle = W_HEIGHT / 2;
    while (! WindowShouldClose()) {
        UpdateMusicStream(music);

        if (IsKeyPressed(KEY_ENTER)) { // Start / Restart
            StopMusicStream(music);
            PlayMusicStream(music);
        }

        if (IsKeyPressed(KEY_SPACE)) { // Pause / Resume
            if (IsMusicStreamPlaying(music))
                PauseMusicStream(music);
            else
                ResumeMusicStream(music);
        }

        if (IsKeyPressed(KEY_Q)) break; // Close

        if (IsKeyPressed(KEY_MINUS) && curr_volume > 0.0f) { // Decrease Volume
            curr_volume -= 0.05f;
            SetMusicVolume(music, curr_volume);
        }

        if (IsKeyPressed(KEY_EQUAL) && curr_volume < 1.0f) { // Increase Volume
            curr_volume += 0.05f;
            SetMusicVolume(music, curr_volume);
        }

        char str_desc[50];
        snprintf(str_desc, sizeof(str_desc), "(%.0f) %d / %d", curr_volume * 100, (int) GetMusicTimePlayed(music), music_time_length);

        BeginDrawing(); //###########################################################################
        ClearBackground(BACKGROUND_COLOR);

        const float cell_width = (float) W_WIDTH / global_frames_count;
        for (size_t i = 0; i < global_frames_count; i++) {
            float sampleL = global_frames[i].left;

            if (sampleL == 0) continue; // Skip on zero

            const int pos_x = i * cell_width;
            const int rec_w = 1;
            const int rec_h = sampleL * W_HEIGHT;

            if (sampleL > 0) {
                const int left_y = middle - rec_h;
                DrawRectangle(pos_x, left_y, rec_w, rec_h, RECT_COLOR);
            } else {
                const int left_y = middle;
                DrawRectangle(pos_x, left_y, rec_w, abs(rec_h), RECT_NEG_COLOR);
            }
        }

        draw_text(noto_font, str_desc, (Vector2) { 10, W_HEIGHT - 35 }); // Draw Temp and Volume to the corner
        EndDrawing(); // ###########################################################################
    }

    UnloadMusicStream(music);
    UnloadFont(noto_font);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
