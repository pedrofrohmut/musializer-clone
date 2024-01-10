#include <stdint.h>

#include "../include/logger.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

typedef struct {
    float left;
    float right;
} Frame;

Frame global_frames[4410 * 2] = {0}; // 44100 is the number of audio samples for second
size_t global_frames_count = 0;

// Ring Buffer - Makes the effect of the wave going to the left
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
