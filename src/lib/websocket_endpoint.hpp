#ifndef WEBSOCKET_ENDPOINT
#define WEBSOCKET_ENDPOINT

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include "connection_metadata.hpp"
#include "ws_util.hpp"
#include "ws_message.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> client;

class websocket_endpoint {
    private:
        typedef std::map<int, connection_metadata::ptr> con_list;

        client m_endpoint;
        websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

        con_list m_connection_list;
        int m_next_id;
    
    public:
        // constructor
        websocket_endpoint();
        // destructor
        ~websocket_endpoint();
        // connection function
        int connect(std::string uri, std::string user);
        // close function
        void close(int, websocketpp::close::status::value, std::string);
        // function to send messages
        void send(int id, ws_message * message);
        // get metadata
        const connection_metadata::ptr get_metadata(int);
        // registers username onto the server
        void register_username(int);
};

websocket_endpoint::websocket_endpoint()
    : m_next_id(0) {
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

        m_endpoint.init_asio();
        m_endpoint.start_perpetual();

        m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
}

websocket_endpoint::~websocket_endpoint() {
    m_endpoint.stop_perpetual();

    for(con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
        if (it->second->get_status() != "Open") {
            continue;
        }

        std::cout << "> Closing connection " << it->second->get_id() << std::endl;
        websocketpp::lib::error_code ec;
        m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
        if (ec) {
            std::cout << "> Error closing connection " << it->second->get_id() << ": "  
                        << ec.message() << std::endl;
        }
    }
        
    m_thread->join();
}

int websocket_endpoint::connect(std::string uri, std::string user) {
    websocketpp::lib::error_code ec;
    uri = make_uri(uri);
    client::connection_ptr con = m_endpoint.get_connection(uri, ec);

    if(ec) {
        std::cout << "> Connection initialization error: " << ec.message() << std::endl;
        return -1;
    }

    int new_id = m_next_id++;
    connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(new_id, user, con->get_handle(), uri);
    m_connection_list[new_id] = metadata_ptr;

    con->set_open_handler(websocketpp::lib::bind(
        &connection_metadata::on_open,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));

    con->set_fail_handler(websocketpp::lib::bind(
        &connection_metadata::on_fail,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));

    con->set_close_handler(websocketpp::lib::bind(
        &connection_metadata::on_close,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));

    con->set_message_handler(websocketpp::lib::bind(
        &connection_metadata::on_message,
        metadata_ptr,
        websocketpp::lib::placeholders::_1,
        websocketpp::lib::placeholders::_2
    ));

    m_endpoint.connect(con);
    connection_metadata::ptr metadata = get_metadata(new_id);
    std::cout << *metadata << std::endl;
    while(metadata->get_status() != "Open") {
        metadata = get_metadata(new_id);
        if(metadata->get_status() == "Failed") {
            return new_id;
        }
    }
    metadata = get_metadata(new_id);
    std::cout << *metadata << std::endl;
    register_username(new_id);

    return new_id;
}

void websocket_endpoint::close(int id, websocketpp::close::status::value code, std::string reason) {
    websocketpp::lib::error_code ec;
    
    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        std::cout << "> No connection found with id " << id << std::endl;
        return;
    }
    
    m_endpoint.close(metadata_it->second->get_hdl(), code, reason, ec);
    if (ec) {
        std::cout << "> Error initiating close: " << ec.message() << std::endl;
    }
}

void websocket_endpoint::send(int id, ws_message * message) {
    websocketpp::lib::error_code ec;

    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        std::cout << "> No connection found with id " << id << std::endl;
        return;
    }

    m_endpoint.send(metadata_it->second->get_hdl(), serialize_ws_message(message), websocketpp::frame::opcode::text, ec);
    
    if (ec) {
        std::cout << "> Error sending message: " << ec.message() << std::endl;
        return;
    }
    
    metadata_it->second->record_sent_message(form_sender_message(message));
}

void websocket_endpoint::register_username(int id) {
    connection_metadata::ptr metadata = get_metadata(id);
    std::string username = metadata->get_user();

    websocketpp::lib::error_code ec;
    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        std::cout << "> No connection found with id " << id << std::endl;
        return;
    }
    std::string reg_user = "_register: " + username;

    m_endpoint.send(metadata_it->second->get_hdl(), reg_user, websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cout << "> Error sending message: " << ec.message() << std::endl;
        return;
    }
}

const connection_metadata::ptr websocket_endpoint::get_metadata(int id) {
    con_list::const_iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        return connection_metadata::ptr();
    } else {
        return metadata_it->second;
    }
}


#endif