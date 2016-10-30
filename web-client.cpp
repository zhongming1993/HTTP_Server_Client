#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include "http_message.h"

bool parse_total_url(string total_url, string& host_name, int& port_num, string& dir)
{
    size_t position = total_url.find("://");
    if (position == string::npos)
        position = 0;
    else
        position += 3;
    string element;
    while (position < total_url.size() && total_url[position] != ':')
    {
        element += total_url[position];
        position++;
    }
    if (position >= total_url.size())
        return false;
    host_name = element;
    element.clear();
    position++;
    if (position >= total_url.size())
        return false;
    while (position < total_url.size() && total_url[position] != '/')
    {
        element += total_url[position];
        position++;
    }
    port_num = stoi(element);
    dir = total_url.substr(position);
    if (dir == "/")
        dir = "/index.html";
    return true;
}

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


int main(int argc, char *argv[])
{
    string total_url;
    string host_name;
    string host_address;
    int port_num;
    string dir;
    if (argc <= 1)
    {
        cerr << "Please specify an url" << endl;
        return -1;
    }
    else
        total_url = argv[1];

    cout << "total_url: " << total_url << endl;

    if  (!parse_total_url(total_url, host_name, port_num, dir))
    {
        perror("invalid url");
        return 2;
    }

    if (!address_look_up(host_name, to_string(port_num), host_address))
    {
        perror("Unknown hostname");
        return 3;
    }

    cout << "host_name: " << host_name << endl;
    cout << "port_num " << port_num << endl;
    cout << "dir " << dir << endl;

    // create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    // short, network byte order
    // define the port of the server
    serverAddr.sin_port = htons(port_num); 
    // define the ip address of the server
    serverAddr.sin_addr.s_addr = inet_addr(host_address.c_str());
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    // connect to the server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("connect");
        return 3;
    }

    // // get client's IP and port
    // struct sockaddr_in clientAddr;
    // socklen_t clientAddrLen = sizeof(clientAddr);
    // if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    //     perror("getsockname");
    //     return 4;
    // }

    // char ipstr[INET_ADDRSTRLEN] = {'\0'};
    // inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    // std::cout << "Set up a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;
    // // 
 
    char buf[1000] = {0};

    //while (true) {
    memset(buf, '\0', sizeof(buf));

    std::cout << "sending http request" << endl;
    http_request my_request;
    my_request.set_url(dir);
    vector<char> wire = my_request.encode();
    string wired_string(wire.begin(), wire.end());

    cout << wired_string << "size of http_request " << wire.size() << endl;

        //std::cin >> input;
    if (send(sockfd, &wire[0], wire.size(), 0) == -1) {
        perror("send");
        return 5;
    }

    vector<char> total_data;
    while (recv(sockfd, buf, 1000, 0) > 0)
    {
        vector<char> wire2(buf, buf + strlen(buf));
        string wired_string2(wire2.begin(), wire2.end());

        // cout << "received wired_string: " << wired_string2 << endl;

        http_response my_response;
        my_response.decode(wire2);

        if (my_response.get_status_code() != "200")
        {
            perror("unsuccessful http_response");
            return 6;
        }
        // cout << "received response's version: " << my_response.get_version() << endl;
        // cout << "received response's status_code: " << my_response.get_status_code() << endl;
        std::vector<char> received_data = my_response.get_data();
        total_data.insert(total_data.end(), received_data.begin(), received_data.end());
    }

    // string data_string(total_data.begin(), total_data.end());
    // cout << "received total data: " << data_string << endl;

    string file_name = dir.substr(1);
    ofstream downloaded_file(file_name, ios::out | ios::binary);
    copy(total_data.begin(), total_data.end(), ostreambuf_iterator<char>(downloaded_file));

    close(sockfd);

    return 0;
}
