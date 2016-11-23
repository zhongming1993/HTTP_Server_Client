# CS118 Project 1 

Team Members:
Ming Zhong: 404593326
Yingda Lin: 904590033

Here are the components of the project

1. Makefile

This provides a couple make targets for things.
By default (all target), it makes the `web-server` and `web-client` executables.

It provides a `clean` target, and `tarball` target to create the submission file as well.

You will need to modify the `Makefile` to add your userid for the `.tar.gz` turn-in at the top of the file.

2. http_message.h

This provides the definition of http_message(base class), http_request and http_response

3. http_message.cpp

This provides the implementation of the three classes in http_message.h

4. web-client.cpp

This provides the implementation of a TCP/IP client, which communicate with web-server by HTTP protocol with the headers defined in http_message.h

5. web-server.cpp

This provides the implementation of a multithreaded TCP/IP server, which communicate the web-client by HTTP protocol with the headers defined in http_message.h

6. web-server

binary executable file for the web server

7. web-client

binary executable file for the web client

