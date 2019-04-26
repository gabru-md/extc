#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include "lib/websocket_endpoint.hpp"
#include "lib/connection_metadata.hpp"
#include "lib/argparse.hpp"
#include "lib/ws_message.hpp"

#include <string>
#include <iostream>

int main(int argc, char **argv) {
    argparse * parser = new argparse("help");
    parser->add_argument("connect", 1); // connect <uri> ; if no uri then scan for all
    parser->add_argument("help", 0);
    parser->add_argument("send", 1); // send <username> _message_ with default connection id = 0
    parser->add_argument("show", 1); // show <con_id>
    parser->add_argument("close", 0); // close
    parser->add_argument("quit", 0); // quit

    websocket_endpoint endpoint;

    int id = endpoint.connect("ws://127.0.0.1:9002","user1");
    std::cout << id;
    struct ws_message * msg = new ws_message;
    msg->sender = "user1";
    msg->receiver = "user2";
    msg->message = "wow this is awesome";
    int temp;
    std::cin>>temp;
    endpoint.send(id, msg);
    connection_metadata::ptr metadata = endpoint.get_metadata(id);
    if (metadata) {
        std::cout << *metadata << std::endl;
    } else {
        std::cout << "> Unknown connection id " << id << std::endl;
    }


    return 0;
}