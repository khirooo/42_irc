# 42_irc

This project is about creating your own Internet Relay Chat (IRC) server, a text-based communication protocol on the Internet that offers real-time messaging.

You will use an actual IRC client to connect to your server and test it.

## Getting Started
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

## Prerequisites
The project is written in C++ 98, so you will need a C++ compiler that supports the C++ 98 standard. No additional libraries are required.

## Installation
First, clone the project:

```sh
git clone https://github.com/khirooo/42_irc.git
```

Then, navigate to the project directory and build the project using the provided Makefile:

```sh
cd <repository_dir>
make
```
This should generate an executable called ircserv.

## Usage
You can start the IRC server using the following command:

```sh
./ircserv <port> <password>
```
Where:

port - The port number on which your IRC server will listen for incoming IRC connections.
password - The connection password needed by any IRC client that tries to connect to your server.

## Features
The server has the following features:

Handle multiple clients at the same time without hanging.

Communication via TCP/IP (v4 or v6).

Basic IRC functionality:

Authenticate, set a nickname, a username, join a channel, send and receive private messages.

Forward messages from one client to a channel to every other client in the channel.

Operator and regular user roles with specific commands for channel operators:

KICK - Eject a client from the channel

INVITE - Invite a client to a channel

TOPIC - Change or view the channel topic

MODE - Change the channel’s mode with various options

## Testing
To ensure that your server correctly processes everything that you send to it, a simple test using nc can be done:

```sh
$> nc 127.0.0.1 6667
com^Dman^Dd
$>
```

## Contributing
Any contributions you make are greatly appreciated. For major changes, please open an issue first to discuss what you would like to change.
