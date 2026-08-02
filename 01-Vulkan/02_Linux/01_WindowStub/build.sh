clear

# Compiling the source file to create an object file
gcc -c -o xwindow.o xwindow.c

# Linking the object file to create the executable
gcc -o xwindow xwindow.o -lX11
