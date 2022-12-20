

#include "../headers/lang.hpp"


int main (void) {

    //AKINATOR START


    lang lang = {};
    lang_CTOR (&lang);


    Flang_GRAPHDUMP (&lang, "xuyovoe lang");


    lang_read (&lang);


    Flang_GRAPHDUMP (&lang, "norm lang");


    return 0;
}