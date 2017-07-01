# Proxy server (tested on Linux)

A Web proxy is a program that acts as a middleman between a Web browser and an end server.

Instead of contacting the end server directly to get a Web page, the browser contacts the proxy, 
which forwards the request on to the end server. When the end server replies to the proxy, the
proxy sends the reply on to the browser.

First, the proxy needs to receive a HTTP request from a client. It handles large number of clients
with I/O multiplexing using select () function. So, it is possible that multiple clients will try
to connect to the proxy, and thus the proxy uses multi-threading to handle each client. Second,
when a client sends a HTTP request, the proxy relays the request to a web server to get a content
for the request (technically, the proxy checks whether a content for the request in a local
repository, but I skip it). Third, if the proxy receives the content from a web server, then the
proxy should return this content to a client. Finally, the proxy should keep a log  for each
request of a client.

1. In order to complile the program, please type: make

2. In order to clean all generated files except proxy.log, please type: make clean

3. In order to run the code, please type: ./proxy

4. By default, the proxy will be binded to the port 1234. If you want to specify
any other port that the proxy should be binded to, please provide the port
number as the FIRST option to the compiled program. For example, if you type
./proxy 9876, then the proxy will be binded to the port 9876.

5. The log file proxy.log is created if it does not exist and is cleared every time
the new process of the compiled program is run.
