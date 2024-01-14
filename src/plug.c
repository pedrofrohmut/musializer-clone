#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#include "logger.h"
#include "plug.h"
#include "raylib.h"

float * global_input;
float complex * global_output;

// Call DrawTextEx with some values already set to simplify the call
void draw_text(const Font font, const char * text, const Vector2 pos)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TEXT_SPACING, TEXT_COLOR);
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

    printf("IN:");
    for (size_t i = 0 ; i < 5; i++) {
        printf(" %3.2f", global_input[i]);
    }
    printf("\n");
}

// Refreshes the references lost on the hot-reloading
void plug_reload(Plug * plug)
{
    log_debug("Plug reload");
    global_input = plug->in;
    global_output = plug->out;
}
