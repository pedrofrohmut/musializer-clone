#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "raylib.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

const int W_WIDTH = 800; // 800
const int W_HEIGHT = 600; // 600

const Color BACKGROUND_COLOR = (Color) { 0x23, 0x23, 0x23, 255 }; // Dark Gray
const Color TEXT_COLOR = (Color) { 0xCC, 0xCC, 0xCC, 255 }; // Light Gray
const float TXT_SPACING = 2.0f;

// Play text coords
const Vector2 play_txt_pos = (Vector2) { (float) W_WIDTH / 2 - 175, (float) W_HEIGHT / 2 - 90 };

// Pause text coords
const Vector2 pause_txt_pos = (Vector2) { (float) W_WIDTH / 2 - 190, (float) W_HEIGHT / 2 - 30};

// Volume value
const Vector2 vol_val_txt_pos = (Vector2) { (float) W_WIDTH / 2 - 46, (float) W_HEIGHT / 2  + 30 };

// Time Played
const Vector2 time_txt_pos = (Vector2) { (float) W_WIDTH / 2 - 86, (float) W_HEIGHT / 2  + 90 };

// Close text coords
const Vector2 close_txt_pos = (Vector2) { (float) W_WIDTH / 2 - 86, (float) W_HEIGHT / 2  + 150 };

void draw_text(const Font font, const char * text, const Vector2 pos)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TXT_SPACING, TEXT_COLOR);
}

int64_t global_frames[1024] = {0};
size_t global_frames_count = 0;

void callback(void * bufferData, unsigned int frames)
{
    // Sefety check to prevent overflows
    if (frames > ARRAY_LEN(global_frames)) {
        frames = ARRAY_LEN(global_frames);
    }
    memcpy(global_frames, bufferData, frames * sizeof(int64_t));
    global_frames_count = frames;

    /* for (size_t i = 0; i < frames; i++) { */
    /*     printf("X: %ld\t", global_frames[i]); */
    /* } */

    /* printf("Frames: %d, global_frames: %zu\n", frames, global_frames_count); */
}

int main(void)
{

    InitWindow(W_WIDTH, W_HEIGHT, "Musializer");
    InitAudioDevice();

    // FPS set to 60 to stop flikering the sound
    SetTargetFPS(30);

    const char * music_path = "./resources/mp3/no-way.mp3";

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

    printf("Music frame count: %d\n", music.frameCount);

    const int font_size = 28;
    const int glyph_count = 250;
    const Font noto_font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    const int music_time_length = GetMusicTimeLength(music);

    PlayMusicStream(music); // For testing can remove later

    while (! WindowShouldClose()) {
        UpdateMusicStream(music);

        if (IsKeyPressed(KEY_ENTER)) {
            StopMusicStream(music);
            PlayMusicStream(music);
        }

        if (IsKeyPressed(KEY_SPACE)) {
            if (IsMusicStreamPlaying(music))
                PauseMusicStream(music);
            else
                ResumeMusicStream(music);
        }

        if (IsKeyPressed(KEY_Q)) break;

        if (IsKeyPressed(KEY_MINUS) && curr_volume > 0.0f) {
            curr_volume -= 0.05f;
            SetMusicVolume(music, curr_volume);
        }

        if (IsKeyPressed(KEY_EQUAL) && curr_volume < 1.0f) {
            curr_volume += 0.05f;
            SetMusicVolume(music, curr_volume);
        }

        /* char str_volume[20]; */
        /* snprintf(str_volume, sizeof(str_volume), "Volume: %.0f", curr_volume * 100); */
        /* char str_time_played[20]; */
        /* snprintf(str_time_played, sizeof(str_time_played), "time: %d / %d", (int) GetMusicTimePlayed(music), music_time_length); */

        char str_desc[50];
        snprintf(str_desc, sizeof(str_desc), "(%.0f) %d / %d", curr_volume * 100, (int) GetMusicTimePlayed(music), music_time_length);

        BeginDrawing(); //###########################################################################

        ClearBackground(BACKGROUND_COLOR);

        const float cell_width = (float) W_WIDTH / global_frames_count;

        int min_pos = INT32_MAX;
        int min_neg = INT32_MAX;

        int guess_num = 7;

        // Draw stuff loop (GlobalFramesCount 388)
        for (size_t i = 0; i < global_frames_count; ++i) {
            // Pick only one of the channels (1/2)
            const int32_t sample = * (int32_t *) &global_frames[i];

            /* printf("X:%d\t", sample); */

            const int pos_x = i * cell_width;
            int pos_y;
            const int rec_w = cell_width;
            int rec_h;
            const Color rec_clr = { 0x66, 0xFF, 0x33, 0x77 };


            if (sample >= 0) {
                // gets a value between 0..1 and the multiply by window size
                rec_h = ((float) sample / INT32_MAX) * W_HEIGHT;

                if (rec_h < min_pos) min_pos = rec_h; // Update MinPos

                // Remove the height from min down so it only shows the difference
                // instead of bars almost the same with a minor difference at the top
                rec_h = (rec_h - min_pos) * guess_num;

                // Start from middle - height ends at the middle
                pos_y = W_HEIGHT / 2 - rec_h;
            } else {
                // gets a value between 0..1 and the multiply by window size
                rec_h = ((float) sample / INT32_MIN) * W_HEIGHT;

                if (rec_h < min_neg) min_neg = rec_h; // Update MinNeg

                // Remove the height from min down so it only shows the difference
                // instead of bars almost the same with a minor difference at the top
                rec_h = (rec_h - min_neg) * guess_num;

                // Starts in the middle and ends at middle + height
                pos_y = W_HEIGHT / 2;
            }
            DrawRectangle(pos_x, pos_y, rec_w, rec_h, rec_clr);

            /* if (sample >= 0) { */
            /*     float t = (float) sample / INT32_MAX; */
            /*     rec_h = (float) W_HEIGHT / 2 * t; */
            /*     pos_y = W_HEIGHT / 2 - rec_h; */
            /*     DrawRectangle(pos_x, pos_y, rec_w, rec_h, rec_clr); */
            /* } else { */
            /*     float t = (float) sample / INT32_MIN; */
            /*     rec_h = (float) W_HEIGHT / 2 * t; */
            /*     pos_y = W_HEIGHT / 2; */
            /*     DrawRectangle(pos_x, pos_y, rec_w, rec_h, rec_clr); */
            /* } */
        }

        // Ref to align text. Uncomment to use
        /* draw_text(noto_font, "|", (Vector2) { (float) W_WIDTH / 2, (float) W_HEIGHT / 2 - 40 }); */

        /* draw_text(noto_font, "Press 'Enter' to play/restart sound.", play_txt_pos); */
        /* draw_text(noto_font, "Press 'Space' to pause/resume sound.", pause_txt_pos); */

        // Ref to align text. Uncomment to use
        /* draw_text(noto_font, "|", (Vector2) { (float) W_WIDTH / 2, (float) W_HEIGHT / 2 + 40 }); */

        /* draw_text(noto_font, str_volume, vol_val_txt_pos); */
        /* draw_text(noto_font, str_time_played, time_txt_pos); */
        /* draw_text(noto_font, "Press 'q' to exit", close_txt_pos); */

        draw_text(noto_font, str_desc, (Vector2) { 10, W_HEIGHT - 35 });


        EndDrawing(); // ###########################################################################
    }

    UnloadMusicStream(music);

    UnloadFont(noto_font);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
