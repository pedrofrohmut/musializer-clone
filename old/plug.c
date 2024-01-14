#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

//#include "logger.h"
#include "plug.h"
#include "raylib.h"

float global_input[N];
float complex global_output[N];

void plug_hello(void) { printf("Hello from plug\n"); }

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

// Naming this is hard (no clue)
void audio_callback(void * buffer_data, unsigned int frames_count)
{
    if (buffer_data == NULL) { // Handle null pointer case
        fprintf(stderr, "Error: Null pointer passed to audio_callback\n");
        return;
    }

    if (frames_count > N) frames_count = N;

    Frame * frames = (Frame *) buffer_data;

    if (frames == NULL) { // Handle null pointer case
        fprintf(stderr, "Error: Null pointer passed as frames to audio_callback\n");
        return;
    }

    assert(frames != NULL);
    assert(frames_count > 0);

    for (size_t i = 0; i < frames_count; i++) {
        if (i < N) {
            global_input[i] = frames[i].left;
        } else { // Handle index out of bounds
            fprintf(stderr, "Error: Index out of bounds in audio_callback\n");
            break;
        }
    }

    /* for (size_t i = 0; i < frames_count; i++) { */
    /*     global_input[i] = frames[i].left; */
    /* } */
}

// Call DrawTextEx with some values already set to simplify the call
void draw_text(const Font font, const char * text, const Vector2 pos)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TXT_SPACING, GREEN);
}




// TODO: after refactor get this #ifs out of here
#if 1
// Initialize the state
void plug_init(Plug * plug, const char * file_path)
{
    // Load font
    const int font_size = 35;
    const int glyph_count = 250;
    plug->font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    if (!IsFontReady(plug->font)) {
        log_error("Fonts not loaded");
        exit(1);
    }

    if (IsMusicReady(plug->music)) {
        log_debug("Music is ready (Why?)");
        UnloadMusicStream(plug->music);
        //DetachAudioStreamProcessor(plug->music.stream, audio_callback);
    } else {
        log_debug("Music is NOT ready (normal)");
    }

    plug->music = LoadMusicStream(file_path);

    /* printf("Plug init called\n"); */
    /*  */
    /* const int music_exists = access(file_path, F_OK); // -1 for file not found */
    /* if (music_exists == -1) { */
    /*     printf("Music file not found in the path provided\n"); */
    /* } */
    /* assert(music_exists != -1); */
    /* printf("music.frameCount = %u\n", plug->music.frameCount); */
    /* printf("music.stream.sampleRate = %u\n", plug->music.stream.sampleRate); */
    /* printf("music.stream.sampleSize = %u\n", plug->music.stream.sampleSize); */
    /* printf("music.stream.channels = %u\n", plug->music.stream.channels); */
    /* assert(plug->music.stream.sampleSize == 32); */
    /* assert(plug->music.stream.channels == 2); */

    plug->music_len = GetMusicTimeLength(plug->music);
    plug->curr_volume = 0.0f;

    if (! IsAudioStreamReady(plug->music.stream)) {
        printf("\nERROR: audio stream not ready\n");
        exit(1);
    } else {
        log_info("Audio Stream is Ready.");
    }

    if (! IsAudioStreamProcessed(plug->music.stream)) {
        log_error("\nERROR: audio stream not processed\n");
        exit(1);
    } else {
        log_info("Audio Stream is Processed.");
    }

    if (! IsMusicReady(plug->music)) {
        log_error("\nERROR: music is not ready\n");
        exit(1);
    } else {
        log_info("Music is Ready.");
    }

    AttachAudioStreamProcessor(plug->music.stream, audio_callback);
    SetMusicVolume(plug->music, plug->curr_volume);
    PlayMusicStream(plug->music); // For testing can remove later
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

// TODO: after refactor get this #ifs out of here
#if 1
void plug_update(Plug * plug)
{
    /* assert(plug != NULL); */
    /* log_debug("UPDATE"); */
    assert(IsMusicReady(plug->music));

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
    snprintf(str_vol_temp, sizeof(str_vol_temp), "(%2.0f) %3.0f / %3.0f",
             plug->curr_volume * 100, GetMusicTimePlayed(plug->music), plug->music_len);

    // Get dimensions to be used on drawing
    const float width = GetRenderWidth();
    const float height = GetRenderHeight();
    const float half_height = (float) height / 2;

    BeginDrawing(); //##############################################################################
    ClearBackground(BACKGROUND_COLOR);

    // UI Text -------------------------------------------------------------------------------------
    // App title
    draw_text(plug->font, "Musializer", (Vector2) { 30, 24 });
    // Is it playing or not feedback
    if (IsMusicStreamPlaying(plug->music)) {
        draw_text(plug->font, "Playing...", (Vector2) { 30, height - 55 });
    } else {
        draw_text(plug->font, "Not Playing.", (Vector2) { 30, height - 55 });
    }
    // Temp and Volume to the corner
    draw_text(plug->font, str_vol_temp, (Vector2) { width - 195, height - 55 });
    // UI Text -------------------------------------------------------------------------------------

    // Draw Rectangle ------------------------------------------------------------------------------
    fft(global_input, 1, global_output, N);

    float max_amp = 0.0f;
    for (size_t i = 0; i < N; i++) {
       float a = amp(global_output[i]);
       if (max_amp < a) max_amp = a;
    }

    const float cell_width = (float) width / N;

    for (size_t i = 0; i < N; i++) {
       //if (global_output[i] == 0) continue; // skip on zero
       /* float t = amp(out[i]); */

       float t = amp(global_output[i]) / max_amp;

       const int pos_x = i * cell_width;
       const int pos_y = half_height - (half_height * t);
       const int rect_width = cell_width;
       const int rect_height = half_height * t;

       DrawRectangle(pos_x, pos_y, rect_width, rect_height, LIME);
    }
    // Draw Rectangle ------------------------------------------------------------------------------

    EndDrawing(); // ###############################################################################
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
