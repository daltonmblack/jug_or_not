#include <csignal>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#define BACKLOG_SIZE 2

void signal_handler(int sig_num) {
  std::cout << "User killed the proxy." << std::endl;
  // TODO: make this cleaner later. For now we just don't want to get
  //       stuck because of the second thread running.
  exit(sig_num);
}

typedef struct thread_helper {
  int sock_fd;
} thread_helper_t;

void *accept_connections(void *helper_void) {
  thread_helper_t *helper = (thread_helper_t *) helper_void;
  int sock_fd = helper->sock_fd;
  free(helper);

  struct sockaddr_un client_addr;
  socklen_t client_addr_size = sizeof(struct sockaddr_un);
  int client_fd = accept(sock_fd, (struct sockaddr *) &client_addr,
			 &client_addr_size);
  if (client_fd < 0) {
    // TODO: figure out what to do here. This is bad.
    std::cerr << "accept() failed with error: " << strerror(errno) << std::endl;
  }

  return NULL;
}

class Proxy {
 public:
  Proxy(std::string port_num) : port_num(port_num) {}

  bool start() {
    int res;
    struct addrinfo hints, *addr_res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Fill in IP address in 'getaddrinfo()'
    getaddrinfo(NULL, "1234", &hints, &addr_res);

    sock_fd = socket(addr_res->ai_family, addr_res->ai_socktype,
			 addr_res->ai_protocol);
    if (sock_fd < 0) {
      // TODO: come up with error-handling framework.
      std::cerr << "socket() failed in Proxy::initialize() with error: " << strerror(errno) << std::endl;
      return false;
    }

    res = bind(sock_fd, addr_res->ai_addr, addr_res->ai_addrlen);
    if (res) {
      // TODO: come up with error-handling framework.
      std::cerr << "bind() failed in Proxy::initialize() with error: " << strerror(errno) << std::endl;
      return false;
    }

    if (listen(sock_fd, BACKLOG_SIZE)) {
      // TODO: this be bad, figure this out.
      std::cerr << "listen() failed in Proxy::initialize() with error: " << strerror(errno) << std::endl;
      return false;
    }

    thread_helper_t *helper = (thread_helper_t *) malloc(sizeof(thread_helper_t));
    helper->sock_fd = sock_fd;
    if (pthread_create(&accept_thread, NULL, accept_connections, helper)) {
      std::cerr << "pthread_create() failed in Proxy::initialize() with error: " << strerror(errno) << std::endl;
      return false;
    }

    if (pthread_join(accept_thread, NULL)) {
      std::cerr << "pthread_join() failed in Proxy::initialize() with error: " << strerror(errno) << std::endl;
      return false;
    }

    return true;
  }

 private:
  std::string port_num;
  int sock_fd;
  pthread_t accept_thread;
};

int main(int argc, char **argv) {
  // Register the signal handler to catch '^C'.
  signal(SIGINT, signal_handler);

  std::cout << "Hello, world!" << std::endl;

  Proxy prox("1234");
  if (!prox.start()) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
