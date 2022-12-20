#ifndef FRONT_ENUMS_HPP_INCLUDED
#define FRONT_ENUMS_HPP_INCLUDED


//--------------------------------------------------


enum Word_type {

    LWT_UNKNOWN,
    LWT_NAME,
    LWT_CONST,
    LWT_NVAR,
    LWT_NFUNC,
    LWT_BLOCK_OPEN,
    LWT_BLOCK_CLOSE,
    LWT_ARG_OPEN,
    LWT_ARG_CLOSE,
    LWT_COMMA,
    LWT_SEMICOLON,

    LWT_CALL,
    LWT_OP,
    LWT_ASS,
    LWT_WHILE,
    LWT_IF,
    LWT_ELSE,
    LWT_RET,
    LWT_VAR,
};


//--------------------------------------------------


#endif