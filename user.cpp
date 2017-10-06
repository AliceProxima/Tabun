#include "libtabun.h"
#include <curl/curl.h>
#include <htmlcxx/html/ParserDom.h>
#include <regex>

#include <iostream>


static char error_buffer[CURL_ERROR_SIZE];
static std::string buffer;
static std::string header_buffer;

static int write(char *data, size_t size, size_t nmemb, std::string *buffer) {
  int result = 0;
  if (buffer != NULL) {
    buffer->append(data, size * nmemb);
    result = int(size * nmemb);
  }
  return result;
}

static int writeHeader(char *data, size_t size, size_t nmemb, std::string *header_buffer) {
  int result = 0;
  if (header_buffer != NULL) {
    header_buffer->append(data, size * nmemb);
    result = int (size * nmemb);
  }
  return result;
}


std::string User::getCookies(void) {
    std::string cookies = "";
    if (this->session_id != "") {
        cookies += "TABUNSESSIONID=" + this->session_id + ";";
    }

    if (this->key != "") {
        cookies += "key=" + this->key + ";";
    }

    return cookies;
}

void User::request(std::string url, std::string cookies, std::string request) {

    buffer = "";
    header_buffer = "";

    CURL *curl;
    CURLcode res;

    char const *c_url = url.c_str();

    curl = curl_easy_init();
    if (!curl) return;

    curl_easy_setopt(curl, CURLOPT_URL, c_url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeader);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER,    &header_buffer);

    if (cookies != "") {
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    }

    if (request != "") {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.length());
    }

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

    curl_easy_cleanup(curl);
}

std::string User::urlOpen(std::string url) {
    this->request(url, this->getCookies(), "");
    return buffer;
}

void User::login(std::string username, std::string passwd) {
    std::string url;
    url = this->host_url + "login/ajax-login";

    std::string request;
    request = "login=" + username + "&";
    request += "password=" + passwd + "&";
    request += "return-path=https%3A%2F%2Ftabun.everypony.ru%2Flogin&remember=on&security_ls_key=";
    request += this->security_ls_key;

    this->request(url, this->getCookies(), request);

    std::string s = header_buffer;

    std::regex re("key=.*; e");
    std::regex re2("; e");
    std::regex re3("key=");

    const std::string format = "";
    std::smatch match;
    std::string res;

    std::regex_search(s, match, re);
    res = match[0];
    res = std::regex_replace(res, re2, format, std::regex_constants::format_default);
    res = std::regex_replace(res, re3, format, std::regex_constants::format_default);

    this->key = res;
}

User::User(void) {
    this->request(this->host_url, "", "");
    std::string headers = header_buffer;
    std::string s_key = buffer;

    std::regex re("LIVESTREET_SECURITY_KEY = '.*';");
    std::regex re2("LIVESTREET_SECURITY_KEY = '");
    std::regex re3("';");
    std::smatch match;
    std::regex_search(s_key, match, re);
    std::string res = std::regex_replace(match[0].str(), re2, "", std::regex_constants::format_default);
    res = std::regex_replace(res, re3, "", std::regex_constants::format_default);
    this->security_ls_key = res;

    std::regex re_0("TABUNSESSIONID=.*; e");
    std::regex re_2("; e");
    std::regex re_3("TABUNSESSIONID=");

    const std::string format = "";
    std::smatch match2;

    std::regex_search(headers, match, re_0);
    res = match[0];
    res = regex_replace(res, re_2, format, std::regex_constants::format_default);
    res = regex_replace(res, re_3, format, std::regex_constants::format_default);
    this->session_id = res;
}

Comments User::getComments(unsigned int post_id) {
    std::vector<Comment> res;
    std::string html = this->urlOpen(this->host_url + "blog/" + std::to_string(post_id) + ".html");

    htmlcxx::HTML::ParserDom parser;
    tree<htmlcxx::HTML::Node> dom = parser.parseTree(html);

    tree<htmlcxx::HTML::Node>::iterator it = dom.begin();
    tree<htmlcxx::HTML::Node>::iterator end = dom.end();

    Comment comment;

    for (; it != end; it ++) {
        it->parseAttributes();
        if (it->attribute("class").second == "folding") {
            comment.id = stoi(it->attribute("data-id").second);

        } else if (it->attribute("class").second == "text current") {
            it ++;
            comment.body = "";
            comment.body += it->text();
            while (it->tagName() != "ul") {
                if (it->tagName() != "") {
                    it++;
                    if (it->tagName() == "ul") break;
                    comment.body += it->text();
                    it --;
                    comment.body += it->closingText();
                    it ++;
                } else {
                    comment.body += it->text();
                    it ++;
                }
            }
        } else if (it->tagName() == "li") {
            it->parseAttributes();
            if (it->attribute("class").second == "goto goto-comment-parent") {
                it += 2;
                it->parseAttributes();
                std::string s = it->attribute("href").second;
                std::regex re("[0-9]+");
                std::smatch match;
                std::regex_search(s, match, re);
                res.back().parent_id = stoi(match[0]);
            }
            if (it->attribute("class").second == "comment-author") {
                it += 8;
                comment.author = it->text();
            }
            it += 2;
            it->parseAttributes();
            if (it->attribute("datetime").first) {
                comment.date = it->attribute("datetime").second;
                comment.post_id = post_id;
                res.push_back(comment);
                comment.author = "";
                comment.date = "";
                comment.body = "";
                comment.id = 0;
            }
        } 
    }
    res.erase(res.begin());
    return Comments(res);
}

