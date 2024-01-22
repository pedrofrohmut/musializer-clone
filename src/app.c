#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include <stdio.h>
#include <raylib.h>
#include <assert.h>

#include "app.h"
#include "logger.h"

#define C_DARK_GRAY     CLITERAL(Color){ 0x23, 0x23, 0x23, 0xFF } // Dark  Gray
#define C_LIGHT_GRAY    CLITERAL(Color){ 0xCC, 0xCC, 0xCC, 0xFF } // Light Gray
#define C_MATRIX_GREEN  CLITERAL(Color){ 0x66, 0xFF, 0x33, 0xFF } // Matrix Green
#define C_MATRIX_PURPLE CLITERAL(Color){ 0x99, 0x00, 0xCC, 0xFF } // Matrix Purple

const Color BACKGROUND_COLOR = C_DARK_GRAY;
const Color RECT_COLOR       = C_MATRIX_GREEN;
const Color RECT_NEG_COLOR   = C_MATRIX_PURPLE;
const Color TEXT_COLOR       = C_LIGHT_GRAY;

const float TEXT_SPACING = 2.0f;

static AppState * global_state;

size_t calculate_m(const size_t n, const float step, const float low_freq)
{
    size_t m = 0; // M frequencies
    for (float f = low_freq; (size_t) f < n/2; f = ceilf(f * step)) m++;
    return m;
}

void load_music(AppState * state, const char * file_path)
{
    if (!file_path || strcmp(file_path, "") == 0) {
        fprintf(stderr, "File path not provided");
        exit(1);
    }

    // Load music
    state->music = LoadMusicStream(file_path);

    // Check music
    if (! IsMusicReady(state->music)) {
        log_error("Could not load music for path: %s", file_path);
        return;
    }

    // Setup
    state->music_len = GetMusicTimeLength(state->music);
    state->curr_time = GetMusicTimePlayed(state->music);
    SetMusicVolume(state->music, state->curr_volume);
}

