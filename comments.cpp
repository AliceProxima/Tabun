#include "libtabun.h"
#include <htmlcxx/html/ParserDom.h>
#include <regex>

std::string Comment::format(void) {
    std::string res = "\nComment: \n" 
                 "id: " + std::to_string(this->id) + "\n" +
                 "author: " + this->author + "\n" +
                 this->body + "\n" + 
                 "parent_id: " + std::to_string(this->parent_id) + "\n" +
                 "datetime: " + this->date + "\n" +
                 "post_id: " + std::to_string(this->post_id) + "\n" +
                 "\n" + "--------------\n";
    return res;
}
