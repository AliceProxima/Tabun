#ifndef TABUN_API_LIBTABUN_H
#define TABUN_API_LIBTABUN_H
#pragma once
#include <string>
#include <vector>

class Comment {

public:
    std::string author = "";
    std::string body = "";
    int id = 0;
    std::string date = "";
    int parent_id = 0;
    int post_id = 0;
    
    std::string format(void);
};

class Comments {

private:
    std::vector<Comment> comms;

public:
    Comments(std::vector<Comment> comms) {
        this->comms = comms;
    }
    Comment& operator[](int index) {
        return comms[index];
    }
    const Comment& byId(const unsigned int id) const {
        for (int i = 0; i < this->comms.size(); i ++) {
            if (this->comms[i].id == id) return this->comms[i];
        }
    }
    int size(void) {return this->comms.size();}
};

class UserInfo {
public:
    double strength = 0;
    double rating = 0;
    unsigned int votes = 0;
    std::string description;
    std::vector<std::string> blogs;
    std::string username;
};

class User {

private:
    std::string host_url = "https://tabun.everypony.ru/";
    std::string session_id = "";
    std::string key = "";
    std::string security_ls_key = "";

    void request(std::string, std::string, std::string);
    std::string getCookies(void);

public:
    User(void);
    void login(std::string, std::string);
    Comments getComments(unsigned int);
    Comments getComments(void);
    std::string urlOpen(std::string);
    int addComment(std::string, unsigned int, unsigned int);
    UserInfo getUserInfo(std::string);
    void setSessionId(std::string);
    void setKey(std::string);
    void setSecurityLsKey(std::string);
};

#endif
