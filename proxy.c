
// Libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

// Global Var Declaration
// ---------------------
const int httpPort = 80;
int port;
int serverSocket;
struct sockaddr_in server;
struct sockaddr_in requestFromBrowser;
int bindingStatus;
int listenStatus;
int sockAddrSize;
int clientSocket;
char browserReqReceived[1024];
char browserReqHost[1024];
char browserReqPath[1024];
char *IPbuffer;
char getRequest[1024];
int clientToServer;
struct sockaddr_in serverComms;
char *responseFromServer[2048];
int connectStatusServer;
int sendStatusServer;
int sizeOfResponseFromServer;
char pathOfBlocked[1024] = "/~carey/CPSC441/ass1/error.html";
char hostOfBlocked[1024] = "136.159.2.17";
char* blockedWords[3] = {"floppy","spongebob","Curling"};

// Function Declarations
// ---------------------
void askForPort();
/*
* Asks for a Port Value.
* Must be greater then 1000 and less then 9999
* Will reprompt if the Port is deemed invalid
* Sets the value of int Port
*/

void buildFailure(int programPos);
/*
* Will return a message to console depending on the pos of the error in the code
*/

void buildSuccess(int programPos);
/*
* Will return a message to console depending on the pos of the success in the code
*/

void parseHTTP();
/*
* Function Parses GET REQUEST from Web browser to use for Web Server
*/

int checkForBlocked();
/*
* Will return -1 if blocked string exists, else will return 1
*/

int main(int argc, char *argv[])
{
    // prompt for PORT
    askForPort();
    // create Socket
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    sockAddrSize = sizeof(struct sockaddr_in);
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1 ||
        ((bindingStatus = bind(serverSocket, (struct sockaddr *)&server, sizeof(server))) == -1) ||
        (listenStatus = listen(serverSocket, 3)) == -1)
    {
        buildFailure(1);
    }
    buildSuccess(1);
    while (1)
    {
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&requestFromBrowser, (socklen_t *)&sockAddrSize)) == -1)
        {
            buildFailure(2);
        }
        buildSuccess(2);
        recv(clientSocket, browserReqReceived, 1024, 0);
        parseHTTP();
        serverComms.sin_addr.s_addr = inet_addr(IPbuffer);
        serverComms.sin_family = AF_INET;
        serverComms.sin_port = htons(httpPort);
        if(checkForBlocked() == -1) {
            sprintf(getRequest, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", pathOfBlocked, hostOfBlocked);
            puts("Request is BLOCKED, redirecting error page\n");
        }
        if ((clientToServer = socket(AF_INET, SOCK_STREAM, 0)) == -1 ||
            (connectStatusServer = connect(clientToServer, (struct sockaddr *)&serverComms, sizeof(serverComms)) == -1) ||
            ((sendStatusServer = send(clientToServer, getRequest, strlen(getRequest), 0))) == -1)
        {
            buildFailure(3);
        }
        buildSuccess(3);
        while ((sizeOfResponseFromServer = recv(clientToServer,responseFromServer, 2048, 0)) > 0)
        {
            send(clientSocket, responseFromServer, sizeOfResponseFromServer, 0);
            memset(&responseFromServer, 0, sizeof(responseFromServer));
        }
        memset(&responseFromServer, 0, sizeof(responseFromServer));
        close(clientToServer);
        close(clientSocket);
    }

    return 0;
}

void askForPort()
{
    printf("Please Enter The Port You Wish To Use: ");
    scanf("%d", &port);
    while (port < 1000 || port > 9999)
    {
        printf("Please Input A Valid Port Between 1000 and 9999: ");
        scanf("%d", &port);
    }
    printf("You've Selected Port: %d - Please Configure Your Browser to use Localhost or 127.0.0.1 and %d\n", port, port);
}

void buildFailure(int programPos)
{
    if (programPos == 1)
    {
        printf("Build Failure: Error Building, Binding, Listening of Server Socket");
        exit(0);
    }
    if (programPos == 2)
    {
        printf("Build Failure: Error Accepting Incoming Request");
        exit(0);
    }
    if (programPos == 3)
    {
        printf("Build Failure: Connecting or Sending to Web Server");
        exit(0);
    }
}

void buildSuccess(int programPos)
{
    if (programPos == 1)
    {
        printf("Checkpoint 1: Server Socket Succesfully Created, Binded, Listening\nPlease Configure Web Browser...\n");
    }
    if (programPos == 2)
    {
        printf("Update: Connection has been Accepted\n");
    }
    if (programPos == 3)
    {
        printf("Checkpoint 2: Request Sent To Browser\n");
    }
}

void parseHTTP()
{
    int pos = 11;
    int tempPos = 0;
    memset(browserReqHost, 0, sizeof(browserReqHost));
    memset(browserReqPath, 0, sizeof(browserReqPath));
    while (1)
    {
        if (browserReqReceived[pos] != '/')
        {
            browserReqHost[tempPos] = browserReqReceived[pos];
            pos++;
            tempPos++;
        }
        else
        {
            break;
        }
    }
    tempPos = 0;
    while (browserReqReceived[pos] != ' ')
    {
        browserReqPath[tempPos] = browserReqReceived[pos];
        pos++;
        tempPos++;
    }

    struct hostent *host_entry;
    host_entry = gethostbyname(browserReqHost);
    IPbuffer = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
    sprintf(getRequest, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", browserReqPath, IPbuffer);
}

int checkForBlocked() {

    for(int i = 0;i<3;i++) {
        char *temp = blockedWords[i];
        int lengthOfWord = strlen(temp);
        int tempPos = 0;
        for(int j = 0; j<strlen(browserReqPath);j++) {
            if(tolower(browserReqPath[j]) == tolower(temp[tempPos])) {
                while(tolower(browserReqPath[j]) == tolower(temp[tempPos])) {
                    tempPos++;
                    j++;
                    if(tempPos == lengthOfWord) {
                        return -1;
                    }
                }
            }
        }

    }
    return 1;
}
