# Socket Programming

Jason King

Implementation of an HTTP client and server running a simplified version of the HTTP/1.1 protocol.



## To run the HTTP Server:

First, "Make all"

Then, run "http_server"

  Arguments:
  port_number: port on which to start the server

  Ex: ./http_server 9980

  Then, run client with same port_number to contact your server:

  Ex: ./http_client -p ccc.wpi.edu 9980
  Ex: ./http_client -p ccc.wpi.edu/TMDG.html 9980



## To run the HTTP Client:

First, "Make all" if you have not already

Then, run "http_client"

  Arguments:
  server_url: (http://www.server.com/path/to/file) or an IP address
  port_number: port on which to contact the server
  options: (-p) prints the RTT for accessing the URL on the terminal before server’s
  response

  Ex: ./http_client [-options] server_url port_number
  Ex: ./http_client www.google.com 80
  Ex: ./http_client ccc.wpi.edu/TMGD.html 9990
