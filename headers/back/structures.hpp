#ifndef BACK_STRUCTURES_HPP_INCLUDED
#define BACK_STRUCTURES_HPP_INCLUDED


//--------------------------------------------------


typedef struct Var_info {

    char* name;
    size_t ptr;

} Var_info; const size_t VAR_INFO_SIZE = sizeof (Var_info);


typedef struct Name_table {

    Var_info* list;
    size_t num_vars;

    size_t free_ptr;

} Name_table; const size_t NAME_TABLE_SIZE = sizeof (Name_table);


typedef struct Compiler_info {

    size_t num_if;
    size_t num_while;

    Name_table* tables;
    size_t  num_tables;

} Compiler_info; const size_t COMPILER_INFO_SIZE = sizeof (Compiler_info);


//--------------------------------------------------


#endif