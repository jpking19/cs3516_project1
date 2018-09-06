# cs3516_project1
**Socket Programming**

First project in CS3516 Computer Networks

Implementation of an HTTP client and server running a simplified
version of the HTTP/1.1 protocol.

To run the HTTP client:

server_url: (http://www.server.com/path/to/file) or an IP address
port_number: which port on which to contact the server
options: (-p) prints the RTT for accessing the URL on the terminal before server’s
response

Ex: ./http_client [-options] server_url port_number 
Ex: ./http_client www.google.com 80 
Ex: ./http_client cccworks4.wpi.edu/index.html 7890 

To run the HTTP Server:

Should start before the client.

