# Musializer Clone

Clone of tsoding musializer to make first steps on learning raylib with clang

Link: [Youtube playlist](https://www.youtube.com/playlist?list=PLpM-Dvs8t0Vak1rrE2NJn8XYEJ5M7-BqT)

## Links

### Technologies

- Raylib: [Github](https://github.com/raysan5/raylib)

- Raylib: [Cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)

- GLFW: [Docs](https://www.glfw.org/documentation.html)

### Algorithms and Math

- Euler's Formula: [Wikipedia](https://en.wikipedia.org/wiki/Euler%27s_formula)

- Frequency Table: [Equations](https://pages.mtu.edu/~suits/NoteFreqCalcs.html)

- Veritasium on FFT: [Youtube Video](https://www.youtube.com/watch?v=nmgFG7PUHfo)

- Understanding FFT: [PDF File](https://download.ni.com/evaluation/pxi/Understanding%20FFTs%20and%20Windowing.pdf)

- Better FFT audio visualization: [Article](https://dlbeer.co.nz/articles/fftvis.html)

- Hann Function: [Wikipedia](https://en.wikipedia.org/wiki/Hann_function)

- Numpy Hanning: [Reference Docs](https://numpy.org/doc/stable/reference/generated/numpy.hanning.html)

### Meta programming

- X Macro: [Wikipedia](https://en.wikipedia.org/wiki/X_macro)

### Miscellaneous

- Music used: [Youtube Channel](https://www.youtube.com/@nu11_ft)

- Flywaydb: [Database Migrations](https://flywaydb.org/)

- C gibberish ↔ English: [Cdecl Web Site](https://cdecl.org/)

## Versions:

### Implemented:

- v1 = Play music stream by hard coded file_path, simple UI and keybinds to restart, pause, resume, volume_up, volume_down and quit.
- v2 = Attach audio_processor to capture stream frames and draw rectangles on screen based on the frames.
- v2.1 = The animation is too fast. Made a ring buffer to slow it down.
- v3 = Implemented FFT and used it on the left channel samples to draw animation.
- v4 = Added support to hot reloading. Can make sepated functionalities at the file libplug.so and then with pressing r in the app, and rebuilding libplug.so, you can reload the version of libplug.so without having to restart the app.
- v4.1 = File_path not hard coded can be provided as argument
- extra: Made macros to auto generate code for hot reloading functions. (* extra branch not merged in main)
- v5 = Drag and Drop: using raylib to load files dropped.
- v5.1 = file_path argument is now optional
- v5.2 = Deals with files dropped that are not music

### TODOS:

- [ ] Remove hot reloading to reduce unecessary complexity
- [ ] Remove callback support for mono files (Raylib make everything 2 channels)
