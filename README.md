# Energy Provider System
 system to track energy customers

 compile with:  g++ -std=c++23 -Wall -Wextra server.cpp database.cxx -I"asio-1.30.2\include" -o energy_server -o energy_server -lsqlite3 -lws2_32 -lmswsock -lpthread
