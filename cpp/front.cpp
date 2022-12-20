

//--------------------------------------------------


#include "../headers/front.hpp"

#include "../lib/logs.hpp"
#include "../lib/onegin.hpp"
#include "../lib/tree_lang.hpp"


//--------------------------------------------------


Return_code  read_word_list  (FILE* file, Words* words) {

    if (!file || !words) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    size_t file_len = get_file_len (file);
    char buff [file_len + 1] = ""; buff [file_len] = '\0';
    fread (buff, 1, file_len, file);



    bool in_word = false; int word = -1;

    for (size_t i = 0; buff [i] != '\0'; i++) {

        if (!in_word) {

            if (is_space   (buff [i])) continue;


            if (is_special (buff [i])) {

                word += 1;
                words->list [word].str [0] = buff [i]; words->list [word].str [1] = '\0';
                words->list [word].len = 1;

                continue;
            }


            in_word = true;
            word += 1;
            words->list [word].str [0] = buff [i];
            words->list [word].len = 1;

            continue;
        }

        if (!is_space (buff [i]) && !is_special (buff [i])) { //продолжение слова

            words->list [word].str [words->list [word].len] =  buff [i];
            words->list [word].len                          += 1;

            continue;
        }

        words->list [word].str [words->list [word].len] = '\0';
        in_word = false;

        if (is_special (buff [i])) i -= 1; //еще раз обработать символ
    }


    words->len = word + 1;


    return SUCCESS;
}


Return_code  front_end  (const char* code_file_name, const char* tree_file_name) {

    if (!code_file_name || !tree_file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    FILE* code_file = fopen (code_file_name, "r");
    FILE* tree_file = fopen (tree_file_name, "w");

    Words words = {}; words_ctor (&words, MAX_NUM_WORDS);
    Tree  tree  = {}; TREE_CTOR  (&tree);


    try (read_word_list      (code_file,        &words));
    try (analyse_words                         (&words));
    try (analyse_calls_and_vars                (&words));
    try (build_tree                     (&tree, &words));
    try (STANDART_write_tree (tree_file, &tree));


    //FTREE_GRAPHDUMP (&tree);


    /*for (size_t i = 0; i < words.len; i++) {
        printf ("%zd - \"%s\", type - %s\n", i, words.list [i].str, word_type_to_str (words.list [i].type));
    }*/


    return SUCCESS;
}


Return_code  words_ctor  (Words* words, size_t len) {

    if (!words) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    words->len = 0;
    words->list = (Word*) calloc (WORD_SIZE * len, 1);

    for (size_t i = 0; i < len; i++) {
    
        words->list [i].len  = 0;
        words->list [i].str  = (char*) calloc (MAX_WORD_LEN + 1, 1); words->list [i].str [0] = '\0';
        words->list [i].type = LWT_UNKNOWN;
        words->list [i].val  = { .value = {.val_str = nullptr}, .poisoned = true };
    }


    return SUCCESS;
}


bool  is_special  (char c) { //one of the following: "(){};,"

    if (c == '(') return true;
    if (c == ')') return true;
    if (c == '{') return true;
    if (c == '}') return true;
    if (c == ';') return true;
    if (c == ',') return true;


    return false;
}


bool is_start_digitlike  (char* str) {

    if (isdigit (str [0]))                       return true;
    if ((str [0] == '-') && (isdigit (str [1]))) return true;


    return false;
}


Return_code  analyse_words  (Words* words) {

    if (!words) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < words->len; i++) {

        if (is_special         (words->list [i].str [0])) { try (analyse_special (&words->list [i])); continue; }

        if (is_start_digitlike (words->list [i].str))     { try (analyse_const   (&words->list [i])); continue; }

        try (analyse_name (&words->list [i]));
    }


    return SUCCESS;
}


