#include <string>
#include <zmqpp/zmqpp.hpp>

int main() {
  zmqpp::context ctx;
  zmqpp::socket sock(ctx, zmqpp::socket_type::push);

  sock.connect("tcp://127.0.0.1:5000");

  std::string payload(1024, 'A');

  while (true) {
    zmqpp::message msg;
    msg << payload;
    sock.send(msg);
  }

  return 0;
}
