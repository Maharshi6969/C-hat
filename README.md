# C-hat 

A multi-client chatroom written in pure **C** using raw TCP sockets.  
Powered by system calls, pthreads, coffee, and raw despair.  

##  Features
- Multiple clients can connect to one server
- Username-based identification
- Messages broadcast to all connected users
- 100% C (Winsock + pthreads)

##  How to Build & Run
```bash
gcc socket.c -o server -lws2_32 -lpthread
./server

gcc client.c -o client -lws2_32 -lpthread
./client
