#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#include "plug.h"
#include "raylib.h"

float global_input[N];
float complex global_output[N];

void plug_hello(void)
{
    printf("Hello from plug\n");
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

void callback(void * data, unsigned int frames_count)
{
    if (frames_count > N) frames_count = N;

    Frame * frames = data;

    for (size_t i = 0; i < frames_count; i++) {
        global_input[i] = frames[i].left;
    }
}

// Call DrawTextEx with some values already set to simplify the call
void draw_text(const Font font, const char * text, const Vector2 pos)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TXT_SPACING, TEXT_COLOR);
}

#if 1
// Initialize the state
void plug_init(Plug * plug, const char * file_path)
{
    const int music_exists = access(file_path, F_OK); // -1 for file not found
    if (music_exists == -1) {
        printf("Music file not found in the path provided\n");
    }
    assert(music_exists != -1);
    plug->music = LoadMusicStream(file_path);
    printf("music.frameCount = %u\n", plug->music.frameCount);
    printf("music.stream.sampleRate = %u\n", plug->music.stream.sampleRate);
    printf("music.stream.sampleSize = %u\n", plug->music.stream.sampleSize);
    printf("music.stream.channels = %u\n", plug->music.stream.channels);
    assert(plug->music.stream.sampleSize == 32);
    assert(plug->music.stream.channels == 2);

    plug->music_len = GetMusicTimeLength(plug->music);
    plug->curr_volume = 0.5f;
    SetMusicVolume(plug->music, plug->curr_volume);
    PlayMusicStream(plug->music); // For testing can remove later

    // Load font
    const int font_size = 28;
    const int glyph_count = 250;
    plug->font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);
}
#else
// Initialize the state
void plug_init(Plug * plug, const char * file_path)
{
    // Check music exists
    const int music_exists = access(file_path, F_OK);
    if (music_exists == -1) {
        printf("Music file not found in the path provided\n");
    }
    assert(music_exists != -1);

    plug->music = LoadMusicStream(file_path);

    printf("music.frameCount = %u\n", plug->music.frameCount);
    printf("music.stream.sampleRate = %u\n", plug->music.stream.sampleRate);
    printf("music.stream.sampleSize = %u\n", plug->music.stream.sampleSize);
    printf("music.stream.channels = %u\n", plug->music.stream.channels);
    assert(plug->music.stream.sampleSize == 32);
    assert(plug->music.stream.channels == 2);

    plug->music_len = GetMusicTimeLength(plug->music);

    AttachAudioStreamProcessor(plug->music.stream, callback);

    plug->curr_volume = 0.5f;
    SetMusicVolume(plug->music, plug->curr_volume);
    PlayMusicStream(plug->music); // For testing can remove later

    // Load font
    const int font_size = 28;
    const int glyph_count = 250;
    plug->font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);
}
#endif

#if 1
void plug_update(Plug * plug)
{
    UpdateMusicStream(plug->music);

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

    char str_vol_temp[50];
    snprintf(str_vol_temp, sizeof(str_vol_temp), "(%3.0f) %3.0f / %3.0f",
             plug->curr_volume * 100, GetMusicTimePlayed(plug->music), plug->music_len);

    // Get dimensions to be used on drawing
    const float width = GetRenderWidth();
    const float height = GetRenderHeight();
    const float half_height = (float) height / 2;

    BeginDrawing(); //###########################################################################
    ClearBackground(BACKGROUND_COLOR);

    // UI Text ----------------------------------------------------------------------------------
    // App title
    draw_text(plug->font, "Musializer", (Vector2) { 30, 24 });
    // Is it playing or not feedback
    if (IsMusicStreamPlaying(plug->music)) {
        draw_text(plug->font, "Playing...", (Vector2) { 30, height - 55 });
    } else {
        draw_text(plug->font, "Not Playing.", (Vector2) { 30, height - 55 });
    }
    // Temp and Volume to the corner
    draw_text(plug->font, str_vol_temp, (Vector2) { width - 185, height - 55 });
    // UI Text ----------------------------------------------------------------------------------

    EndDrawing(); // ###########################################################################
}
#else
// Run on every loop
void plug_update(Plug * plug)
{
    printf("BEGIN UPDATE\n");

    UpdateMusicStream(plug->music);

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

    printf("After keys\n");

    char str_vol_temp[50];
    snprintf(str_vol_temp, sizeof(str_vol_temp), "(%.0f) %d / %d",
            plug->curr_volume * 100, (int) GetMusicTimePlayed(plug->music), plug->music_len);

    const float width = GetRenderWidth();
    const float height = GetRenderHeight();
    const float half_height = (float) height / 2;

    printf("After vars\n");

    BeginDrawing(); //###########################################################################
    ClearBackground(BACKGROUND_COLOR);

    //draw_text(plug->font, "Hello, World!", (Vector2) { 30, 30 });

    DrawText("Hello, DrawText", 30, 30, 28, WHITE);

    /* fft(global_input, 1, global_output, N); */
    /* float max_amp = 0.0f; */
    /* for (size_t i = 0; i < N; i++) { */
    /*     float a = amp(global_output[i]); */
    /*     if (max_amp < a) max_amp = a; */
    /* } */
    /*  */
    /* const float cell_width = (float) width / N; */
    /*  */
    /* for (size_t i = 0; i < N; i++) { */
    /*     float t = amp(global_output[i]) / max_amp; */
    /*     #<{(| float t = amp(out[i]); |)}># */
    /*  */
    /*     const int pos_x = i * cell_width; */
    /*     const int pos_y = half_height - (half_height * t); */
    /*     const int rect_width = cell_width; */
    /*     const int rect_height = half_height * t; */
    /*  */
    /*     DrawRectangle(pos_x, pos_y, rect_width, rect_height, RED); */
    /* } */
    /*  */

    //draw_text(plug->font, str_vol_temp, (Vector2) { 10, height - 35 }); // Draw Temp and Volume to the corner
    EndDrawing(); // ###########################################################################
}
#endif
