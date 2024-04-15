#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Tchar.h>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

bool initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0; /* sure the socket startup is working. Makeword 2,2 is version*/
    return 1;
}


int main(){
    if (!initialize()) {
        cout << "winsock initialization failed" << endl; /* if statup failed*/
    }

    cout << "Karim's chat application" << endl;

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0); /*create listen socket with param. of IP, protocol, 0 service provider choice*/

    if (listenSocket == INVALID_SOCKET) {
        cout << "cannot create socket" << endl;
        return 1;
    }

    // address structure//
    int port = 12345;
    sockaddr_in srvraddr;
    srvraddr.sin_family = AF_INET;
    srvraddr.sin_port = htons(port);

    // conevert ip to binary (0.0.0.0) put in sin_family in binary

    if (InetPton(AF_INET, _T("127.0.0.1"), &srvraddr.sin_addr) != 1) {
        cout << "cannot set address structure" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
   }

    //bind ip and port with socket
    if (bind(listenSocket, reinterpret_cast <sockaddr*>(&srvraddr), sizeof(srvraddr)) == SOCKET_ERROR){
        cout << "bind unsuccesfful" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    //listen
    if(listen(listenSocket, SOMAXCONN) == SOCKET_ERROR){
        cout << "listen failed" << endl;
    closesocket(listenSocket);
    WSACleanup();
    return 1;
    }

    cout << " server listening on port: " << port << endl;

    //accept 
    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        cout << "invalid client socket" << endl;
    }

    char buffer[4096];
    int bytesrrecvd = recv(clientSocket, buffer, sizeof(buffer),0);

    string message(buffer, bytesrrecvd);
    cout << "message from client: " << message << endl;

    closesocket(clientSocket);

    closesocket(listenSocket);


    //
     
    WSACleanup(); /*clean up or finalize*/
    return 0;
}


