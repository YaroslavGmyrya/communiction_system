#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>

using namespace std;

int main(int argc, char *argv[]) {
  const string endpoint = "tcp://*:5000";

  // initialize the 0MQ context
  zmqpp::context context;

  // generate a pull socket
  zmqpp::socket_type type = zmqpp::socket_type::pull;
  zmqpp::socket socket(context, type);

  // bind to the socket
  cout << "Binding to " << endpoint << "..." << endl;
  socket.bind(endpoint);

  // receive the message
  cout << "Receiving message..." << endl;
  while (true) {
    zmqpp::message message;
    // decompose the message
    socket.receive(message);
    std::string text;
    message >> text;
    std::cout << "Received: " << text << "\n";
  }

  return 0;
}