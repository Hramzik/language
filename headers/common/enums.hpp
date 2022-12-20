#ifndef COMMON_ENUMS_HPP_INCLUDED
#define COMMON_ENUMS_HPP_INCLUDED


//--------------------------------------------------


enum Atom_type {

    LAT_DEFS,
    LAT_NVAR,
    LAT_NFUNC,
    LAT_BLOCK,
    LAT_ARG,
    LAT_CALL,
    LAT_PAR,
    LAT_OP,
    LAT_SEQ,
    LAT_ASS,
    LAT_WHILE,
    LAT_IF,
    LAT_BRANCH,
    LAT_RET,
    LAT_CONST,
    LAT_VAR,
};


//--------------------------------------------------


enum Op_code {

    LOC_ADD,
    LOC_SUB,
    LOC_MULT,
    LOC_DIV,
    LOC_AND,
    LOC_OR,
    LOC_LT,
    LOC_GRT,
    LOC_LEQ,
    LOC_GREQ,
    LOC_EQ,
    LOC_NEQ,
};


//--------------------------------------------------


#endif