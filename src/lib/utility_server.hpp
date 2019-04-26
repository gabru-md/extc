#ifndef UTILITY_SERVER
#define UTILITY_SERVER

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <functional>

#include "ws_message.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

class utility_server {
    private:
        server m_endpoint;
        std::map<std::string, websocketpp::connection_hdl> user_map;
    public:
        // constructor
        utility_server();
        // message_handler
        void message_handler(websocketpp::connection_hdl, server::message_ptr);
        // run server
        void run();
        // open handler
        void open_conn_handler(websocketpp::connection_hdl hdl);
        // register users
        int register_username(websocketpp::connection_hdl, std::string);
        // get hdl from user_map by searching value from (key, value)
        websocketpp::connection_hdl get_recv_hdl(ws_message *);
};

utility_server::utility_server() {
    m_endpoint.set_error_channels(websocketpp::log::elevel::all);
    m_endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

    m_endpoint.init_asio();
    m_endpoint.set_message_handler(std::bind(
        &utility_server::message_handler, this,
        std::placeholders::_1, std::placeholders::_2
    ));

    m_endpoint.set_open_handler(websocketpp::lib::bind(
        &utility_server::open_conn_handler, this,
        websocketpp::lib::placeholders::_1
    ));
}

void utility_server::run() {
    m_endpoint.listen(9002);
    m_endpoint.start_accept();
    m_endpoint.run();
}

void utility_server::message_handler(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    std::string inc_msg = msg->get_payload();
    if(inc_msg.substr(0, 10) == "_register:") {
        std::cout << "register in progress\n";
        std::string username = inc_msg.substr(11, inc_msg.length());
        if(register_username(hdl, username) == 0) {
            std::cout << "registration successful" << std::endl;
        }else {
            std::cout << "registration failed" << std::endl;
        }
    }else{
        struct ws_message * ds_msg = new ws_message;
        ds_msg = deserialize_ws_message(msg->get_payload());
        websocketpp::connection_hdl recv_hdl = get_recv_hdl(ds_msg);
        std::cout << "receiver is: "<<ds_msg->receiver<<std::endl;
        m_endpoint.send(recv_hdl, form_message(ds_msg), msg->get_opcode());
    }
}

websocketpp::connection_hdl utility_server::get_recv_hdl(ws_message * msg) {
    std::string receiver = msg->receiver;
    std::map<std::string, websocketpp::connection_hdl>::iterator m_it = user_map.find(receiver);
    if(m_it != user_map.end()) {
        // user has already registered and has been found
        return m_it->second;
    }else {
        m_it = user_map.find(msg->sender);
        return m_it->second;
    }
}

int utility_server::register_username(websocketpp::connection_hdl hdl, std::string username) {
    std::map<std::string, websocketpp::connection_hdl>::iterator m_it = user_map.find(username);
    if(m_it != user_map.end()) {
        return 1;
    }
    // register user here
    user_map.insert(make_pair(username, hdl));
    return 0;
}

void utility_server::open_conn_handler(websocketpp::connection_hdl hdl) {
    std::cout << "wow" <<  std::endl;
}

#endif