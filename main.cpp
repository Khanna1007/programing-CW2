#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Tchar.h>
#include <thread>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

// Define a Node structure to represent each client socket
struct Node {
    SOCKET clientSocket;
    Node* next;
    Node(SOCKET socket) : clientSocket(socket), next(nullptr) {}
};

bool initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void Interactwithclient(SOCKET clientSocket, Node*& head) {
    cout << "Client connected" << endl;
    char buffer[4096];

    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            cout << "Disconnected from client" << endl;
            break;
        }

        string message(buffer, bytesReceived);
        cout << "Message from client: " << message << endl;

        // Send the message to all clients except the sender
        Node* current = head;
        while (current != nullptr) {
            if (current->clientSocket != clientSocket) {
                send(current->clientSocket, message.c_str(), message.length(), 0);
            }
            current = current->next;
        }
    }

    // Remove the client from the linked list
    Node* current = head;
    Node* prev = nullptr;
    while (current != nullptr) {
        if (current->clientSocket == clientSocket) {
            if (prev != nullptr) {
                prev->next = current->next;
            }
            else {
                head = current->next;
            }
            delete current;
            break;
        }
        prev = current;
        current = current->next;
    }

    closesocket(clientSocket);
}

int main() {
    if (!initialize()) {
        cout << "Winsock initialization failed" << endl;
        return 1;
    }

    cout << "Karim's chat application" << endl;

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        cout << "Cannot create socket" << endl;
        return 1;
    }

    // Address structure
    int port = 12345;
    sockaddr_in srvraddr;
    srvraddr.sin_family = AF_INET;
    srvraddr.sin_port = htons(port);

    if (InetPton(AF_INET, _T("127.0.0.1"), &srvraddr.sin_addr) != 1) {
        cout << "Cannot set address structure" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&srvraddr), sizeof(srvraddr)) == SOCKET_ERROR) {
        cout << "Bind unsuccessful" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server listening on port: " << port << endl;

    Node* head = nullptr; // Head of the linked list

    // Accept client connections
    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cout << "Invalid client socket" << endl;
        }
        else {
            // Create a new node for the client socket
            Node* newNode = new Node(clientSocket);
            newNode->next = head;
            head = newNode;

            // Start a new thread to handle client interaction
            thread t1(Interactwithclient, clientSocket, std::ref(head));
            t1.detach();
        }
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
