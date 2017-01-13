#include <iostream>
#include <stdlib.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cstring>

using namespace std;

class Client
{
private:
    static void logError(string msg)
    {
        cerr << "Error: " << msg <<endl;
    }

    static void log(string msg)
    {
        cout << msg <<endl;
    }

    int prepareClientSocket(string* server, int* port)
    {
        struct sockaddr_in clientSockAddr;
        char* buffer = (char*)calloc(sizeof(char), 2048);
        int buffer_len = 2048;
        int clientSock;
        int optVal = 1;

        clientSock = socket(AF_INET, SOCK_STREAM, 0);
        if(clientSock == -1){
            logError("Could not create socket");
            return -1;
        }
        if( (setsockopt(clientSock, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(int)) == -1 )||
            (setsockopt(clientSock, SOL_SOCKET, SO_KEEPALIVE, (char*)&optVal, sizeof(int)) == -1 ) ){
            logError("Setup socket");
            return -1;
        }
        clientSockAddr.sin_family = AF_INET ;
        clientSockAddr.sin_port = htons(*port);
        memset(&(clientSockAddr.sin_zero), 0, 8);
        clientSockAddr.sin_addr.s_addr = inet_addr(server->c_str());
        if(connect(clientSock, (struct sockaddr*)&clientSockAddr, sizeof(clientSockAddr)) == -1){
            logError("connect socket");
            return -1;
        }
        return clientSock;
    }

    void startListening(int clientSocket)
    {
        char* response = (char*)calloc(sizeof(char),2048);
        if(recv(clientSocket, response, 2048, 0) == -1){
            logError("receiving response");
        }

        cout << "Received response: " << response << endl;
        close(clientSocket);
    }

public:
    static const int MIN_PORT = 1;

    void start(string* server, int* port)
    {
        int clientSocket = prepareClientSocket(server, port);
        if (clientSocket < 0)
        {
            logError("Cannot prepare client socket");
            return;
        }
        startListening(clientSocket);
    }
};


static void showUsage(std::string name)
{
    cerr << "Usage: " << name << " SERVER_ADDRESS PORT_NUMBER" << endl;
}

int parseInput(int argc, char* argv[], string* server, int* port)
{
    try
    {
        if (argc != 3)
        {
            throw invalid_argument("");
        }
        std::string::size_type sz;
        *server = string(argv[1]);
        int number = stoi(argv[2], &sz);
        if (server->size() < 1 || number < Client::MIN_PORT)
        {
            throw invalid_argument("");
        }
        *port = number;
    }
    catch (exception &e)
    {
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    string server;
    int port;
    if (parseInput(argc, argv, &server, &port) < 0)
    {
        showUsage(argv[0]);
        return -1;
    }

    Client client;
    client.start(&server, &port);
    return 0;
}