Return_code  analyse_special  (Word* word) {

    if (!word)                       { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (!is_special (word->str [0])) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    switch (word->str [0]) {

        case '(': word->type = LWT_ARG_OPEN; break;
        case ')': word->type = LWT_ARG_CLOSE; break;
        case '{': word->type = LWT_BLOCK_OPEN; break;
        case '}': word->type = LWT_BLOCK_CLOSE; break;
        case ';': word->type = LWT_SEMICOLON; break;
        case ',': word->type = LWT_COMMA; break;

        default: LOG_ERROR (CRITICAL); return CRITICAL;
    }


    return SUCCESS;
}


Return_code  analyse_const  (Word* word) {

    if (!word)                           { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (!is_start_digitlike (word->str)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    char* good_end_ptr = word->str + word->len;
    char* end_ptr = nullptr;

    double value = strtod (word->str, &end_ptr);


    if (end_ptr != good_end_ptr) { LOG_MESSAGE ("bad const value!\n"); return BAD_ARGS; }


    word->type = LWT_CONST;
    word->val =  { .value = {.val_double = value}, .poisoned = false };


    return SUCCESS;
}


#include "../headers/front/keywords/keywords_on.hpp"

Return_code  analyse_name  (Word* word) {

    if (!word)                                                        { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (is_start_digitlike (word->str) || is_special (word->str [0])) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!strcmp (word->str, WHILE))  { word->type = LWT_WHILE; return SUCCESS; }
    if (!strcmp (word->str, IF))     { word->type = LWT_IF;    return SUCCESS; }
    if (!strcmp (word->str, ELSE))   { word->type = LWT_ELSE;  return SUCCESS; }
    if (!strcmp (word->str, NVAR))   { word->type = LWT_NVAR;  return SUCCESS; }
    if (!strcmp (word->str, NFUNC))  { word->type = LWT_NFUNC; return SUCCESS; }
    if (!strcmp (word->str, RETURN)) { word->type = LWT_RET;   return SUCCESS; }
    if (!strcmp (word->str, ASS))    { word->type = LWT_ASS;   return SUCCESS; }

    if (!strcmp (word->str, ADD))  { word->type = LWT_OP; word->val = { {.val_op_code = LOC_ADD},  false }; return SUCCESS; }
    if (!strcmp (word->str, SUB))  { word->type = LWT_OP; word->val = { {.val_op_code = LOC_SUB},  false }; return SUCCESS; }
    if (!strcmp (word->str, MULT)) { word->type = LWT_OP; word->val = { {.val_op_code = LOC_MULT}, false }; return SUCCESS; }
    if (!strcmp (word->str, DIV))  { word->type = LWT_OP; word->val = { {.val_op_code = LOC_DIV},  false }; return SUCCESS; }
    if (!strcmp (word->str, AND))  { word->type = LWT_OP; word->val = { {.val_op_code = LOC_AND},  false }; return SUCCESS; }
    if (!strcmp (word->str, OR))   { word->type = LWT_OP; word->val = { {.val_op_code = LOC_OR},   false }; return SUCCESS; }
    if (!strcmp (word->str, LT))   { word->type = LWT_OP; word->val = { {.val_op_code = LOC_LT},   false }; return SUCCESS; }
    if (!strcmp (word->str, GRT))  { word->type = LWT_OP; word->val = { {.val_op_code = LOC_GRT},  false }; return SUCCESS; }
    if (!strcmp (word->str, LEQ))  { word->type = LWT_OP; word->val = { {.val_op_code = LOC_LEQ},  false }; return SUCCESS; }
    if (!strcmp (word->str, GREQ)) { word->type = LWT_OP; word->val = { {.val_op_code = LOC_GREQ}, false }; return SUCCESS; }
    if (!strcmp (word->str, EQ))   { word->type = LWT_OP; word->val = { {.val_op_code = LOC_EQ},   false }; return SUCCESS; }
    if (!strcmp (word->str, NEQ))  { word->type = LWT_OP; word->val = { {.val_op_code = LOC_NEQ},  false }; return SUCCESS; }


    word->type = LWT_NAME;
    word->val  = { {.val_str = word->str}, false };


    return SUCCESS;
}

#include "../headers/front/keywords/keywords_off.hpp"


const char*  word_type_to_str  (Word_type word_type) {

    switch (word_type) {

        case LWT_UNKNOWN:     return "unknown";
        case LWT_NAME:        return "name";
        case LWT_CONST:       return "const";
        case LWT_NVAR:        return "nvar";
        case LWT_NFUNC:       return "nfunc";
        case LWT_BLOCK_OPEN:  return "{";
        case LWT_BLOCK_CLOSE: return "}";
        case LWT_ARG_OPEN:    return "(";
        case LWT_ARG_CLOSE:   return ")";
        case LWT_COMMA:       return ",";
        case LWT_SEMICOLON:   return ";";

        case LWT_CALL:  return "call";
        case LWT_OP:    return "op";
        case LWT_ASS:   return "ass";
        case LWT_WHILE: return "while";
        case LWT_IF:    return "if";
        case LWT_ELSE:  return "else";
        case LWT_RET:   return "return";
        case LWT_VAR:   return "var";


        default: LOG_ERROR (CRITICAL); return nullptr;
    }
}


Return_code  analyse_calls_and_vars  (Words* words) {

    if (!words) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    Function_list func_list = {}; function_list_ctor (&func_list);
    collect_fuctions (&func_list, words);

    Variable_list var_list = {}; variable_list_ctor (&var_list);
    collect_variables (&var_list, words);


    for (size_t i = 0; i < words->len; i++) {

        if ((words->list [i].type == LWT_NVAR) || (words->list [i].type == LWT_NFUNC)) { i += 1; continue; } //после nvar и nfunc идет name - все хорошо

        if (words->list [i].type != LWT_NAME) continue;



        bool in_func = in_function_list (&func_list, words->list [i].str);
        bool in_var  = in_variable_list ( &var_list, words->list [i].str);

        if ( in_func &&  in_var) { LOG_MESSAGE ("name collision for variable and function "); LOG_MESSAGE (words->list [i].str); LOG_MESSAGE ("\n"); return BAD_ARGS; }
        //if (!in_func && !in_var) { LOG_MESSAGE ("unknown token ");                            LOG_MESSAGE (words->list [i].str); LOG_MESSAGE ("\n"); return BAD_ARGS; }


        if (in_func) {

            words->list [i].type = LWT_CALL;
            continue;
        }

        words->list [i].type = LWT_VAR;
    }


    return SUCCESS;
}


Return_code  function_list_ctor  (Function_list* list) {

    if (!list) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    list->len = 0;
    list->list = (char**) calloc (sizeof (char*) * MAX_NUM_FUNCTIONS, 1);

    for (size_t i = 0; i < MAX_NUM_FUNCTIONS; i++) {

        list->list [i] = nullptr;
    }


    return SUCCESS;
}


Return_code  variable_list_ctor  (Variable_list* list) {

    if (!list) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    list->len = 0;
    list->list = (char**) calloc (sizeof (char*) * MAX_NUM_VARIABLES, 1);

    for (size_t i = 0; i < MAX_NUM_VARIABLES; i++) {

        list->list [i] = nullptr;
    }


    return SUCCESS;
}


Return_code  collect_fuctions  (Function_list* list, Words* words) {

    if (!list || !words) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < words->len; i++) {
    
        if (words->list [i].type == LWT_NFUNC) {

            if ( (i + 1) >= words->len ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

            list->list [list->len] = words->list [i + 1].str;
            list->len += 1;
        }
    }


    return SUCCESS;
}


Return_code  collect_variables  (Variable_list* list, Words* words) {

    if (!list || !words) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < words->len; i++) {
    
        if (words->list [i].type == LWT_NVAR) {

            if ( (i + 1) >= words->len ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

            list->list [list->len] = words->list [i + 1].str;
            list->len += 1;
        }
    }


    return SUCCESS;
}


bool  in_function_list  (Function_list* list, char* function) {

    if (!list || !function) { LOG_ERROR (BAD_ARGS); return false; }


    for (size_t i = 0; i < list->len; i++) {

        if (!strcmp (list->list [i], function)) {

            return true;
        }
    }


    return false;
}


bool  in_variable_list  (Variable_list* list, char* variable) {

    if (!list || !variable) { LOG_ERROR (BAD_ARGS); return false; }


    for (size_t i = 0; i < list->len; i++) {

        if (!strcmp (list->list [i], variable)) {

            return true;
        }
    }


    return false;
}


Return_code  build_tree  (Tree* tree, Words* words) {

    if (!tree || !words) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (tree->root) node_dtor (tree->root);


    build_defs (&tree->root, &words->list, words->list + words->len);


    return SUCCESS;
}


Return_code  build_defs  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (*word >= max_word)                         { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (((*word)->type != LWT_NVAR) && ((*word)->type != LWT_NFUNC)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    *node_ptr = create_node (LAT_DEFS);
    if ((*word)->type == LWT_NVAR) { try (build_nvar  (&(*node_ptr)->left_son, word, max_word)); }
    else                           { try (build_nfunc (&(*node_ptr)->left_son, word, max_word)); }



    Node* new_node = nullptr;
    Node* cur_node = *node_ptr;
    while (((*word) < max_word) && (((*word)->type == LWT_NVAR) || ((*word)->type == LWT_NFUNC))) {

        new_node = create_node (LAT_DEFS);

        if ((*word)->type == LWT_NVAR) { try (build_nvar  (&new_node->left_son, word, max_word)); }
        else                           { try (build_nfunc (&new_node->left_son, word, max_word)); }

        cur_node->right_son = new_node;
        cur_node = new_node;
    }


    return SUCCESS;
}


Node*  create_node  (Atom_type type, Atom value) {

    Node* ans = (Node*) calloc (NODE_SIZE, 1);

    ans->atom_type = type;
    ans->element   = { value, false };
    ans->left_son  = nullptr;
    ans->right_son = nullptr;


    return ans;
}


Return_code  build_nvar  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if ((*word)->type != LWT_NVAR) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    *node_ptr = create_node (LAT_NVAR);
    *word += 1;


    if (((*word) >= max_word) || ((*word)->type != LWT_NAME)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    (**node_ptr).element = (*word)->val;
    *word += 1;


    if (((*word) >= max_word) || ((*word)->type != LWT_ASS)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;


    if ((*word) >= max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    build_logical_sum (&(*node_ptr)->right_son, word, max_word);


    if (((*word) >= max_word) || ((*word)->type != LWT_SEMICOLON)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;


    return SUCCESS;
}


Return_code  build_logical_sum  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (build_logical_product (node_ptr, word, max_word));


    Node* new_node = nullptr;

    while (((*word)->type == LWT_OP) && ((*word)->val.value.val_op_code == LOC_OR)) {

        new_node = create_node (LAT_OP, (*word)->val.value);
        new_node->left_son = *node_ptr;


        *word += 1;

        *node_ptr = new_node;

        try (build_logical_product (&(*node_ptr)->right_son, word, max_word));
    }


    return SUCCESS;
}


Return_code  build_logical_product  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (build_relation_sum (node_ptr, word, max_word));


    Node* new_node = nullptr;

    while (((*word)->type == LWT_OP) && ((*word)->val.value.val_op_code == LOC_AND)) {

        new_node = create_node (LAT_OP, (*word)->val.value);
        new_node->left_son = *node_ptr;


        *word += 1;

        *node_ptr = new_node;

        try (build_relation_sum (&(*node_ptr)->right_son, word, max_word));
    }


    return SUCCESS;
}


Return_code  build_relation_sum  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (build_sum (node_ptr, word, max_word));


    Node* new_node = nullptr;

    while (((*word)->type == LWT_OP) && ((*word)->val.value.val_op_code == LOC_LT  || (*word)->val.value.val_op_code == LOC_GRT ||
                                         (*word)->val.value.val_op_code == LOC_LEQ || (*word)->val.value.val_op_code == LOC_GREQ ||
                                         (*word)->val.value.val_op_code == LOC_EQ  || (*word)->val.value.val_op_code == LOC_NEQ)) {


        new_node = create_node (LAT_OP, (*word)->val.value);
        new_node->left_son = *node_ptr;


        *word += 1;

        *node_ptr = new_node;

        try (build_sum (&(*node_ptr)->right_son, word, max_word));
    }


    return SUCCESS;
}


Return_code  build_sum  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (build_product (node_ptr, word, max_word));


    Node* new_node = nullptr;

    while (((*word)->type == LWT_OP) && ((*word)->val.value.val_op_code == LOC_ADD || (*word)->val.value.val_op_code == LOC_SUB)) {

        new_node = create_node (LAT_OP, (*word)->val.value);
        new_node->left_son = *node_ptr;


        *word += 1;

        *node_ptr = new_node;

        try (build_product (&(*node_ptr)->right_son, word, max_word));
    }


    return SUCCESS;
}


Return_code  build_product  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (build_primary (node_ptr, word, max_word));


    Node* new_node = nullptr;

    while (((*word)->type == LWT_OP) && ((*word)->val.value.val_op_code == LOC_MULT || (*word)->val.value.val_op_code == LOC_DIV)) {

        new_node = create_node (LAT_OP, (*word)->val.value);
        new_node->left_son = *node_ptr;


        *word += 1;

        *node_ptr = new_node;

        try (build_primary (&(*node_ptr)->right_son, word, max_word));
    }


    return SUCCESS;
}


//--------------------------------------------------
#define CHECK_CLOSING_BRACKET\
    if ((*word)->type != LWT_ARG_CLOSE) {\
\
        LOG_ERROR (BAD_ARGS);\
        return BAD_ARGS;\
    }
//--------------------------------------------------

Return_code  build_primary  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if ((*word)->type == LWT_ARG_OPEN) {

        *word += 1;

        try (build_logical_sum (&(*node_ptr)->right_son, word, max_word));

        CHECK_CLOSING_BRACKET

        *word += 1;
    }

    else { try (build_single (node_ptr, word, max_word)) };


    return SUCCESS;
}

//--------------------------------------------------
#undef CHECK_CLOSING_BRACKET
//--------------------------------------------------


Return_code  build_single  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if      ((*word)->type == LWT_VAR)   { try (build_variable (node_ptr, word, max_word)); }
    else if ((*word)->type == LWT_CONST) { try (build_const    (node_ptr, word, max_word)); }
    else if ((*word)->type == LWT_CALL)  { try (build_call     (node_ptr, word, max_word)); }

    else { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    return SUCCESS;
}


Return_code  build_variable  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if ((*word)->type != LWT_VAR)                                 { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    *node_ptr = create_node (LAT_VAR, (*word)->val.value);


    *word += 1;


    return SUCCESS;
}


Return_code  build_const  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if ((*word)->type != LWT_CONST)                               { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    *node_ptr = create_node (LAT_CONST, (*word)->val.value);


    *word += 1;


    return SUCCESS;
}


Return_code  build_call  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if ((*word)->type != LWT_CALL)                                { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    *node_ptr = create_node (LAT_CALL, (*word)->val.value);
    *word += 1;


    if ((*word >= max_word) || ((*word)->type != LWT_ARG_OPEN)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;


    try (build_arg (&(*node_ptr)->right_son, word, max_word));


    if ((*word >= max_word) || ((*word)->type != LWT_ARG_CLOSE)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;


    return SUCCESS;
}


Return_code  build_arg  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    *node_ptr = create_node (LAT_ARG);

    try (build_logical_sum (&(*node_ptr)->left_son, word, max_word));



    Node* new_node = nullptr;
    Node* cur_node = *node_ptr;

    while ((*word)->type == LWT_COMMA) {

        *word += 1;

        new_node = create_node (LAT_ARG);
        try (build_logical_sum (&new_node->left_son, word, max_word));

        cur_node->right_son = new_node;
        cur_node = new_node;
    }


    return SUCCESS;
}


Return_code  build_nfunc  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if ((*word)->type != LWT_NFUNC) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;


    if (((*word) >= max_word) || ((*word)->type != LWT_NAME)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *node_ptr = create_node (LAT_NFUNC, (*word)->val.value);
    *word += 1;



    if (((*word) >= max_word) || ((*word)->type != LWT_ARG_OPEN)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;



    if (((*word) < max_word) && ((*word)->type != LWT_ARG_CLOSE)) {

        build_par (&(*node_ptr)->left_son, word, max_word);
    }


    if (((*word) >= max_word) || ((*word)->type != LWT_ARG_CLOSE)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;



    try (build_block (&(*node_ptr)->right_son, word, max_word));



    return SUCCESS;
}


Return_code  build_block  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if ((*word)->type != LWT_BLOCK_OPEN) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;


    *node_ptr = create_node (LAT_BLOCK);


    build_seq (&(*node_ptr)->right_son, word, max_word);


    if (((*word) >= max_word) || ((*word)->type != LWT_BLOCK_CLOSE)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;


    return SUCCESS;
}


Return_code  build_seq  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    *node_ptr = create_node (LAT_SEQ);

    try (build_action (&(*node_ptr)->left_son, word, max_word));



    Node* new_node = nullptr;
    Node* cur_node = *node_ptr;

    while ((*word)->type != LWT_BLOCK_CLOSE) {

        new_node = create_node (LAT_SEQ);
        try (build_action (&new_node->left_son, word, max_word));

        cur_node->right_son = new_node;
        cur_node = new_node;
    }


    return SUCCESS;
}


Return_code  build_action  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    switch ((*word)->type) {

        case LWT_BLOCK_OPEN: try (build_block (node_ptr, word, max_word));                                        break;
        case LWT_NVAR:       try (build_nvar  (node_ptr, word, max_word));                                        break;
        case LWT_VAR:        try (build_ass   (node_ptr, word, max_word)); try (skip_semicolon (word, max_word)); break;
        case LWT_IF:         try (build_if    (node_ptr, word, max_word));                                        break;
        case LWT_WHILE:      try (build_while (node_ptr, word, max_word));                                        break;
        case LWT_RET:        try (build_ret   (node_ptr, word, max_word)); try (skip_semicolon (word, max_word)); break;
        case LWT_CALL:       try (build_call  (node_ptr, word, max_word)); try (skip_semicolon (word, max_word)); break;

        case LWT_UNKNOWN:     LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_NAME:        LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_CONST:       LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_NFUNC:       LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_BLOCK_CLOSE: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_ARG_OPEN:    LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_ARG_CLOSE:   LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_COMMA:       LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_SEMICOLON:   LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_OP:          LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_ASS:         LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LWT_ELSE:        LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        default:              LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }


    return SUCCESS;
}


Return_code  skip_semicolon  (Word** word, Word* max_word) {

    if (!word || !max_word || (*word >= max_word)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if ((*word)->type != LWT_SEMICOLON) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;


    return SUCCESS;
}


Return_code  build_ass  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if ((*word)->type != LWT_VAR) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    *node_ptr = create_node (LAT_ASS, (*word)->val.value);
    *word += 1;


    if ((*word)->type != LWT_ASS) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    *word += 1;


    try (build_logical_sum (&(*node_ptr)->right_son, word, max_word));


    return SUCCESS;
}


Return_code  build_if  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if ((*word)->type != LWT_IF) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; } //if

    *node_ptr = create_node (LAT_IF);
    *word += 1;


    if ((*word)->type != LWT_ARG_OPEN) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; } //(
    *word += 1;

    try (build_logical_sum (&(*node_ptr)->left_son, word, max_word)); //x == 0

    if ((*word)->type != LWT_ARG_CLOSE) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; } //)
    *word += 1;


    (*node_ptr)->right_son = create_node (LAT_BRANCH);
    try (build_block (&(*node_ptr)->right_son->left_son, word, max_word));


    if ((*word)->type == LWT_ELSE) { //else

        *word += 1;
        (*node_ptr)->right_son->right_son = create_node (LAT_BRANCH);
        try (build_block (&(*node_ptr)->right_son->right_son->left_son, word, max_word));
    }


    return SUCCESS;
}


