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

static PlugState * global_plug;   // global reference to the plug (required for the audio_callback)

// Refreshes the references lost on the hot-reloading
void plug_reload(PlugState * plug)
{
    global_plug = plug;
}

// Must use global_input because you cannot pass the Plug and keep a valid callback signature
void plug_audio_callback(void * data, unsigned int frames_count)
{
    if (data == NULL || frames_count == 0) {
        fprintf(stderr, "No data in this iteration");
        return;
    }
    assert(global_plug->n > frames_count);

    /* Frame * frames = (Frame *) data; */
    unsigned int channels = global_plug->music.stream.channels;
    frame_t * frames = data;
    const bool new_data_fits = frames_count <= (global_plug->n - global_plug->in_size);

    // Ring buffer
    if (new_data_fits) {
        // Append the data no moving
        for (size_t i = 0; i < frames_count; i++) {
            size_t index = global_plug->in_size + i;
            global_plug->in[index] = frames[i][0]; // left channel
            global_plug->in_size++;
        }
    } else {
        global_plug->in_size = global_plug->n; // sets it as filled up

        // Shift left to fit new
        for (size_t i = 0; i < (global_plug->n - frames_count); i++) {
            global_plug->in[i] = global_plug->in[i + frames_count];
        }

        // Append new data at the space shifted for it
        for (size_t i = 0; i < frames_count; i++) {
            size_t index = global_plug->n - frames_count + i;
            global_plug->in[index] = frames[i][0]; // left channel
        }
    }
}

void plug_load_music(PlugState * plug, const char * file_path)
{
    // Unload if any
    if (IsMusicReady(plug->music)) {
        DetachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
        UnloadMusicStream(plug->music);
    }

    // Load music
    plug->music = LoadMusicStream(file_path);

    // Check music
    if (! IsMusicReady(plug->music)) {
        fprintf(stderr, "Music not loaded: %s", file_path);
        exit(1);
    }
    assert(plug->music.stream.channels == 2);

    // Setup
    plug->music_len = GetMusicTimeLength(plug->music);
    AttachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
}

// Call DrawTextEx with some values already set to simplify the call
void draw_text(const Font font, const char * text, const Vector2 pos)
{
    const float text_spacing = 2.0f;
    DrawTextEx(font, text, pos, (float) font.baseSize, text_spacing, DARKGRAY);
}

void check_key_pressed(PlugState * plug)
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

float calc_amp(float x)
{
    float a = fabsf(crealf(x));
    float b = fabsf(cimagf(x));
    if (a < b) return b;
    return a;
}

float calc_normalizer(PlugState * plug, float freq, float next_freq, float max_amp)
{
    // Calculate the avarage for this amplitudes interval
    float amps_acc = 0.0f;
    size_t q = (size_t) freq;
    while (q < plug->n && q < (size_t) next_freq) {
        amps_acc += calc_amp(plug->out[q]);
        q++;
    }
    float amps_avarage = amps_acc / ((size_t)next_freq - (size_t)freq + 1);

    // Value between 0..1 to set rect height
    float normalizer  = amps_avarage / max_amp;

    return normalizer;
}

void calc_fft(PlugState * plug)
{
#if 0
    // Make the animation slower (skiping the change of plug->out)
    const unsigned int skip_step = 1; // only fft on every (n + 1) frames
    if (plug->skip_c >= skip_step) {
        plug->skip_c = 0;
        fft(global_input, 1, plug->out, plug->n);
    } else {
        plug->skip_c++;
    }
#else
    fft(plug->in, 1, plug->out, plug->n);
#endif

}

float calc_max_amp(PlugState * plug)
{
    float max_amp = 0.0f;
    for (size_t i = 0; i < plug->n; i++) {
       float amp = calc_amp(plug->out[i]);
       if (max_amp < amp) max_amp = amp;
    }
    return max_amp;
}

void main_update(PlugState * plug)
{
    UpdateMusicStream(plug->music);

    check_key_pressed(plug);

    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();
        if (droppedFiles.count > 0) {
            StopMusicStream(plug->music);

            const char * file_path = droppedFiles.paths[0];
            plug_load_music(plug, file_path);

            PlayMusicStream(plug->music);
        }
        UnloadDroppedFiles(droppedFiles);
    }

    // Makes the text for: (<volume>) <current_time> / <total_time>
    char vol_time[50];
    snprintf(vol_time, sizeof(vol_time), "(%2.0f) %3.0f / %3.0f",
             plug->curr_volume * 100, GetMusicTimePlayed(plug->music), plug->music_len);
    plug->str.vol_time = vol_time;

    const char * playing = IsMusicStreamPlaying(plug->music) ? "Playing..." : "Not playing";
    plug->str.play_state = playing;
}

void main_draw(PlugState * plug)
{
    BeginDrawing(); //##############################################################################
    ClearBackground(BACKGROUND_COLOR);

    // UI Text -------------------------------------------------------------------------------------
    // App title
    draw_text(plug->font, plug->str.title, (Vector2) { 15, plug->height - 40 });
    // Is it playing or not feedback
    draw_text(plug->font, plug->str.play_state, (Vector2) { plug->width - 320, plug->height - 40 });
    // Temp and Volume to the corner
    draw_text(plug->font, plug->str.vol_time, (Vector2) { plug->width - 168, plug->height - 40 });
#ifdef DEV_ENV
    char n_str[50];
    snprintf(n_str, sizeof(n_str), "%zu", plug->n);
    draw_text(plug->font, n_str, (Vector2) { 165, plug->height - 40 });
#else
#endif
    // UI Text -------------------------------------------------------------------------------------

    // TODO: Draw -> check if can skip calculations on skip frames on
    // Draw Rectangles -----------------------------------------------------------------------------
    calc_fft(plug);
    const float max_amp = calc_max_amp(plug);
    const float half_height = plug->height / 2;
    const float cell_width = plug->width / plug->m;

    float freq = 20.0f;
    for (size_t i = 0; i < plug->m; i++) { // Iterate through m frequencies
        float next_freq = freq * plug->step;

        float normalizer  = calc_normalizer(plug, freq, next_freq, max_amp);

        // Draw Rect with calculated amps_avarage
        const float pos_x = i * cell_width;
        const float pos_y = half_height - (half_height * normalizer);
        const int rect_width = cell_width;
        const int rect_height = half_height * normalizer;
        DrawRectangle(pos_x, pos_y, rect_width, rect_height, SKYBLUE);

        freq = next_freq;
    }
    // Draw Rectangles -----------------------------------------------------------------------------

    EndDrawing(); // ###############################################################################
}

void plug_update(PlugState * plug)
{
    main_update(plug);
    main_draw(plug);
}
