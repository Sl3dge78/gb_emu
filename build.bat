
@clang src/main.c -g -o bin/gb.exe -I "../_include" -L "../_lib" -lSDL2.lib -lSDL2main.lib -lShell32.lib -lSDL2_ttf.lib -D_CRT_SECURE_NO_WARNINGS -Xlinker -SUBSYSTEM:WINDOWS
