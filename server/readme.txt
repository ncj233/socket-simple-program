服务端源代码为server.c

编译:
gcc -pthread server.c -o server
（如果gcc版本低，有可能要添加-std=c99）

程序需要在 linux 下运行