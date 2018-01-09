#Part2Part
This is a peer-to-peer application meant to share files with anybody who's connected to the server. Any client running this application will have support for uploading and downloading shared files on the network. It has support for both IPv4, IPv6, but only one of it may be used at a time.
##Dependencies
You need the c++14 standard in order to compile and build the server and the client and Internet connectivity.
##Installation
Download the project on your machine. After unzip-ing the downloaded file, navigate from the terminal to the unarchived folder and simply run the `make` command.
##Running the application
By default, the `make` command creates 2 files: `server.out` and `client.out`.
To start the server, simply run the `server.out` file (it takes no additional arguments). To start the client. run `client.out <server_ip> <listening_port>`, where the `<listening_port>` default value is 1234.
An example of installing and running the application is:
* `make` - builds the binary files
* `./server.out` - start the server on port `1234`
* `./client.out 127.0.0.1 1234` - connect to localhost on port 1234
After starting the client, the user has access to the following commands:
* `add file` - the application reads the relative path to a file which the user wishes to put at disposal for download
* `show files` - shows the available files for download
* `download file` - downloads a specific file; additional instructions are given inside the application
* `exit` - exits the application