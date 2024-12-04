#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string_view>
#include <fstream>
#include <unistd.h>

using namespace std;


int main(int argc, char* argv[]) {

    if (argc != 2) {
        cout << "please specify port" << endl;
        exit(0);

    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cout << "socket creation broken" << endl;
        exit(0);
    } 

    int serverPort = atoi(argv[1]);


    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(serverPort);

    if(bind(serverSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0 ) {
        cout << "socket bind broken" << endl;
        exit(0);
    }

    if(listen(serverSocket, 5) < 0) {
        cout << "listen broken" << endl;
        exit(0);
    }

    cout << "server running" << endl;

    int clientSocket;
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr); // not sure why this doesnt work when i try sizeof() inside accept()


    while(1) {
        clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            cout << "accept broken" << endl;
            continue;
        }

        char buf[1500];
        int bytesRead = 0;
        bytesRead = recv(clientSocket, buf, sizeof(buf), 0);

        string request(buf);
        size_t requestType = request.find("GET");

        // format: GET /home.html HTTP/1.1\r\nHost: 127.0.0.1:28000\r\nAccept: text/html\r\nConnection: keep-alive\r\n\r\n
        // need to check GET and find home.html (exclude / in filepath)
        if (requestType == string::npos) {
            cout << "request type not supported " << endl;
            close(clientSocket);
            continue;
        }
        else {
            requestType += 4; // skip 'GET '
            size_t fileEnd = request.find(' ', requestType);
            string filePath = request.substr(requestType, fileEnd - requestType);
            filePath = '.' + filePath;

            
            string response;
            fstream file;
            file.open(filePath, ios::in | ios::binary);

            if(file) {
                string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
                response = "HTTP/1.1 200 OK\r\nContent-Type: ";
                if(request.find(".html") != string::npos) {
                    response += "text/html; charset=utf-8\r\n\r\n";
                    response += content;
                }
                else if (request.find(".pdf") != string::npos) {
                    response += "application/pdf\r\n\r\n";
                    response += content;
                }
                else if (request.find(".jpeg") != string::npos) {
                    response += "image/jpeg\r\n\r\n";
                    response += content;
                }
                else {
                    response = "HTTP/1.1 404 Not Found\r\n\r\n404 Not Found";
                }
            }

            else {
                response = "HTTP/1.1 404 Not Found\r\n\r\n404 Not Found";
            }

            send(clientSocket, response.c_str(), response.size(), 0);
            
        }
        close(clientSocket);
    }

}