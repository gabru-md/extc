#ifndef ARGPARSE
#define ARGPARSE

#include <string>
#include <vector>


std::string char_ptr_to_string(char *);

class argparse {
    private:
        std::string help;
        std::vector<std::pair<std::string, int> > args;
    public:
        argparse(std::string);
        void add_argument(std::string, int);
        std::vector<std::string> parse(int, char **);
};

argparse::argparse(std::string desc):help(desc)
{}

void argparse::add_argument(std::string arg, int vals) {
    args.push_back(std::make_pair(arg, vals));
}

std::vector<std::string> argparse::parse(int argc, char **argv) {
    std::vector<std::string> char_to_str;
    for(unsigned int i=1; i<argc; i++) {
        char_to_str.push_back(char_ptr_to_string(argv[i]));
    }

    std::pair<std::string, int> p("error in parsing", -101);
    for(unsigned int i=0; i<args.size(); i++) {
        if(args[i].first == char_to_str[0]) {
            p.first = args[i].first;
            p.second = args[i].second;
        }
    }
    if(p.second == -101) {
        std::cout << p.first << std::endl;
        exit;
    }

    std::vector<std::string> res;
    for(unsigned int i=0; i<=p.second; i++) {
        res.push_back(char_to_str[i]);
    }
    std::string remaining = "";
    for(unsigned int i=p.second+1; i<argc; i++) {
        remaining = remaining + " " + char_to_str[i];
    }
    std::cout<<"remaining: "<<remaining<<std::endl;
    res.push_back(remaining.substr(1, remaining.length()));
    return res;
}

std::string char_ptr_to_string(char * str) {
    size_t len = 0;
    while(str[len] != '\0') {
        len++;
    }

    return std::string(str, len);
}

#endif