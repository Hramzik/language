

#include "../headers/front.hpp"


int main (int argc, char** argv) {

    if (argc >  2) { printf ("enter correct code file name while starting the program"); return 0; }


    Return_code code = SUCCESS;


    if (argc == 2) {

        char file_name [MAX_FILE_NAME_LEN + 1]= "work/"; strcat (file_name, argv [1]); strcat (file_name, ".txt"); printf ("(%s)", file_name);
        code = front_end (file_name);
    }

    else {

        code = front_end ();
    }


    printf ("return code - %d\n", code);


    printf ("goodbye!\n");


    return 0;
}