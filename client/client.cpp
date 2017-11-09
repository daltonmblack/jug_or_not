#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

class Client {
public:
  Client(const char *proxy_ip, const char *proxy_port) :
    proxy_ip(proxy_ip), proxy_port(proxy_port) {}

  bool start() {
    int res;
    struct addrinfo server_hints, *server_addr;
    memset(&server_hints, 0, sizeof server_hints);
    server_hints.ai_family = AF_INET;
    server_hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(proxy_ip.c_str(), proxy_port.c_str(), &server_hints, &server_addr);

    sock_fd = socket(server_addr->ai_family, server_addr->ai_socktype,
		     server_addr->ai_protocol);
    if (sock_fd < 0) {
      // TODO: come up with error-handling framework.
      cerr << "socket() failed in Client::start() with error: " << strerror(errno) << endl;
      return false;
    }

    res = connect(sock_fd, server_addr->ai_addr, server_addr->ai_addrlen);
    if (res) {
      // TODO: come up with error-handling framework.
      cerr << "connect() failed in Client::start() with error: " << strerror(errno) << endl;
      return false;
    }

    cout << "Successfully connected to the server!" << endl;

    // Collect URL request from the user.
    // TODO: eventually make this functionality accessible via a '-i' (interactive) flag.
    string url;
    cout << "Enter URL: ";
    cin >> url;

    // Send user's request to the proxy.
    res = send(sock_fd, url.c_str(), url.size(), 0);
    if (res < 0) {
      cerr << "send() failed in Client::start() with error: " << strerror(errno) << endl;
      return false;
    }

    // Wait for confirmation from the proxy.
    size_t BUF_LEN = 100;
    char buf[BUF_LEN];
    res = recv(sock_fd, buf, BUF_LEN, 0);
    if (res < 0) {
      cerr << "recv() failed in Client::start() with error: " << strerror(errno) << endl;
      return false;
    } else if (res == 0) {
      cout << "Proxy has closed the connection" << endl;
    } else {
      buf[res] = '\0';
      cout << "Proxy responded with: " << buf << endl;
    }

    close(sock_fd);

    return true;
  }

private:
  string proxy_ip;
  string proxy_port;
  int sock_fd;
};

int main(int argc, char **argv) {
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " <proxy_ip> <proxy_port>" << endl;
    exit(EXIT_FAILURE);
  }

  // Parse command line arguments.
  char *proxy_ip = argv[1];
  char *proxy_port = argv[2];

  Client cli(proxy_ip, proxy_port);
  if (!cli.start()) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