Comments User::getComments(void) {
    std::vector<Comment> res;
    std::string html = this->urlOpen(this->host_url + "comments");

    htmlcxx::HTML::ParserDom parser;
    tree<htmlcxx::HTML::Node> dom = parser.parseTree(html);

    tree<htmlcxx::HTML::Node>::iterator it = dom.begin();
    tree<htmlcxx::HTML::Node>::iterator end = dom.end();

    Comment comment;
    
    for (; it != end; it ++) {
        
        it->parseAttributes();
        if (it->attribute("class").second == "comment-content") {
            it += 3;
            comment.body = "";
            comment.body += it->text();
            while (it->tagName() != "div") {
                if (it->tagName() != "") {
                    it ++;
                    if (it->tagName() == "div") break;
                    comment.body += it->text();
                    it --;
                    comment.body += it->closingText();
                    it ++;
                } else {
                    comment.body += it->text();
                    it ++;
                }
            }
        } else if (it->attribute("class").second == "comment-author") {
            it += 8;
            comment.author = it->text();
        } else if (it->attribute("datetime").first) {
            comment.date = it->attribute("datetime").second;
        } else if (it->attribute("id").first && it->attribute("class").second == "vote") {
            std::string s = it->attribute("id").second;
            std::regex re("[0-9]+");
            std::smatch match;
            std::regex_search(s, match, re);
            comment.id = stoi(match[0]);
        } else if (it->attribute("class").second == "goto goto-comment-parent") {
            it += 2;
            it->parseAttributes();
            std::string s = it->attribute("href").second;
            std::regex re("[0-9]+");
            std::smatch match;
            std::regex_search(s, match, re);
            comment.parent_id = stoi(match[0]);
        } else if (it->attribute("title").second == "Ссылка на комментарий") {
            std::string s = it->attribute("href").second;
            std::regex re("[0-9]+");
            std::smatch match;
            std::regex_search(s, match, re);
            comment.post_id = stoi(match[0]);
            res.push_back(comment);
            comment.parent_id = 0;
        }
    }
    return Comments(res);
}

int User::addComment(std::string body, unsigned int post_id, unsigned int reply) {
    std::string request = "comment_text=" + body;
    request += "&reply=" + std::to_string(reply);
    request += "&cmt_target_id=" + std::to_string(post_id);
    request += "&security_ls_key=" + this->security_ls_key;
    this->request(this->host_url + "blog/ajaxaddcomment", this->getCookies(), request);
    return 0;
}

UserInfo User::getUserInfo(std::string uname) {
    std::string url = this->host_url + "profile/" + uname;
    std::string html = this->urlOpen(url);
    UserInfo user;
    user.username = uname;

    htmlcxx::HTML::ParserDom parser;
    tree<htmlcxx::HTML::Node> dom = parser.parseTree(html);

    tree<htmlcxx::HTML::Node>::iterator it = dom.begin();
    tree<htmlcxx::HTML::Node>::iterator end = dom.end();

    bool f = false;

    for (; it != end; it ++) {
        if (it->tagName() == "span") {
            it->parseAttributes();
            std::string s;
            if (it->attribute("id").first) {
                it ++;
                s = it->text();
                user.rating = atof(s.c_str());
            }
        }
        if (it->tagName() == "div") {
            it->parseAttributes();
            if (it->attribute("class").second == "vote-label") {
                it ++;
                if (it->text() != "Сила") {
                    std::string s = it->text();
                    user.votes = stoi(std::regex_replace(s, std::regex("голосов: "), "", std::regex_constants::format_default));
                }
            }
        }
        it->parseAttributes();
        if (it->attribute("class").second == "profile-info-about") {
            it ++;
            while (it->tagName() != "div") it ++;
            it ++;
            user.description = it->text();
            while (it->tagName() != "div") {
                if (it->tagName() != "") {
                    it ++;
                    it->parseAttributes();
                    if (it->tagName() == "div" || it->attribute("class").second == "edit") break;
                    user.description += it->text();
                    it --;
                    user.description += it->closingText();
                    it ++;
                } else {
                    user.description += it->text();
                    it ++;
                }
            }
        }
        if (it->text() == "Состоит в:") {
            while (it->tagName() != "a") it++;
            it->parseAttributes();
            if (it->attribute("href").first) user.blogs.push_back(it->attribute("href").second);
            if (it->tagName() == "li") break;
        }
    }
    
    for (int i = 0; i < user.blogs.size(); i ++) {
        std::smatch match;
        user.blogs[i] = std::regex_replace(user.blogs[i], std::regex("https://tabun.everypony.ru/blog/"), "", std::regex_constants::format_default);
        user.blogs[i] = std::regex_replace(user.blogs[i], std::regex("/"), "", std::regex_constants::format_default);
    }

    return user;
}

void User::setKey(std::string new_key) {
    this->key = new_key;
}

void User::setSecurityLsKey(std::string new_ls_key) {
    this->security_ls_key = new_ls_key;
}

void User::setSessionId(std::string new_session_id) {
    this->session_id = new_session_id;
}