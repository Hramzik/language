#ifndef FRONT_STRUCTURES_HPP_INCLUDED
#define FRONT_STRUCTURES_HPP_INCLUDED


//--------------------------------------------------


typedef struct Word {

    char*     str;
    size_t    len;

    Word_type type;
    Element   val;

} Word; const size_t WORD_SIZE = sizeof (Word);


typedef struct Words {

    Word* list;
    size_t len;

} Words; const size_t WORDS_SIZE = sizeof (Words);


typedef struct Function_list {

    char** list;
    size_t len;

} Function_list; const size_t FUNCTION_LIST_SIZE = sizeof (Function_list);


typedef struct Variable_list {

    char** list;
    size_t len;

} Variable_list; const size_t VARIABLE_LIST_SIZE = sizeof (Variable_list);


//--------------------------------------------------


#endif