// Must use global_state because you cannot pass the state and keep a valid callback signature
void audio_callback(void * data, unsigned int framesc)
{
    if (data == NULL || framesc == 0) {
        fprintf(stderr, "No data in this iteration");
        return;
    }

    const size_t N = global_state->n;
    size_t size = global_state->in_size;
    float * in = global_state->in1;

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

// Set UI string based on playing state
void set_playing(AppState * state, bool is_playing)
{
    if (is_playing) {
        strncpy(state->str.play_state, "Playing...", sizeof(state->str.play_state));
    } else {
        strncpy(state->str.play_state, "Not Playing", sizeof(state->str.play_state));
    }
}

AppState * app_init(const char * file_path)
{
    AppState * state = malloc(sizeof(AppState));

    // Window
    state->width = 800;
    state->height = 600;

    // Input/Output buffers
    state->n = (size_t) 2 << 9; // 2 << 13 == 16,384 (13 is default, min is 9)
    state->in1 = (float *) calloc(state->n, sizeof(float));
    state->in2 = (float *) calloc(state->n, sizeof(float));
    state->out = (float complex *) calloc(state->n, sizeof(float complex));
    state->in_size = 0;

    // Calculate frequencies
    state->lowf = 1.0f;
    state->step = 1.06f;
    state->m = calculate_m(state->n, state->step, state->lowf);

    // Skip frames
    state->skip_c = 0;

    // UI strings
    strncpy(state->str.title, "Musializer", sizeof(state->str.title));
    strncpy(state->str.drag_txt, "Drag & Drop Music Files Here", sizeof(state->str.drag_txt));
#ifdef DEV_ENV // String to print N on dev mode
    snprintf(state->str.n_str, sizeof(state->str.n_str), "%zu", state->n);
#endif

    // Error
    strncpy(state->error.message, "", sizeof(state->error.message));
    state->error.has_error = false;

    InitWindow(state->width, state->height, "Musializer");
    SetTargetFPS(60); // FPS set to 60 to stop flikering the sound, 30 for testing
    InitAudioDevice();

    if (file_path != NULL) {
        load_music(state, file_path);
        state->curr_volume = 0.0f;
        SetMusicVolume(state->music, state->curr_volume);
        AttachAudioStreamProcessor(state->music.stream, audio_callback);
        PlayMusicStream(state->music); // For testing can remove later
    }

    // Load font
    const int font_size = 30;
    const int glyph_count = 250;
    state->font = LoadFontEx("resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    // Check font
    if (! IsFontReady(state->font)) {
        fprintf(stderr, "Fonts not loaded");
        exit(1);
    }

    log_info("main app initialized");
    global_state = state;
    return state;
}

void app_unload_and_close(AppState * state)
{
    free(state->in1);
    free(state->in2);
    free(state->out);

    // Raylib
    if (IsMusicReady(state->music)) {
        DetachAudioStreamProcessor(state->music.stream, audio_callback);
        UnloadMusicStream(state->music);
    }
    UnloadFont(state->font);

    free(state);

    CloseAudioDevice();
    CloseWindow();

    log_info("main app unload and closed");
}

void check_key_pressed(AppState * state)
{
    if (IsKeyPressed(KEY_ENTER)) { // Start / Restart
        StopMusicStream(state->music);
        PlayMusicStream(state->music);
    }

    if (IsKeyPressed(KEY_SPACE)) { // Pause / Resume
        if (IsMusicStreamPlaying(state->music)) {
            PauseMusicStream(state->music);
        } else {
            ResumeMusicStream(state->music);
        }
    }

    if (IsKeyPressed(KEY_MINUS) && state->curr_volume > 0.0f) { // Decrease Volume
        state->curr_volume -= 0.05f;
        SetMusicVolume(state->music, state->curr_volume);
    }

    if (IsKeyPressed(KEY_EQUAL) && state->curr_volume < 1.0f) { // Increase Volume
        state->curr_volume += 0.05f;
        SetMusicVolume(state->music, state->curr_volume);
    }
}

void update_ui(AppState * state)
{
    if (! IsMusicStreamPlaying(state->music)) {
        set_playing(state, false);
        return;
    }

    float updated_music_time = GetMusicTimePlayed(state->music);

    // update gap 200ms (5x sec)
    if (updated_music_time - state->curr_time > 0.2) {
        state->curr_time = updated_music_time;
        // Makes the text for: (<volume>) <current_time> / <total_time>
        snprintf(state->str.vol_time, sizeof(state->str.vol_time), "(%2.0f) %3.0f / %3.0f",
                 state->curr_volume * 100, updated_music_time, state->music_len);
        set_playing(state, true);
    }
}

void check_file_dropped(AppState * state)
{
    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();
        if (droppedFiles.count > 0) {
            if (IsMusicReady(state->music)) {
                StopMusicStream(state->music);
                DetachAudioStreamProcessor(state->music.stream, audio_callback);
                UnloadMusicStream(state->music);
            }

            const char * file_path = droppedFiles.paths[0];
            load_music(state, file_path);

            if (! IsMusicReady(state->music)) {
                log_error("Could not be loaded music by file drop: %s\n", file_path);
                state->error.has_error = true;
                strncpy(state->error.message, "Dropped file is not valid",
                        sizeof(state->error.message));
            } else {
                state->error.has_error = false;
                AttachAudioStreamProcessor(state->music.stream, audio_callback);
                PlayMusicStream(state->music);
            }
        }
        UnloadDroppedFiles(droppedFiles);
    }
}

void app_update(AppState * state)
{
    if (IsMusicReady(state->music)) {
        UpdateMusicStream(state->music);
        check_key_pressed(state);
        update_ui(state);
    }
    check_file_dropped(state);
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

void draw_ui(AppState * state)
{
    // TODO: come up with some king of strut to be used here and that can be store
    // not just with the text be with the Vector2 too
    if (IsMusicReady(state->music)) {
        // App title
        draw_text(state->font, state->str.title, (Vector2) { 15, state->height - 40 });
        // Is it playing or not feedback
        draw_text(state->font, state->str.play_state, (Vector2) {
                state->width - 320, state->height - 40 });
        // Temp and Volume to the corner
        const float extra_padding = state->curr_time >= 100 ? 5 : 0;
        draw_text(state->font, state->str.vol_time, (Vector2) {
                state->width - 168 - extra_padding, state->height - 40 });
#ifdef DEV_ENV // String to print N on dev mode
        draw_text(state->font, state->str.n_str, (Vector2) { 165, state->height - 40 });
#endif
    } else if (state->error.has_error) {
        const Vector2 dimensions = MeasureTextEx(state->font, state->error.message,
                (float) state->font.baseSize, TEXT_SPACING);
        draw_text_color(state->font, state->error.message, (Vector2) {
                (state->width / 2) - (dimensions.x / 2),
                (state->height / 2) - (dimensions.y / 2) }, RED);
    } else {
        const Vector2 dimensions = MeasureTextEx(state->font, state->str.drag_txt,
                (float) state->font.baseSize, TEXT_SPACING);
        draw_text(state->font, state->str.drag_txt, (Vector2) {
                (state->width / 2) - (dimensions.x / 2),
                (state->height / 2) - (dimensions.y / 2) });
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

void fft_skip_frames(AppState * state)
{
    // Make the animation slower (skiping the change of state->out)
    const unsigned int skip_step = 3; // only fft on every (n + 1) frames

    const size_t N = state->n;
    if (state->skip_c >= skip_step) {
        for (size_t i = 0; i < N; i++) {
            float t = (float) i / (N - 1);
            // Windowing function (remove phantom frequencies)
            float hann = 0.5 - 0.5 * cosf(2 * PI * t);
            state->in2[i] = state->in1[i] * hann;
        }

        fft(state->in2, 1, state->out, state->n);

        state->skip_c = 0;
    } else {
        state->skip_c++;
    }
}

float calc_amp(float complex x)
{
    float a = crealf(x);
    float b = cimagf(x);
    return logf((a * a) + (b * b));
}

void draw_rectangles(AppState * state)
{
    const size_t N = state->n;
    const float STEP = state->step;
    const float LOWF = 1.0f;

    fft_skip_frames(state);

    float max_amp = 0.0f;
    for (size_t i = 0; i < N; i++) {
        float amp = calc_amp(state->out[i]);
        if (max_amp < amp) max_amp = amp;
    }

    const float cell_width = state->width / state->m;
    const float half_height = state->height / 2;
    const float bottom = state->height - 50;

    size_t i = 0;
    for (float f = LOWF; (size_t) f < N/2; f = ceilf(f * STEP)) {
        float next_f = ceilf(f * STEP);
        float max = 0;
        for (size_t q = (size_t) f; q < N/2 && q < (size_t) next_f; q++) {
            float amp = calc_amp(state->out[q]);
            if (amp > max) max = amp;
        }
        // Draw Rectangles -----------------------------------------------------------------------------
        float norm = max / max_amp; // Normalizer
        DrawRectangle(i * cell_width, bottom - half_height*norm, cell_width, half_height*norm, GREEN);
        i++;
    }
}
void app_draw(AppState * state)
{
    ClearBackground(BACKGROUND_COLOR);

    draw_ui(state);

    // TODO: Draw -> check if can skip calculations on skip frames on
    if (IsMusicReady(state->music)) draw_rectangles(state);
}
