#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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

Frame global_frames[4410 * 2] = {0}; // 44100 is the number of audio samples for second
size_t global_frames_count = 0;

// Ring Buffer - Makes the effect of the wave going to the left
void capture_frames_callback(void * data, unsigned int frames_count);

// Call DrawTextEx with some values already set to simplify the call
void draw_text(const Font font, const char * text, const Vector2 pos);

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

    const int middle_y = W_HEIGHT / 2;
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

        const float cell_width = (float) W_WIDTH / global_frames_count;
        for (size_t i = 0; i < global_frames_count; i++) {
            float sample_left = global_frames[i].left; // Always between 0 and 1

            if (sample_left == 0) continue; // Skip on zero

            const int pos_x = i * cell_width;
            const int rect_width = 1;
            const int rect_height = sample_left * ((float) W_HEIGHT / 2); // Always betwwen 0 and Half Screen

            if (sample_left > 0) {
                const int pos_y = middle_y - rect_height;
                DrawRectangle(pos_x, pos_y, rect_width, rect_height, RECT_COLOR);
            } else {
                const int pos_y = middle_y;
                DrawRectangle(pos_x, pos_y, rect_width, abs(rect_height), RECT_NEG_COLOR);
            }
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

void capture_frames_callback(void * data, unsigned int frames_count)
{
    if (frames_count < 1) {  log_warn("0 zero for this call"); return; }// Skip on zero

    const size_t capacity = ARRAY_LEN(global_frames);

    if (frames_count > capacity) { log_debug("Over capacity code this!!!"); return; }

    // Has space -> just append data
    //   [ --------                        ] Data Before
    //   [ -------- ********               ] New Data Appended
    //
    // It is full -> Shift (g_capacity - frames_count) to left and then append data
    //   [ ------------------------------- ] Data Before
    //   [ ------- ####################### ] Data to shift
    //   [ ####################### ------- ] Data shifted left
    //   [ ####################### ******* ] New Data Appended

    const size_t free_space = capacity - global_frames_count;

    if (frames_count > free_space) {
        const size_t chunk_size = capacity - frames_count;
        global_frames_count = chunk_size;

        for (size_t i = 0; i < chunk_size; i++) {
            global_frames[i] = global_frames[i + frames_count];
        }
    }

    const unsigned int sample_count = frames_count * 2; // 1 frames == 2 samples in 2 channels

    // Append to global_frames (Step 2 - because it iterates samples here)
    for (size_t i = 0; i < sample_count; i += 2) {
        float left = ((float *) data)[i];
        float right = ((float *) data)[i + 1];
        global_frames[global_frames_count] = (Frame) { left, right };
        global_frames_count++;
    }
}

void draw_text(const Font font, const char * text, const Vector2 pos)
{
    DrawTextEx(font, text, pos, (float) font.baseSize, TXT_SPACING, TEXT_COLOR);
}
