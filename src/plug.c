#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

const float TEXT_SPACING = 2.0f;

static PlugState * global_plug;  // global reference to the plug (required for the audio_callback)

// Refreshes the references lost on the hot-reloading
void plug_reload(PlugState * plug)
{
    global_plug = plug;
}

// Must use global_input because you cannot pass the Plug and keep a valid callback signature
void plug_audio_callback(void * data, unsigned int framesc)
{
    if (data == NULL || framesc == 0) {
        fprintf(stderr, "No data in this iteration");
        return;
    }

    const size_t N = global_plug->n;
    size_t size = global_plug->in_size;
    float * in = global_plug->in1;

    if (N < framesc) {
        for (size_t i = 0; i < N; i++) {
            float left = ((float *) data)[i * 2];
            in[i] = left;
        }
        return;
    }

    if (framesc > N - size) {
        size = N - framesc;
        for (size_t i = 0; i < size; i++) in[i] = in[i + framesc];
    }

    for (size_t i = 0; i < framesc; i++) {
        float left = ((float *) data)[i * 2];
        in[size + i] = left;
    }
    size += framesc;
}

void plug_load_music(PlugState * plug, const char * file_path)
{
    if (!file_path || strcmp(file_path, "") == 0) {
        fprintf(stderr, "File path not provided");
        exit(1);
    }

    // Load music
    plug->music = LoadMusicStream(file_path);

    // Check music
    if (! IsMusicReady(plug->music)) {
        log_error("Could not load music for path: %s", file_path);
        return;
    }

    // Setup
    plug->music_len = GetMusicTimeLength(plug->music);
    plug->curr_time = GetMusicTimePlayed(plug->music);
    SetMusicVolume(plug->music, plug->curr_volume);
}

// Set UI string based on playing state
void plug_set_playing(PlugState * plug, bool is_playing)
{
    if (is_playing) {
        strncpy(plug->str.play_state, "Playing...", sizeof(plug->str.play_state));
    } else {
        strncpy(plug->str.play_state, "Not Playing", sizeof(plug->str.play_state));
    }
}

// Call DrawTextEx with some values already set to simplify the call (default color)
void draw_text(const Font font, const char * text, const Vector2 pos)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TEXT_SPACING, DARKGRAY);
}

// Call DrawTextEx with some values already set to simplify the call
void draw_text_color(const Font font, const char * text, const Vector2 pos, Color color)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TEXT_SPACING, color);
}

