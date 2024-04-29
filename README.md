# cpp-linux-sockets-app

Client-server application based on Linux sockets and written in C++ programming language.

## Overview

### Server

The server is capable of connecting multiple clients and processing their requests.

`main_server.cc` is specialized in such way that it can only process 3 types of commands:

1. `connections` - return current number of connections to the server
2. `count <message>` - count letters in the message and return it in the table form
3. `send <id> <message>` - send a message to the client with ID `id` (unfortunately, it is impossible to determine the IDs of clients yet)

If you want to wtite down your own server - you can specialize `server.h` server by providing `ResponseProcessor` caller to the `Serve` method.

### Client

The client is capable of connecting to the server once and sending/receiving data with it.

`main_client.cc` is specialized to endlessly ask for command and send it to the server if it is correct. After each succesfull transaction to the server client checks for incoming messages from server or other clients.

## Installation

```shell
cmake -B build src
cmake --build build
```

## Launching

### Server

Server accepts only one command-line argument - **port** on which it will be serving.

```shell
./server 8888
```

### Client

Client accepts two command-line arguments: **address** and **port**. Instead of actual address *localhost* can be specified to connect to local instances of the server.

```shell
./client localhost 8888
```
