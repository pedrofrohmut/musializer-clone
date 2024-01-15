#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#include "logger.h"
#include "plug.h"
#include "raylib.h"

#define C_DARK_GRAY     CLITERAL(Color){ 0x23, 0x23, 0x23, 0xFF } // Dark  Gray
#define C_LIGHT_GRAY    CLITERAL(Color){ 0xCC, 0xCC, 0xCC, 0xFF } // Light Gray
#define C_MATRIX_GREEN  CLITERAL(Color){ 0x66, 0xFF, 0x33, 0xFF } // Matrix Green
#define C_MATRIX_PURPLE CLITERAL(Color){ 0x99, 0x00, 0xCC, 0xFF } // Matrix Purple

const Color BACKGROUND_COLOR = C_DARK_GRAY;
const Color RECT_COLOR       = C_MATRIX_GREEN;
const Color RECT_NEG_COLOR   = C_MATRIX_PURPLE;
const Color TEXT_COLOR       = C_LIGHT_GRAY;

float * global_input;
float complex * global_output;

// Call DrawTextEx with some values already set to simplify the call
void draw_text(const Font font, const char * text, const Vector2 pos)
{
    const float text_spacing = 2.0f;
    DrawTextEx(font, text, pos, (float) font.baseSize, text_spacing, TEXT_COLOR);
}

void check_key_pressed(Plug * plug)
{
    if (IsKeyPressed(KEY_ENTER)) { // Start / Restart
        StopMusicStream(plug->music);
        PlayMusicStream(plug->music);
    }

    if (IsKeyPressed(KEY_SPACE)) { // Pause / Resume
        if (IsMusicStreamPlaying(plug->music))
            PauseMusicStream(plug->music);
        else
            ResumeMusicStream(plug->music);
    }

    if (IsKeyPressed(KEY_MINUS) && plug->curr_volume > 0.0f) { // Decrease Volume
        plug->curr_volume -= 0.05f;
        SetMusicVolume(plug->music, plug->curr_volume);
    }

    if (IsKeyPressed(KEY_EQUAL) && plug->curr_volume < 1.0f) { // Increase Volume
        plug->curr_volume += 0.05f;
        SetMusicVolume(plug->music, plug->curr_volume);
    }
}

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
        float complex v = cexp(-2 * I * PI * t) * out[k + n / 2];
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

void plug_update(Plug * plug)
{
    UpdateMusicStream(plug->music);

    check_key_pressed(plug);

    // Makes the text for: (<volume>) <current_time> / <total_time>
    char str_vol_temp[50];
    snprintf(str_vol_temp, sizeof(str_vol_temp), "(%2.0f) %3.0f / %3.0f",
             plug->curr_volume * 100, GetMusicTimePlayed(plug->music), plug->music_len);

    BeginDrawing(); //##############################################################################
    ClearBackground(BACKGROUND_COLOR);

    // UI Text -------------------------------------------------------------------------------------
    // App title
    draw_text(plug->font, "Musializer", (Vector2) { 30, 24 });
    // Is it playing or not feedback
    if (IsMusicStreamPlaying(plug->music)) {
        draw_text(plug->font, "Playing...", (Vector2) { 30, plug->height - 55 });
    } else {
        draw_text(plug->font, "Not Playing.", (Vector2) { 30, plug->height - 55 });
    }
    // Temp and Volume to the corner
    draw_text(plug->font, str_vol_temp, (Vector2) { plug->width - 195, plug->height - 55 });
    // UI Text -------------------------------------------------------------------------------------

    // Draw Rectangles -----------------------------------------------------------------------------
    fft(global_input, 1, global_output, N);

    float max_amp = 0.0f;
    for (size_t i = 0; i < N; i++) {
       float a = amp(global_output[i]);
       if (max_amp < a) max_amp = a;
    }

    const float cell_width = plug->width / N;
    const float half_height = plug->height / 2;

    for (size_t i = 0; i < N; i++) {
       float t = amp(global_output[i]) / max_amp;

       const int pos_x = i * cell_width;
       const int pos_y = half_height - (half_height * t);
       const int rect_width = cell_width;
       const int rect_height = half_height * t;

       DrawRectangle(pos_x, pos_y, rect_width, rect_height, LIME);
    }
    // Draw Rectangles -----------------------------------------------------------------------------

    EndDrawing(); // ###############################################################################
}

// Must use global_input and global_output because you cannot access the Plug
// and keep a valid callback signature
void plug_audio_callback(void * data, unsigned int frames_count)
{
    if (frames_count > N) frames_count = N;

    assert(data != NULL);

    /* log_debug("Plug audio. Frames: %d", frames_count); */

    Frame * frames = (Frame *) data;

    for (size_t i = 0; i < N; i++) {
        global_input[i] = frames[i].left;
    }

    /* printf("IN:"); */
    /* for (size_t i = 0 ; i < 5; i++) { */
    /*     printf(" %3.6f\t", global_input[i]); */
    /* } */
    /* printf("\n"); */
}

// Refreshes the references lost on the hot-reloading
void plug_reload(Plug * plug)
{
    log_debug("Plug reload");
    global_input = plug->in;
    global_output = plug->out;
}
