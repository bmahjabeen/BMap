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
#include "Common.h"

using namespace std;

static void logError(string msg)
{
    cerr << "Error: " << msg <<endl;
}

static void log(string msg)
{
    cout << msg <<endl;
}

class Client
{
private:

    enum UserType {
        Admin,
        Normal
    };

    UserType userType;
    const static int normalUserPermLength = 4;
    const static int adminUserPermLength = 10;
    Command userInputToCommandForNormal[normalUserPermLength] = {Exit, PrintMap, GetTrip, FindPoi};
    Command userInputToCommandForAdmin[adminUserPermLength] = {Exit, AddVertex, AddEdge, PrintMap, GetTrip, FindPoi, AddEdgeEvent,
                                              Store, Retrieve, Reset};


    int prepareClientSocket(string* server, int* port)
    {
        struct sockaddr_in clientSockAddr;
        int clientSock;
        int optionValue = 1;

        clientSock = socket(AF_INET, SOCK_STREAM, 0);
        if(clientSock == -1){
            logError("Could not create socket");
            return -1;
        }
        if( (setsockopt(clientSock, SOL_SOCKET, SO_REUSEADDR, (char*)&optionValue, sizeof(int)) == -1 )||
            (setsockopt(clientSock, SOL_SOCKET, SO_KEEPALIVE, (char*)&optionValue, sizeof(int)) == -1 ) ){
            logError("Could not setup socket");
            return -1;
        }
        clientSockAddr.sin_family = AF_INET ;
        clientSockAddr.sin_port = htons(*port);
        memset(&(clientSockAddr.sin_zero), 0, 8);
        clientSockAddr.sin_addr.s_addr = inet_addr(server->c_str());
        if(connect(clientSock, (struct sockaddr*)&clientSockAddr, sizeof(clientSockAddr)) == -1){
            logError("Could not connect to socket");
            return -1;
        }
        return clientSock;
    }

    UserType processLogin()
    {
        string uname, pwd;

        // TODO implement real server side encrypted authentication + authorization
        while (true) {
            cout << "Enter username: ";
            cin >> uname;
            cout << "Enter password: ";
            cin >> pwd;
            if (uname == "admin" && pwd == "admin") {
                return UserType::Admin;
            } else if (uname == "user" && pwd == "user") {
                return UserType::Normal;
            } else {
                logError("Invalid username or password");
            }
        }
    }

    void printAvailableCommands() {
        if (userType == Admin) {
            printf("Available Commands: 1: Add Vertex, 2: Add Edge, 3: Print Map, 0: Exit\n");
        } else if (userType == Normal) {
            printf("Available Commands: 1: Print Map, 0: Exit\n");
        }
    }

    Command getCommand(char* request)
    {
        int value = request[0] - '0';
        if (value < 0 || (userType == Normal && value >= normalUserPermLength)
                || (userType == Admin && value >= adminUserPermLength))
        {
            logError("Invalid command");
            return Invalid;
        }
        if (userType == Admin) {
            return userInputToCommandForAdmin[value];
        }
        else if (userType == Normal) {
            return userInputToCommandForNormal[value];
        }
    }

    void startListening(int clientSocket)
    {
        char* request = (char*)malloc(sizeof(char) * REQUEST_BUFFER_SZ);
        char* response = (char*)malloc(sizeof(char) * RESPONSE_BUFFER_SZ);

        userType = processLogin();
        printAvailableCommands();

        while (true)
        {
            memset(request, 0, REQUEST_BUFFER_SZ);
            fgets(request, REQUEST_BUFFER_SZ, stdin);
            request[strlen(request) - 1] = '\0';
            if (strlen(request) < 1) continue;

            Command command = getCommand(request);
            if (command == Invalid) {
                printAvailableCommands();
                continue;
            }
            request[0] = '0' + command;

            if(send(clientSocket, request, strlen(request), 0) == -1){
                logError("sending request");
                break;
            }

            memset(response, 0, RESPONSE_BUFFER_SZ);
            if(recv(clientSocket, response, RESPONSE_BUFFER_SZ, 0) == -1){
                logError("receiving response");
                break;
            }

            log("Received response:");
            log(response);
        }
        free(request);
        free(response);
        close(clientSocket);
    }

public:

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
        string str = string(argv[1]);
        int number = stoi(argv[2], &sz);
        if (str.size() < 1 || number < MIN_PORT)
        {
            throw invalid_argument("");
        }
        *port = number;
        *server = str;
    }
    catch (exception &e)
    {
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    string serverIP;
    int port;
    if (parseInput(argc, argv, &serverIP, &port) < 0)
    {
        showUsage(argv[0]);
        logError("Invalid program argument");
        return -1;
    }

    Client client;
    client.start(&serverIP, &port);
    return 0;
}

