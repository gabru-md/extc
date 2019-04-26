#ifndef WS_UTIL
#define WS_UTIL

#include <string>

// function to make a compatible URI
std::string make_uri(std::string);

std::string make_uri(std::string uri) {
    const std::string port = "9002";
    if(uri.substr(0,5) == "ws://") 
        return uri;
    if(uri.substr(uri.length() - 4, uri.length()) == port)
        return "ws://" + uri;
    
    return "ws://" + uri + ":" + port;
}

#endif