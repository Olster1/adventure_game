clear
clear
# g++ -std=c++11  -DDEBUG_BUILD=1 -o ../bin/main ./sdl_backend/platform_layer.cpp -Wno-deprecated-declarations -Wno-writable-strings -Wno-c++11-compat-deprecated-writable-strings -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -rpath /Library/Frameworks -F/Library/Frameworks -framework OpenGL -framework SDL2
g++ -std=c++11  -DDEBUG_BUILD=1 -o ../bin/main ./sdl_backend/platform_layer.cpp -Wno-deprecated-declarations -Wno-writable-strings -Wno-c++11-compat-deprecated-writable-strings -rpath /Library/Frameworks -F/Library/Frameworks -framework OpenGL -framework SDL2