Return_code  build_while  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if ((*word)->type != LWT_WHILE) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; } //while

    *node_ptr = create_node (LAT_WHILE);
    *word += 1;


    if ((*word)->type != LWT_ARG_OPEN) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; } //(
    *word += 1;

    try (build_logical_sum (&(*node_ptr)->left_son, word, max_word)); //x == 0

    if ((*word)->type != LWT_ARG_CLOSE) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; } //)
    *word += 1;


    try (build_block (&(*node_ptr)->right_son, word, max_word)); //{ code; }


    return SUCCESS;
}


Return_code  build_ret  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if ((*word)->type != LWT_RET) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    *node_ptr = create_node (LAT_RET, (*word)->val.value);
    *word += 1;


    try (build_logical_sum (&(*node_ptr)->right_son, word, max_word));


    return SUCCESS;
}


Return_code  build_par  (Node** node_ptr, Word** word, Word* max_word) {

    if (!node_ptr || !word || !*word || !max_word) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    *node_ptr = create_node (LAT_PAR, (*word)->val.value);
    *word += 1;


    Node* new_node = nullptr;
    Node* cur_node = *node_ptr;

    while ((*word)->type == LWT_COMMA) {

        *word += 1;

        new_node = create_node (LAT_PAR, (*word)->val.value);
        *word += 1;

        cur_node->right_son = new_node;
        cur_node = new_node;
    }


    return SUCCESS;
}


