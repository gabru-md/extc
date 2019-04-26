#ifndef CONNECTION_METADATA
#define CONNECTION_METADATA

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <string>
#include <vector>
#include <sstream>

typedef websocketpp::client<websocketpp::config::asio_client> client;

class connection_metadata {
    private:
        int m_id;
        std::string m_user;
        websocketpp::connection_hdl m_hdl;
        std::string m_status;
        std::string m_uri;
        std::string m_server;
        std::string m_error_reason;
    public:
        typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;
        std::vector<std::string> m_messages;
        
        // connection_metadata constructor
        connection_metadata(int, std::string, websocketpp::connection_hdl, std::string);
        // fires whenever a connection is opened
        void on_open(client *, websocketpp::connection_hdl);
        // fires whenever fails
        void on_fail(client *, websocketpp::connection_hdl);
        // fires whenever connection closes
        void on_close(client *, websocketpp::connection_hdl);
        // fires on receiving a message
        void on_message(websocketpp::connection_hdl, client::message_ptr);

        // getters
        websocketpp::connection_hdl get_hdl();
        const int get_id();
        const std::string get_user();
        const std::string get_status();
        void record_sent_message(std::string);
        void get_all_messages();
        std::string get_recent_message();

        friend std::ostream & operator<< (std::ostream & out, connection_metadata const & data);
};

// overloading the << operator to display data
std::ostream & operator<< (std::ostream & out, connection_metadata const & data) {
    out << "> URI: " << data.m_uri << "\n"
        << "> Username: " << data.m_user << "\n"
        << "> Status: " << data.m_status << "\n"
        << "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
        << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason) << "\n";
    out << "> Messages Processed: (" << data.m_messages.size() << ") \n";

    std::vector<std::string>::const_iterator it;
    for(it = data.m_messages.begin(); it != data.m_messages.end(); ++it) {
        out << *it << "\n";
    }

    return out;
}

// constructor for connection_metadata
connection_metadata::connection_metadata (int id, std::string user, websocketpp::connection_hdl hdl, std::string uri)
    : m_id(id)
    , m_hdl(hdl)
    , m_user(user)
    , m_status("Connecting")
    , m_uri(uri)
    , m_server("N/A")
    {}

// gets fired when connection is opened
void connection_metadata::on_open(client * c, websocketpp::connection_hdl hdl) {
    m_status = "Open";

    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
}

// gets fired if connection fails
void connection_metadata::on_fail(client * c, websocketpp::connection_hdl hdl) {
    m_status = "Failed";

    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
    m_error_reason = con->get_ec().message();
}

// gets called when the connection closes
void connection_metadata::on_close(client * c, websocketpp::connection_hdl hdl) {
    m_status = "Closed";

    client::connection_ptr con = c->get_con_from_hdl(hdl);
    std::stringstream s;

    s << "close code: " << con->get_remote_close_code() << " (" 
          << websocketpp::close::status::get_string(con->get_remote_close_code()) 
          << "), close reason: " << con->get_remote_close_reason();
    m_error_reason = s.str();
}

// gets fired when a message is received
void connection_metadata::on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
    if(msg->get_opcode() == websocketpp::frame::opcode::text) {
        m_messages.push_back("<< " + msg->get_payload());
    }else {
        m_messages.push_back("<< " + websocketpp::utility::to_hex(msg->get_payload()));
    }
}

void connection_metadata::get_all_messages() {
    std::vector<std::string>::const_iterator it;
    for(it = m_messages.begin(); it != m_messages.end(); ++it) {
        std::cout << *it << "\n";
    }
}

std::string connection_metadata::get_recent_message() {
    std::vector<std::string>::const_iterator it;
    it = m_messages.end();
    return *(--it);
}

websocketpp::connection_hdl connection_metadata::get_hdl() {
    return m_hdl;
}

const int connection_metadata::get_id() {
    return m_id;
}

const std::string connection_metadata::get_status() {
    return m_status;
}

const std::string connection_metadata::get_user() {
    return m_user;
}

void connection_metadata::record_sent_message(std::string message) {
    m_messages.push_back(">> " + message);
}


#endif