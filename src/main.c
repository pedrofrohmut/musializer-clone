#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "raylib.h"

#include "../include/logger.h"

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

/* Frame global_frames[4410 * 2] = {0}; // 44100 is the number of audio samples for second */
/* size_t global_frames_count = 0; */

float pi = 3.141592;
#define N 256
float in[N];
float complex out[N];
float max_amp;

void fft(float in[], size_t step, float complex out[], size_t n)
{
    assert(n > 0);

    if (n == 1) {
        out[0] = in[0];
        return;
    }

    fft(in,        step * 2, out,         n / 2);
    fft(in + step, step * 2, out + n / 2, n / 2);

    for (size_t k = 0; k < n / 2; k++) {
        float t  = (float) k / n;
        float complex v = cexp(-2 * I * pi * t) * out[k + n / 2];
        float complex e = out[k];
        out[k]         = e + v;
        out[k + n / 2] = e - v;
    }
}

float amp(float x)
{
    float a = fabsf(crealf(x));
    float b = fabsf(cimagf(x));
    if (a < b) return b;
    return a;
}

// Ring Buffer - Makes the effect of the wave going to the left
void capture_frames_callback(void * data, unsigned int frames_count)
{
    if (frames_count > N) frames_count = N;

    Frame * frames = data;

    for (size_t i = 0; i < frames_count; i++) {
        in[i] = frames[i].left;
    }

    fft(in, 1, out, N);

    max_amp = 0.0f;
    for (size_t i = 0; i < frames_count; i++) {
        float a = amp(out[i]);
        if (max_amp < a) max_amp = a;
    }
}

// Call DrawTextEx with some values already set to simplify the call
void draw_text(const Font font, const char * text, const Vector2 pos)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TXT_SPACING, TEXT_COLOR);
}

int main(void)
{
    InitWindow(W_WIDTH, W_HEIGHT, "Musializer");
    InitAudioDevice();
    SetTargetFPS(30); // FPS set to 60 to stop flikering the sound, 30 for testing

    // Load font
    const int font_size = 28;
    const int glyph_count = 250;
    const Font noto_font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    const char * music_path = "./resources/mp3/no-way.mp3";

    // Check music exists
    const int music_exists = access(music_path, F_OK);
    if (music_exists == -1) {
        log_error("Music file not found in the path provided\n");
        return 1;
    }

    const Music music = LoadMusicStream(music_path);

    float curr_volume = 0.5f;
    SetMusicVolume(music, curr_volume);

    AttachAudioStreamProcessor(music.stream, capture_frames_callback);

    const int music_time_length = GetMusicTimeLength(music);

    PlayMusicStream(music); // For testing can remove later

    const float half_height = (float) W_HEIGHT / 2;

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

        char str_vol_temp[50];
        snprintf(str_vol_temp, sizeof(str_vol_temp), "(%.0f) %d / %d",
                curr_volume * 100, (int) GetMusicTimePlayed(music), music_time_length);

        BeginDrawing(); //###########################################################################
        ClearBackground(BACKGROUND_COLOR);

        const float cell_width = (float) W_WIDTH / N;

        for (size_t i = 0; i < N; i++) {
            float t = amp(out[i]) / max_amp;
            /* float t = amp(out[i]); */

            const int pos_x = i * cell_width;
            const int pos_y = half_height - (half_height * t);
            const int rect_width = cell_width;
            const int rect_height = half_height * t;

            DrawRectangle(pos_x, pos_y, rect_width, rect_height, RECT_COLOR);
        }

        draw_text(noto_font, str_vol_temp, (Vector2) { 10, W_HEIGHT - 35 }); // Draw Temp and Volume to the corner
        EndDrawing(); // ###########################################################################
    }

    UnloadMusicStream(music);
    UnloadFont(noto_font);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
