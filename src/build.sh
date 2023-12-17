home=$(pwd)


cd $home/server
clang server.cpp -o $home/build/server.o

cd $home/client
clang client.cpp -o $home/build/client.o
