#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <thread>

#include <iostream>
#include <sstream>
#include <fstream>
#include "http_message.h"

bool address_look_up(string host_name, string port, string& host_address)
{
    struct addrinfo hints;
    struct addrinfo* res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = 0;
    if ((status = getaddrinfo(host_name.c_str(), port.c_str(), &hints, &res)) != 0) 
    {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        return false;
    }

    std::cout << "IP addresses for " << host_name << ": " << std::endl;

    for(struct addrinfo* p = res; p != 0; p = p->ai_next) 
    {
        // convert address to IPv4 address
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
        // convert the IP to a string and print it:
        char ipstr[INET_ADDRSTRLEN] = {'\0'};
        inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
        std::cout << "  " << ipstr << std::endl;
        host_address = ipstr;
    }
    freeaddrinfo(res); 
    return true;
}

void task(int id, int clientSockfd, string dir)
{
    char buf[1000] = {0};

    memset(buf, '\0', sizeof(buf));

    if (recv(clientSockfd, buf, 1000, 0) == -1) {
        perror("recv");
        return;
    }

    vector<char> wire(buf, buf + strlen(buf));
    string wired_string(wire.begin(), wire.end());  

    http_request my_request;
    my_request.decode(wire);

    cout << "received request's method: " << my_request.get_method() << endl;
    cout << "received request's url: " << my_request.get_url() << endl;

    http_response my_response;

    string file_name = dir + my_request.get_url();

    cout << "file_name" << file_name << endl;

    ifstream file_stream(file_name, ios::in|ios::binary);
    if (!file_stream)
    {
        my_response.set_status_code("404");
        cout << "open not successfully " << endl;
    }
    else
    {
        my_response.set_status_code("200");
        vector<char> data_vec((istreambuf_iterator<char>(file_stream)), istreambuf_iterator<char>());
        my_response.add_data(data_vec);
    }

    wire = my_response.encode();

    if (send(clientSockfd, &wire[0], wire.size(), 0) == -1) {
        perror("send");
        return;
    }
    close(clientSockfd);
}

int main(int argc, char *argv[])
{
    // set the argument of the server
    string host_name = "localhost";
    string host_address;
    int port = 4000;
    string dir = ".";

    if (argc >= 2)
        host_name = argv[1];
    if (argc >= 3)
        port = stoi(argv[2]);
    if (argc >= 4)
        dir = argv[3];

    cout << "information about server:    host_name = " << host_name << " port = " << port << " dir = " << dir << endl;

    // create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // allow others to reuse the address 
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }

    if (!address_look_up(host_name, to_string(port), host_address))
    {
        perror("Unknown hostname");
        return 3;
    }

    // bind address to socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    // define the port of the server, short, network byte order
    addr.sin_port = htons(port);         
    // define the IP address of the server
    addr.sin_addr.s_addr = inet_addr(host_address.c_str());
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    // bind the server's socket to the address and port
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return 2;
    }

    // set socket to listen status
    if (listen(sockfd, 1) == -1) {
        perror("listen");
        return 3;
    }

    // accept a new connection, clientSockfd is the new socket for the connection
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    int thread_id = 0;

    while (true)
    {
        int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSockfd == -1) {
            perror("accept");
            continue;
        }

        // get the IP and port of the client
        char ipstr[INET_ADDRSTRLEN] = {'\0'};
        inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
        std::cout << "Accept a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;
        
        thread t;
        t = thread(task, thread_id, clientSockfd, dir);
        thread_id++;
        t.join();
    }

    return 0;
}
