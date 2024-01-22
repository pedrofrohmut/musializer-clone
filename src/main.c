#include <raylib.h>
#include <stddef.h>

#include "app.h"

// Handy length function
#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])

int main(int argc, char **argv)
{
    (void) argc;

    // Initialization ------------------------------------------------------------------------------
    const char * file_path = argv[1];
    AppState * state = app_init(file_path);

    // Main game loop ------------------------------------------------------------------------------
    while (! WindowShouldClose()) {
        if (IsKeyPressed(KEY_Q)) break; // Quit/Close

        // Update ----------------------------------------------------------------------------------
        app_update(state);

        // Draw
        BeginDrawing(); //--------------------------------------------------------------------------

        app_draw(state);

        EndDrawing(); //----------------------------------------------------------------------------
    }

    // De-Initialization ---------------------------------------------------------------------------
    app_unload_and_close(state);

    return 0;
}
