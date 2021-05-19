/*
 * Toy TCP server demonstrating socket leak.
 *
 * */

#include <arpa/inet.h>
#include <iostream>
#include <netinet/ip.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_set>

std::unordered_set<int> leaked_sockets;

void upon_sighup(int) {
  std::cout << "Received SIGHUP. Cleaning up..." << std::endl;
  for (auto it = leaked_sockets.begin(); it != leaked_sockets.end();) {
    auto sock = *it;
    auto close_res = close(sock);
    auto err_src = close_res >= 0 ? close_res : errno;
    std::cout << "Closing socket " << sock << ": " << strerror(err_src)
              << std::endl;
    it = leaked_sockets.erase(it);
  }
  std::cout << "Clean up complete" << std::endl;
}

void setup() {
  std::cout << "PID: " << getpid() << std::endl;
  signal(SIGHUP, &upon_sighup);
}

int main() {
  setup();

  sockaddr_in server_addy = {.sin_family = AF_INET, .sin_port = htons(9001)};
  inet_aton("127.0.0.1", &server_addy.sin_addr);

  auto addy = reinterpret_cast<sockaddr *>(&server_addy);
  uint32_t size = sizeof(sockaddr_in);

  int s = socket(AF_INET, SOCK_STREAM, 0);
  bind(s, addy, size);
  listen(s, -1);

  do {
    int client = accept(s, addy, &size);
    if (client > 0) {
      leaked_sockets.emplace(client);
      std::cout << "Accepted client " << client << std::endl;
    } else {
      std::cout << "Cannot accept client: " << strerror(errno) << std::endl;
    }
  } while (1);

  return 0;
}
