#ifndef WS_MESSAGE
#define WS_MESSAGE

#include <string>
#include <vector>

struct ws_message {
    std::string sender;
    std::string receiver;
    std::string message;
};

std::string serialize_ws_message(struct ws_message * message) {
    return message->sender + ">" + message->receiver + ">" + message->message;
}

std::string form_message(struct ws_message * msg) {
    return msg->sender + " : " + msg->message;
}

std::string form_sender_message(struct ws_message * msg) {
    return msg->receiver + " : " + msg->message;
}

ws_message * deserialize_ws_message(std::string message) {
    struct ws_message * msg = new ws_message;
    std::string str = "";
    int count = 0;
    for(unsigned int i=0;i<message.size();i++) {
        if(message[i] == '>' && count <=2) {
            count++;
            if(count == 1) {
                msg->sender = str;
            }else if (count == 2) {
                msg->receiver = str;
            }        
            str = "";

        }else {
            str = str + message[i];
        }
    }
    msg->message = str;

    return msg;
}

#endif