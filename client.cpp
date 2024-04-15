#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>


using namespace std; 

#pragma comment(lib, "Ws2_32.lib")

bool initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0; /* sure the socket startup is working. Makeword 2,2 is version*/
    return 1;
}
int main() {
    if (!initialize()) {
        cout << "winsock initialization failed" << endl; /* if statup failed*/
    }

    SOCKET s;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "Invalid socket creation" << endl;
            return 1;
    };
    
    int port = 12345;
    string serveraddress = "127.0.0.1";
    sockaddr_in srvraddr;
    srvraddr.sin_family = AF_INET;
    srvraddr.sin_port = htons(port);
    inet_pton(AF_INET, serveraddress.c_str(), &(srvraddr.sin_addr));
    
    if (connect(s, reinterpret_cast<sockaddr*>(&srvraddr), sizeof(srvraddr)) == SOCKET_ERROR) {
        cout << "unable to connect to server" << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }
    cout << "succssefully conneected to server" << endl;

    //send 
    string message = "Hello and welcome";
    int bytessent;
    bytessent = send(s, message.c_str(), message.length(), 0);

    if (bytessent == SOCKET_ERROR) {
        cout << "sent failed" << endl;
    }
    
    closesocket(s);

    WSACleanup();
    return 0;
}