Return_code  STANDART_write_tree  (FILE* file, Tree* tree) {

    if (!file || !tree) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (STANDART_write_node (file, tree->root, 0));


    return SUCCESS;
}


Return_code  STANDART_write_node  (FILE* file, Node* node, size_t depth) {

    if (!file || !node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    write_tabs (file, depth); fprintf (file, "{");



    STANDART_write_type (file, node);
    STANDART_write_value (file, node);


    if (node->left_son) STANDART_write_node (file, node->left_son, depth + 1);
    else                STANDART_write_blank_node (file, depth + 1);

    if (node->right_son) STANDART_write_node (file, node->right_son, depth + 1);
    else                 STANDART_write_blank_node (file, depth + 1);


    write_tabs (file, depth); fprintf (file, "}\n");


    return SUCCESS;
}


Return_code  STANDART_write_type  (FILE* file, Node* node) {

    if (!file || !node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    /*write_tabs (file, depth);*/ fprintf (file, "%s ", STANDART_atom_type_to_str (node->atom_type));


    return SUCCESS;
}


Return_code  STANDART_write_value  (FILE* file, Node* node) {

    if (!file || !node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    /*write_tabs (file, depth);*/


    switch (node->atom_type) {

        case LAT_DEFS:   fprintf (file, "NULL\n"); break;
        case LAT_NVAR:   fprintf (file, "%s\n", node->element.value.val_str); break;
        case LAT_NFUNC:  fprintf (file, "%s\n", node->element.value.val_str); break;
        case LAT_BLOCK:  fprintf (file, "NULL\n"); break;
        case LAT_ARG:    fprintf (file, "NULL\n"); break;
        case LAT_CALL:   fprintf (file, "%s\n", node->element.value.val_str); break;
        case LAT_PAR:    fprintf (file, "%s\n", node->element.value.val_str); break;
        case LAT_OP:     fprintf (file, "%s\n", STANDART_op_code_to_str (node->element.value.val_op_code)); break;
        case LAT_SEQ:    fprintf (file, "NULL\n"); break;
        case LAT_ASS:    fprintf (file, "%s\n", node->element.value.val_str); break;
        case LAT_WHILE:  fprintf (file, "NULL\n"); break;
        case LAT_IF:     fprintf (file, "NULL\n"); break;
        case LAT_BRANCH: fprintf (file, "NULL\n"); break;
        case LAT_RET:    fprintf (file, "NULL\n"); break;
        case LAT_CONST:  fprintf (file, "%.3lf\n", node->element.value.val_double); break;
        case LAT_VAR:    fprintf (file, "%s\n", node->element.value.val_str); break;

        default: LOG_ERROR (CRITICAL); return CRITICAL;
    }


    return SUCCESS;
}


const char*  STANDART_atom_type_to_str  (Atom_type type) {

    switch (type) {

        case LAT_DEFS:   return "DEFS";
        case LAT_NVAR:   return "NVAR";
        case LAT_NFUNC:  return "NFUN";
        case LAT_BLOCK:  return "BLOCK";
        case LAT_ARG:    return "ARG";
        case LAT_CALL:   return "CALL";
        case LAT_PAR:    return "PAR";
        case LAT_OP:     return "OP";
        case LAT_SEQ:    return "SEQ";
        case LAT_ASS:    return "ASS";
        case LAT_WHILE:  return "WHILE";
        case LAT_IF:     return "IF";
        case LAT_BRANCH: return "BRANCH";
        case LAT_RET:    return "RET";
        case LAT_CONST:  return "CONST";
        case LAT_VAR:    return "VAR";

        default: LOG_ERROR (CRITICAL); return nullptr;
    }
}


const char*  STANDART_op_code_to_str  (Op_code code) {

    switch (code) {

        case LOC_ADD: return "ADD";
        case LOC_SUB: return "SUB";
        case LOC_MULT: return "MUL";
        case LOC_DIV: return "DIV";
        case LOC_AND: return "AND";
        case LOC_OR: return "OR";
        case LOC_LT: return "LT";
        case LOC_GRT: return "GT";
        case LOC_LEQ: return "LEQ";
        case LOC_GREQ: return "GEQ";
        case LOC_EQ: return "EQ";
        case LOC_NEQ: return "NEQ";

        default: LOG_ERROR (CRITICAL); return nullptr;
    }
}


Return_code  STANDART_write_blank_node  (FILE* file, size_t depth) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    write_tabs (file, depth); fprintf (file, "{ }\n");


    return SUCCESS;
}


Return_code  write_tabs  (FILE* file, size_t depth) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < depth; i++) {

        fprintf (file, "    ");
    }


    return SUCCESS;
}
