#include <iostream>
#include <fstream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

// Global variable to track the current logged-in users
string loggedInUser;

// Vigenere cipher key
const string vigenereKey = "encrypt";

// Encrypts the password using the Vigenere cipher
string encryptPassword(string password) {
    string encryptedPassword = "";
    for (size_t i = 0; i < password.length(); ++i) {
        char plainChar = password[i];
        char keyChar = vigenereKey[i % vigenereKey.length()];
        encryptedPassword += char((plainChar + keyChar) % 256);
    }
    return encryptedPassword;
}

// Decrypts the password using the Vigenere cipher
string decryptPassword(string password) {
    string decryptedPassword = "";
    for (size_t i = 0; i < password.length(); ++i) {
        char cipherChar = password[i];
        char keyChar = vigenereKey[i % vigenereKey.length()];
        decryptedPassword += char((cipherChar - keyChar + 256) % 256);
    }
    return decryptedPassword;
}

// Encrypts the message using the Caesar cipher with a shift of 5
string encryptMessage(string message) {
    string encryptedMessage = "";
    for (char& c : message) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            encryptedMessage += char((c - base + 5) % 26 + base);
        }
        else {
            encryptedMessage += c;
        }
    }
    return encryptedMessage;
}

// Decrypts the message using the Caesar cipher with a shift of 5
string decryptMessage(string message) {
    string decryptedMessage = "";
    for (char& c : message) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            decryptedMessage += char((c - base - 5 + 26) % 26 + base);
        }
        else {
            decryptedMessage += c;
        }
    }
    return decryptedMessage;
}

// Checks if a user already exists
bool userExists(const string& username) {
    ifstream userFile(username + "_users.txt");
    return userFile.good();
}

// Adds a new user
void addUser(const string& username, const string& password) {
    if (userExists(username)) {
        cout << "Error: Username already exists." << endl;
        return;
    }
    ofstream userFile(username + "_users.txt");
    if (userFile.is_open()) {
        userFile << encryptPassword(password) << endl;
        userFile.close();
        cout << "User added successfully." << endl;
    }
    else {
        cout << "Error: Unable to open the user file." << endl;
    }
}

// Authenticates a user
bool authenticateUser(const string& username, const string& password) {
    ifstream userFile(username + "_users.txt");
    if (userFile.is_open()) {
        string storedPassword;
        getline(userFile, storedPassword);
        userFile.close();
        if (decryptPassword(storedPassword) == password) {
            loggedInUser = username;
            return true;
        }
    }
    return false;
}

bool initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void SendMsg(SOCKET s) {
    cout << "Enter your chat name: ";
    string name;
    getline(cin, name);
    string message;

    while (true) {
        getline(cin, message);
        string encryptedMessage = encryptMessage(message); // Encrypt the message only

        // Combine the unencrypted name with the encrypted message
        string msg = name + ": " + encryptedMessage;

        // Encrypt the combined message (name + encrypted message)
        msg = encryptMessage(msg);

        int bytesent = send(s, msg.c_str(), msg.length(), 0);
        if (bytesent == SOCKET_ERROR) {
            cout << "Error sending message." << endl;
            break;
        }
        if (message == "exit") {
            cout << "Exiting chat." << endl;
            break;
        }
    }
    closesocket(s);
    WSACleanup();
}

void ReceiveMsg(SOCKET s) {
    char buffer[4096];
    int recvLength;
    string msg = "";
    while (true) {
        recvLength = recv(s, buffer, sizeof(buffer), 0);
        if (recvLength <= 0) {
            cout << "Disconnected from the server." << endl;
            break;
        }
        else {
            string receivedMsg = string(buffer, recvLength);

            // Decrypt the received message
            receivedMsg = decryptMessage(receivedMsg);

            // Find the position of the first colon (':') to separate name and message
            size_t colonPos = receivedMsg.find(':');
            if (colonPos != string::npos && colonPos < receivedMsg.length() - 2) {
                // Extract name and message parts
                string name = receivedMsg.substr(0, colonPos);
                string encryptedMessage = receivedMsg.substr(colonPos + 2);

                // Decrypt the message part only
                string decryptedMessage = decryptMessage(encryptedMessage);

                // Print the decrypted name and encrypted message
                cout << name << ": " << decryptedMessage << endl;
            }
            else {
                cout << "Invalid message format." << endl;
            }
        }
    }
    closesocket(s);
    WSACleanup();
}



int main() {
    if (!initialize()) {
        cout << "Winsock initialization failed." << endl;
        return 1;
    }

    while (true) {
        cout << "Choose an option:" << endl;
        cout << "1. Signup" << endl;
        cout << "2. Login" << endl;
        cout << "3. Exit" << endl;

        int choice;
        cin >> choice;
        cin.ignore(); // Ignore the newline character in the input buffer

        switch (choice) {
        case 1: {
            string username, password;
            cout << "Enter a username: ";
            getline(cin, username);
            cout << "Enter a password: ";
            getline(cin, password);
            password = encryptPassword(password); // Encrypt the password
            addUser(username, password);
            break;
        }
        case 2: {
            string username, password;
            cout << "Enter your username: ";
            getline(cin, username);
            cout << "Enter your password: ";
            getline(cin, password);
            password = encryptPassword(password); // Encrypt the password
            if (authenticateUser(username, password)) {
                // Proceed to chat
                SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
                if (s == INVALID_SOCKET) {
                    cout << "Invalid socket creation" << endl;
                    return 1;
                }

                int port = 12345;
                string serveraddress = "127.0.0.1";
                sockaddr_in srvraddr;
                srvraddr.sin_family = AF_INET;
                srvraddr.sin_port = htons(port);
                inet_pton(AF_INET, serveraddress.c_str(), &(srvraddr.sin_addr));

                if (connect(s, reinterpret_cast<sockaddr*>(&srvraddr), sizeof(srvraddr)) == SOCKET_ERROR) {
                    cout << "Unable to connect to server: " << WSAGetLastError() << endl;
                    closesocket(s);
                    WSACleanup();
                    return 1;
                }
                cout << "Successfully connected to server." << endl;

                thread senderThread(SendMsg, s);
                thread receiverThread(ReceiveMsg, s);

                senderThread.join();
                receiverThread.join();

                return 0;
            }
            else {
                cout << "Invalid username or password. Please try again." << endl;
            }
            break;
        }
        case 3:
            cout << "Exiting program." << endl;
            return 0;
        default:
            cout << "Invalid choice. Please try again." << endl;
            continue;
        }
    }
}
