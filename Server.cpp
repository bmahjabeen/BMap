#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <stdexcept>
#include <sstream>
#include <Constants.h>

using namespace std;

void logError(string msg)
{
    cerr << "Error: " << msg <<endl;
}

void log(string msg)
{
    cout << msg <<endl;
}

class Server
{
private:

    enum Command {
        END_SESSION
    };

    int prepareServerSocket(int port)
    {
        struct sockaddr_in socketAddress;
        int serverSocket;
        int optionValue = 1;

        if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1 ||
           (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optionValue, sizeof(int)) == -1 )||
           (setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&optionValue, sizeof(int)) == -1 )){
            logError("Could not create socket");
            return -1;
        }

        socketAddress.sin_family = AF_INET ;
        socketAddress.sin_port = htons(port);
        memset(&(socketAddress.sin_zero), 0, 8);
        socketAddress.sin_addr.s_addr = INADDR_ANY;

        if(bind(serverSocket, (sockaddr*)&socketAddress, sizeof(socketAddress)) == -1 ||
           listen(serverSocket, 10) == -1) {
            logError("Could not bind or listen");
            return -1;
        }

        return serverSocket;
    }

    static string processCommand(istringstream* requestMsg)
    {
        return requestMsg->str();
    }

    static Command getCommand(istringstream* iss)
    {
        int command;
        *iss >> command;
        Command rv;
        if (command == 0)
        {
            rv = END_SESSION;
        }
        return rv;
    }

    static void* ClientHandler(void *arg)
    {
        int *clientSocket = (int*)arg;
        char* request = (char*)malloc(sizeof(char) * REQUEST_BUFFER_SZ);

        while (true) {
            memset(request, 0, REQUEST_BUFFER_SZ);
            if (!recv(*clientSocket, request, REQUEST_BUFFER_SZ, 0)) {
                logError("Could not receive request");
                break;
            }
            log("Received request: ");
            log(request);

            istringstream requestMsg(request);
            Command command = getCommand(&requestMsg);
            string response = processCommand(&requestMsg);
            log("Server sending:");
            log(response);
            if (!send(*clientSocket, response.c_str(), response.size(), 0)) {
                logError("Could not sending response");
                break;
            }
            if (command == Command::END_SESSION)
            {
                break;
            }
        }
        free(clientSocket);
        free(request);
        log("Client left, session closed.");
        return 0;
    }

    void startListening(int serverSocket)
    {
        struct sockaddr_in socketAddress;
        int* clientSocket;
        socklen_t socketLength = sizeof(sockaddr_in);
        pthread_t threadId = 0;

        while(true)
        {
            log("Waiting for client...");
            clientSocket = (int*)malloc(sizeof(int));
            if((*clientSocket = accept(serverSocket, (sockaddr*)&socketAddress, &socketLength))== -1) {
                logError("Could not establish client connection");
                continue; // or terminate?
            }
            log("Got client: ");
            log(inet_ntoa(socketAddress.sin_addr));
            pthread_create(&threadId, 0, &ClientHandler, (void*)clientSocket);
            pthread_detach(threadId);
        }
    }

public:
    static const int MIN_PORT = 1; // TODO find a better lower bound

    void start(int port)
    {
        int serverSocket = prepareServerSocket(port);
        if (serverSocket < 0)
        {
            logError("Cannot prepare server socket");
            return;
        }
        startListening(serverSocket);
    }
};

void showUsage(std::string name)
{
    cerr << "Usage: " << name << " PORT_NUMBER" << endl;
}

int parseInput(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            throw invalid_argument("");
        }
        std::string::size_type sz;
        int port = stoi(argv[1], &sz);
        if (port < Server::MIN_PORT)
        {
            throw invalid_argument("");
        }
        return port;
    }
    catch (exception &e)
    {
        return -1;
    }
}

int main(int argc, char* argv[])
{
    int port = parseInput(argc, argv);
    if (port < 0)
    {
        showUsage(argv[0]);
        logError("Invalid program parameter");
        return -1;
    }

    Server server;
    server.start(port);
    return 0;
}