void check_key_pressed(PlugState * plug)
{
    if (IsKeyPressed(KEY_ENTER)) { // Start / Restart
        StopMusicStream(plug->music);
        PlayMusicStream(plug->music);
    }

    if (IsKeyPressed(KEY_SPACE)) { // Pause / Resume
        if (IsMusicStreamPlaying(plug->music)) {
            PauseMusicStream(plug->music);
            plug_set_playing(plug, false);
        } else {
            ResumeMusicStream(plug->music);
            plug_set_playing(plug, true);
        }
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

float calc_amp(float complex x)
{
    /* float a = fabsf(crealf(x)); */
    /* float b = fabsf(cimagf(x)); */
    /* if (a < b) return b; */
    /* return a; */
    float a = crealf(x);
    float b = cimagf(x);
    return logf((a * a) + (b * b));
}

/* float calc_normalizer(PlugState * plug, float freq, float next_freq, float max_amp) */
/* { */
/*     // Calculate the avarage for this amplitudes interval */
/*     float amps_acc = 0.0f; */
/*     size_t q = (size_t) freq; */
/*     while (q < plug->n && q < (size_t) next_freq) { */
/*         amps_acc += calc_amp(plug->out[q]); */
/*         q++; */
/*     } */
/*     float amps_avarage = amps_acc / ((size_t)next_freq - (size_t)freq + 1); */
/*  */
/*     // Value between 0..1 to set rect height */
/*     float normalizer  = amps_avarage / max_amp; */
/*  */
/*     return normalizer; */
/* } */

void fft_skip_frames(PlugState * plug)
{
    // Make the animation slower (skiping the change of plug->out)
    const unsigned int skip_step = 3; // only fft on every (n + 1) frames
    if (plug->skip_c >= skip_step) {
        plug->skip_c = 0;
        fft(plug->in2, 1, plug->out, plug->n);
    } else {
        plug->skip_c++;
    }
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
    if (IsMusicReady(plug->music)) {
        UpdateMusicStream(plug->music);

        check_key_pressed(plug);

        // Only generate str on time change
        float updated_music_time = GetMusicTimePlayed(plug->music);
        if (updated_music_time - plug->curr_time > 0.2) { // update gap 200ms
            // Makes the text for: (<volume>) <current_time> / <total_time>
            snprintf(plug->str.vol_time, sizeof(plug->str.vol_time), "(%2.0f) %3.0f / %3.0f",
                     plug->curr_volume * 100, updated_music_time, plug->music_len);
            plug->curr_time = updated_music_time;
        }
    }

    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();
        if (droppedFiles.count > 0) {
            if (IsMusicReady(plug->music)) {
                StopMusicStream(plug->music);
                DetachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
                UnloadMusicStream(plug->music);
            }

            const char * file_path = droppedFiles.paths[0];
            plug_load_music(plug, file_path);

            if (! IsMusicReady(plug->music)) {
                log_error("Could not be loaded music by file drop: %s\n", file_path);
                plug->error.has_error = true;
                strncpy(plug->error.message, "Dropped file is not valid",
                        sizeof(plug->error.message));
            } else {
                plug->error.has_error = false;
                AttachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
                PlayMusicStream(plug->music);
                plug_set_playing(plug, true);
            }
        }
        UnloadDroppedFiles(droppedFiles);
    }
}

void draw_ui(PlugState * plug)
{
    // TODO: come up with some king of strut to be used here and that can be store
    // not just with the text be with the Vector2 too
    if (IsMusicReady(plug->music)) {
        // App title
        draw_text(plug->font, plug->str.title, (Vector2) { 15, plug->height - 40 });
        // Is it playing or not feedback
        draw_text(plug->font, plug->str.play_state, (Vector2) {
                plug->width - 320, plug->height - 40 });
        // Temp and Volume to the corner
        const float extra_padding = plug->curr_time >= 100 ? 5 : 0;
        draw_text(plug->font, plug->str.vol_time, (Vector2) {
                plug->width - 168 - extra_padding, plug->height - 40 });
#ifdef DEV_ENV // String to print N on dev mode
        draw_text(plug->font, plug->str.n_str, (Vector2) { 165, plug->height - 40 });
#endif
    } else if (plug->error.has_error) {
        const Vector2 dimensions = MeasureTextEx(plug->font, plug->error.message,
                (float) plug->font.baseSize, TEXT_SPACING);
        draw_text_color(plug->font, plug->error.message, (Vector2) {
                (plug->width / 2) - (dimensions.x / 2), (plug->height / 2) - (dimensions.y / 2) }, RED);
    } else {
        const Vector2 dimensions = MeasureTextEx(plug->font, plug->str.drag_txt,
                (float) plug->font.baseSize, TEXT_SPACING);
        draw_text(plug->font, plug->str.drag_txt, (Vector2) {
                (plug->width / 2) - (dimensions.x / 2), (plug->height / 2) - (dimensions.y / 2) });
    }
}

void draw_rectangles(PlugState * plug)
{
    const size_t N = plug->n;
    const float STEP = plug->step;
    const float LOWF = 1.0f;

    for (size_t i = 0; i < N; i++) {
        float t = (float) i / (N - 1);
        float hann = 0.5 - 0.5 * cosf(2 * PI * t); // Windowing function (remove phantom frequencies)
        plug->in2[i] = plug->in1[i] * hann;
    }

    /* fft(plug->in2, 1, plug->out, N); */
    fft_skip_frames(plug);

    float max_amp = 0.0f;
    for (size_t i = 0; i < N; i++) {
        float amp = calc_amp(plug->out[i]);
        if (max_amp < amp) max_amp = amp;
    }

    const float cell_width = plug->width / plug->m;
    const float half_height = plug->height / 2;
    const float bottom = plug->height - 50;

    size_t i = 0;
    for (float f = LOWF; (size_t) f < N/2; f = ceilf(f * STEP)) {
        float next_f = ceilf(f * STEP);
        float max = 0;
        for (size_t q = (size_t) f; q < N/2 && q < (size_t) next_f; q++) {
            float amp = calc_amp(plug->out[q]);
            if (amp > max) max = amp;
        }
        // Draw Rectangles -----------------------------------------------------------------------------
        float norm = max / max_amp; // Normalizer
        DrawRectangle(i * cell_width, bottom - half_height*norm, cell_width, half_height*norm, GREEN);
        i++;
    }
}

void main_draw(PlugState * plug)
{
    ClearBackground(BACKGROUND_COLOR);

    draw_ui(plug);

    // TODO: Draw -> check if can skip calculations on skip frames on
    if (IsMusicReady(plug->music)) draw_rectangles(plug);
}

void plug_update(PlugState * plug)
{
    main_update(plug);

    BeginDrawing(); //##############################################################################
    main_draw(plug);
    EndDrawing(); // ###############################################################################
}
