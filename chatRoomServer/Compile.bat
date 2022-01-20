mkdir objectfiles
cd objectfiles

gcc -c ..\main.c ..\src\*.c -I..\include
gcc -o ..\server.exe *.o -lws2_32 -static

cd ..
