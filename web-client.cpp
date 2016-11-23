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

    for(struct addrinfo* p = res; p != 0; p = p->ai_next) 
    {
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
        char ipstr[INET_ADDRSTRLEN] = {'\0'};
        inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
        host_address = ipstr;
    }
    freeaddrinfo(res); 
    return true;
}

int main(int argc, char *argv[])
{
    vector<string> total_url_vec;
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
    {
        for (int i = 1; i < argc; i++)
            total_url_vec.push_back(argv[i]);
    }

    for (auto total_url: total_url_vec)
    {
        cout << "total_url: " << total_url << endl;   
        if  (!parse_total_url(total_url, host_name, port_num, dir))
        {
            perror("invalid url");
            continue;
        }
        if (!address_look_up(host_name, to_string(port_num), host_address))
        {
            perror("Unknown hostname");
            continue;
        }

        // create a socket using TCP IP
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port_num); 
        serverAddr.sin_addr.s_addr = inet_addr(host_address.c_str());
        memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

        // connect to the server
        if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("connect");
            continue;
        }
     
        char buf[1000] = {0};
        memset(buf, '\0', sizeof(buf));

        cout << "sending http request...." << endl;
        http_request my_request;
        my_request.set_url(dir);
        vector<char> wire = my_request.encode();
        string wired_string(wire.begin(), wire.end());


        if (send(sockfd, &wire[0], wire.size(), 0) == -1) {
            perror("send");
            return 5;
        }

        cout << "attempting to fetch file from " << total_url << endl;
        vector<char> total_data;
        int packet_num = 0;
        bool is_success = 1;
        while (recv(sockfd, buf, 1000, 0) > 0)
        {
            vector<char> wire2(buf, buf + strlen(buf));
            string wired_string2(wire2.begin(), wire2.end());

            http_response my_response;

            if (packet_num == 0)
            {
                my_response.decode(wire2);

                cout << "getting response with status_code: " <<  my_response.get_status_code() << endl;

                if (my_response.get_status_code() == "404")
                {
                    is_success = 0;
                    cerr << "Not Found" << endl;
                    break;
                }

                if (my_response.get_status_code() == "400")
                {
                    is_success = 0;
                    cerr << "Bad Request" << endl;
                    break;
                }

                std::vector<char> received_data = my_response.get_data();
                total_data.insert(total_data.end(), received_data.begin(), received_data.end());
            }
            else
                total_data.insert(total_data.end(), wire2.begin(), wire2.end());

            packet_num++;
            memset(buf, '\0', sizeof(buf));
        }

        if (is_success == 1)
        {
            string file_name = dir.substr(1);
            cout << "downloading file to ./" << file_name << endl;
            ofstream downloaded_file(file_name, ios::out | ios::binary);
            copy(total_data.begin(), total_data.end(), ostreambuf_iterator<char>(downloaded_file));
            cout << "download successful" << endl;
        }
        close(sockfd);
    }
    return 0;
}
