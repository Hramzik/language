

//--------------------------------------------------


#include "../headers/back.hpp"

#include "../lib/logs.hpp"
#include "../lib/onegin.hpp"
#include "../lib/tree_lang.hpp"


//--------------------------------------------------


Return_code  back_end  (const char* tree_file_name, const char* asm_file_name) {

    if (!tree_file_name || !asm_file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    FILE* tree_file = fopen (tree_file_name, "r");
    FILE*  asm_file = fopen ( asm_file_name, "w");


    Tree tree = {}; TREE_CTOR (&tree); tree_kill_root (&tree);


    try (build_tree (tree_file, &tree));


    try (compile_tree (asm_file, &tree));


    return SUCCESS;
}


Return_code  build_tree  (FILE* file, Tree* tree) {

    if (!file || !tree) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (build_node (file, &tree->root));


    return SUCCESS;
}


Return_code  build_node  (FILE* file, Node** node_ptr) {

    if (!file || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    skip_block_open (file);


    *node_ptr = (Node*) calloc (NODE_SIZE, 1); node_ctor (*node_ptr);


    Return_code code = build_type (file, *node_ptr);
    if (code) {

        if (code == SPECIAL_CASE) { node_dtor (*node_ptr); *node_ptr = nullptr; return SUCCESS; }

        LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }


    try (build_value (file, *node_ptr));
    try (build_node  (file, &(*node_ptr)-> left_son));
    try (build_node  (file, &(*node_ptr)->right_son));


    skip_block_close (file);


    return SUCCESS;
}


Return_code  build_type  (FILE* file, Node* node) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    char type_str [MAX_WORD_LEN + 1] = "";

    fscanf (file, "%s", type_str);

    if      (!strcmp (type_str, "}"))      { return SPECIAL_CASE; } //пустой узел
    else if (!strcmp (type_str, "DEFS"))   { node->atom_type = LAT_DEFS; }
    else if (!strcmp (type_str, "NVAR"))   { node->atom_type = LAT_NVAR; }
    else if (!strcmp (type_str, "NFUN"))   { node->atom_type = LAT_NFUNC; }
    else if (!strcmp (type_str, "BLOCK"))  { node->atom_type = LAT_BLOCK; }
    else if (!strcmp (type_str, "ARG"))    { node->atom_type = LAT_ARG; }
    else if (!strcmp (type_str, "CALL"))   { node->atom_type = LAT_CALL; }
    else if (!strcmp (type_str, "PAR"))    { node->atom_type = LAT_PAR; }
    else if (!strcmp (type_str, "OP"))     { node->atom_type = LAT_OP; }
    else if (!strcmp (type_str, "SEQ"))    { node->atom_type = LAT_SEQ; }
    else if (!strcmp (type_str, "ASS"))    { node->atom_type = LAT_ASS; }
    else if (!strcmp (type_str, "WHILE"))  { node->atom_type = LAT_WHILE; }
    else if (!strcmp (type_str, "IF"))     { node->atom_type = LAT_IF; }
    else if (!strcmp (type_str, "BRANCH")) { node->atom_type = LAT_BRANCH; }
    else if (!strcmp (type_str, "RET"))    { node->atom_type = LAT_RET; }
    else if (!strcmp (type_str, "CONST"))  { node->atom_type = LAT_CONST; }
    else if (!strcmp (type_str, "VAR"))    { node->atom_type = LAT_VAR; }

    else { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    return SUCCESS;
}


Return_code  build_value  (FILE* file, Node* node) {

    if (!file || !node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    switch (node->atom_type) {

        case LAT_DEFS:   build_null   (file, node); break;
        case LAT_NVAR:   build_str    (file, node); break;
        case LAT_NFUNC:  build_str    (file, node); break;
        case LAT_BLOCK:  build_null   (file, node); break;
        case LAT_ARG:    build_null   (file, node); break;
        case LAT_CALL:   build_str    (file, node); break;
        case LAT_PAR:    build_str    (file, node); break;
        case LAT_OP:     build_op     (file, node); break;
        case LAT_SEQ:    build_null   (file, node); break;
        case LAT_ASS:    build_str    (file, node); break;
        case LAT_WHILE:  build_null   (file, node); break;
        case LAT_IF:     build_null   (file, node); break;
        case LAT_BRANCH: build_null   (file, node); break;
        case LAT_RET:    build_null   (file, node); break;
        case LAT_CONST:  build_double (file, node); break;
        case LAT_VAR:    build_str    (file, node); break;

        default: LOG_ERROR (CRITICAL); return CRITICAL;
    }


    return SUCCESS;
}


Return_code  build_str  (FILE* file, Node* node) {

    if (!file || !node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    node->element.value.val_str  = (char*) calloc (MAX_WORD_LEN + 1, 1);
    fscanf (file, "%s", node->element.value.val_str);

    node->element.poisoned = false;


    return SUCCESS;
}


Return_code  build_double  (FILE* file, Node* node) {

    if (!file || !node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fscanf (file, "%lf", &node->element.value.val_double);
    node->element.poisoned = false;


    return SUCCESS;
}


Return_code  build_null  (FILE* file, Node* node) {

    if (!file || !node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    char null_str [MAX_WORD_LEN + 1] = "";
    fscanf (file, "%s", null_str);


    if (strcmp (null_str, "NULL")) { LOG_MESSAGE ("EXPECTED NULL, got"); LOG_MESSAGE (null_str); return BAD_ARGS; }


    return SUCCESS;
}


Return_code  build_op  (FILE* file, Node* node) {

    if (!file || !node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    char op_str [MAX_WORD_LEN + 1] = "";
    fscanf (file, "%s", op_str);


    if      (!strcmp (op_str, "ADD")) { node->element = { {.val_op_code = LOC_ADD},  false}; }
    else if (!strcmp (op_str, "SUB")) { node->element = { {.val_op_code = LOC_SUB},  false}; }
    else if (!strcmp (op_str, "MUL")) { node->element = { {.val_op_code = LOC_MULT}, false}; }
    else if (!strcmp (op_str, "DIV")) { node->element = { {.val_op_code = LOC_DIV},  false}; }
    else if (!strcmp (op_str, "AND")) { node->element = { {.val_op_code = LOC_AND},  false}; }
    else if (!strcmp (op_str, "OR"))  { node->element = { {.val_op_code = LOC_OR},   false}; }
    else if (!strcmp (op_str, "LT"))  { node->element = { {.val_op_code = LOC_LT},   false}; }
    else if (!strcmp (op_str, "GT"))  { node->element = { {.val_op_code = LOC_GRT},  false}; }
    else if (!strcmp (op_str, "LEQ")) { node->element = { {.val_op_code = LOC_LEQ},  false}; }
    else if (!strcmp (op_str, "GEQ")) { node->element = { {.val_op_code = LOC_GREQ}, false}; }
    else if (!strcmp (op_str, "EQ"))  { node->element = { {.val_op_code = LOC_EQ},   false}; }
    else if (!strcmp (op_str, "NEQ")) { node->element = { {.val_op_code = LOC_NEQ},  false}; }


    return SUCCESS;
}


Return_code  node_ctor  (Node* node) {

    if (!node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    node->atom_type = LAT_DEFS;
    node->element   = { {.val_str = nullptr}, false };
    node->left_son  = nullptr;
    node->right_son = nullptr;


    return SUCCESS;
}


Return_code  skip_block_open  (FILE* file) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    skip_spaces (file);


    int c = ' ';
    c = fgetc (file);
    if (c != '{') { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    return SUCCESS;
}


Return_code  skip_block_close  (FILE* file) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    skip_spaces (file);


    int c = ' ';
    c = fgetc (file);
    if (c != '}') { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    return SUCCESS;
}


Return_code  skip_spaces  (FILE* file) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    int c = ' ';
    while (is_space ( (char) c)) { c = fgetc (file); }
    ungetc (c, file);


    return SUCCESS;
}


//--------------------------------------------------


Return_code  compile_tree  (FILE* file, Tree* tree) {

    if (!file || !tree) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    Compiler_info info = {}; compiler_info_ctor (&info);


    fprintf (file, "push 0\n");
    fprintf (file, "pop RBP\n\n");
    fprintf (file, "call main:\n");
    fprintf (file, "halt\n");


    try (compile_node (file, tree->root, &info));


    print_std (file);


    return SUCCESS;
}


Return_code  compile_node  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    switch (node->atom_type) {

        case LAT_DEFS:   compile_defs   (file, node, info); break;
        case LAT_NVAR:   compile_nvar   (file, node, info); break;
        case LAT_NFUNC:  compile_nfunc  (file, node, info); break;
        case LAT_BLOCK:  compile_block  (file, node, info); break;
        case LAT_ARG:    compile_arg    (file, node, info); break;
        case LAT_CALL:   compile_call   (file, node, info); break;
        case LAT_OP:     compile_op     (file, node, info); break;
        case LAT_SEQ:    compile_seq    (file, node, info); break;
        case LAT_ASS:    compile_ass    (file, node, info); break;
        case LAT_WHILE:  compile_while  (file, node, info); break;
        case LAT_IF:     compile_if     (file, node, info); break;
        case LAT_RET:    compile_ret    (file, node, info); break;
        case LAT_CONST:  compile_const  (file, node);       break;
        case LAT_VAR:    compile_var    (file, node, info); break;

        case LAT_BRANCH: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        case LAT_PAR:    LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        default:         LOG_ERROR (CRITICAL); return CRITICAL;
    }


    return SUCCESS;
}


Return_code  compile_defs  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node)              { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_DEFS) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!node->left_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    try (compile_node (file, node->left_son, info));


    if (node->right_son) { try (compile_node (file, node->right_son, info)); }


    return SUCCESS;
}


Return_code  compile_const  (FILE* file, Node* node) {

    if (!file || !node)               { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_CONST) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "push %lf\n", node->element.value.val_double);


    return SUCCESS;
}


Return_code  compile_op  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node)            { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_OP) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (node-> left_son) { try (compile_node (file, node-> left_son, info)); }
    if (node->right_son) { try (compile_node (file, node->right_son, info)); }


    switch (node->element.value.val_op_code) {

        case LOC_ADD:  fprintf (file, "add\n");  break;
        case LOC_SUB:  fprintf (file, "sub\n");  break;
        case LOC_MULT: fprintf (file, "mul\n"); break;
        case LOC_DIV:  fprintf (file, "div\n");  break;
        case LOC_AND:  fprintf (file, "call and:\n");  break;
        case LOC_OR:   fprintf (file, "call or:\n");   break;
        case LOC_LT:   fprintf (file, "call lt:\n");   break;
        case LOC_GRT:  fprintf (file, "call grt:\n");  break;
        case LOC_LEQ:  fprintf (file, "call leq:\n");  break;
        case LOC_GREQ: fprintf (file, "call greq:\n"); break;
        case LOC_EQ:   fprintf (file, "call eq:\n");   break;
        case LOC_NEQ:  fprintf (file, "call neq:\n");  break;

        default: LOG_ERROR (CRITICAL); return CRITICAL;
    }


    return SUCCESS;
}


Return_code  compile_var  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node)             { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_VAR) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "push ");
    print_variable_ptr (file, node->element.value.val_str, info);
    fprintf (file, "\n");


    return SUCCESS;
}


Return_code  compile_ret  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node)             { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_RET) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!node->right_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    compile_node (file, node->right_son, info);


    fprintf (file, "ret\n");


    return SUCCESS;
}


Return_code  compile_seq  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node)             { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_SEQ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!node->left_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    compile_node (file, node->left_son, info);


    if (node->right_son) compile_node (file, node->right_son, info);


    return SUCCESS;
}


Return_code  compile_ass  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node)             { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_ASS) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!node->right_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    compile_node (file, node->right_son, info);


    fprintf (file, "pop");
    print_variable_ptr (file, node->element.value.val_str, info);
    fprintf (file, "\n");


    return SUCCESS;
}


Return_code  compile_call  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node)              { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_CALL) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!node->right_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; } //не бывает функций без аргументов
    compile_node (file, node->right_son, info);


    fprintf (file, "push RBP\n");
    fprintf (file, "push %zd\n", info->tables [info->num_tables - 1].free_ptr + 1); //почему +1?
    fprintf (file, "add\n");
    fprintf (file, "pop RBP\n");
    fprintf (file, "call %s:\n", node->element.value.val_str);
    fprintf (file, "push RBP\n");
    fprintf (file, "push %zd\n", info->tables [info->num_tables - 1].free_ptr + 1);
    fprintf (file, "sub\n");
    fprintf (file, "pop RBP\n");


    return SUCCESS;
}


Return_code  compile_arg  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node)             { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_ARG) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!node->left_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    try (compile_node (file, node->left_son, info));


    if (node->right_son) { try (compile_node (file, node->right_son, info)); }


    return SUCCESS;
}


Return_code  var_info_ctor  (Var_info* info) {

    if (!info) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    info->name = nullptr;
    info->ptr  = 0;


    return SUCCESS;
}


Return_code  name_table_ctor  (Name_table* table) {

    if (!table) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    table->list     = (Var_info*) calloc (MAX_NUM_VARIABLES * VAR_INFO_SIZE, 1);

    for (size_t i = 0; i < MAX_NUM_VARIABLES; i++) {

        var_info_ctor (&table->list [i]);
    }


    table->num_vars = 0;


    table->free_ptr = 0;


    return SUCCESS;
}


Return_code  compiler_info_ctor  (Compiler_info* info) {

    if (!info) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    info->num_if    = 0;
    info->num_while = 0;


    info->tables = (Name_table*) calloc (MAX_NUM_NAME_TABLES * NAME_TABLE_SIZE, 1);

    for (size_t i = 0; i < MAX_NUM_NAME_TABLES; i++) {

        name_table_ctor (&info->tables [i]);
    }


    info->num_tables = 1;


    return SUCCESS;
}


Return_code  compile_nvar  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node)              { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_NVAR) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!node->right_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    try (compile_node (file, node->right_son, info));


    if (info->num_tables == 1) { //на данном шаге есть только глобальная таблица имен

        compile_info_add_global_variable (info, node->element.value.val_str);
    }

    else {

        compile_info_add_local_variable (info, node->element.value.val_str);
    }


    print_variable_ptr (file, node->element.value.val_str, info);


    return SUCCESS;
}


Return_code  compile_info_add_global_variable  (Compiler_info* info, char* var) {

    if (!info || !var)                     { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (in_table (&info->tables [0], var)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    info->tables [0].list [info->tables [0].num_vars].name = strdup (var);
    info->tables [0].list [info->tables [0].num_vars].ptr  = info->tables [0].free_ptr;


    info->tables [0].num_vars += 1;
    info->tables [0].free_ptr += 1;


    return SUCCESS;
}


Return_code  compile_info_add_local_variable  (Compiler_info* info, char* var) {

    if (!info || !var) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (in_table (&info->tables [info->num_tables - 1], var)) {

        table_set_variable_ptr (&info->tables [info->num_tables - 1], var, info->tables [info->num_tables - 1].free_ptr);
        info->tables [info->num_tables].free_ptr += 1;

        return SUCCESS;
    }


    info->tables [info->num_tables - 1].list [info->tables [info->num_tables - 1].num_vars].name = strdup (var);
    info->tables [info->num_tables - 1].list [info->tables [info->num_tables - 1].num_vars].ptr  = info->tables [info->num_tables - 1].free_ptr;


    info->tables [info->num_tables - 1].num_vars += 1;
    info->tables [info->num_tables - 1].free_ptr += 1;


    return SUCCESS;
}


bool  in_table  (Name_table* table, char* var) {

    if (!table || !var) { LOG_ERROR (BAD_ARGS); return false; }


    for (size_t i = 0; i < table->num_vars; i++) {

        if (!strcmp (table->list [i].name, var)) {

            return true;
        }
    }


    return false;
}


Return_code  table_set_variable_ptr  (Name_table* table, char* var, size_t ptr) {

    if (!table || !var) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < table->num_vars; i++) {

        if (!strcmp (table->list [i].name, var)) {

            table->list [i].ptr = ptr;
            return SUCCESS;
        }
    }


    return BAD_ARGS;
}


Return_code  print_variable_ptr  (FILE* file, char* var, Compiler_info* info) {

    if (!info || !var) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (info->num_tables == 1) print_global_variable_ptr (file, var, info);
    else                       print_local_variable_ptr  (file, var, info);


    return SUCCESS;
}


Return_code  print_global_variable_ptr (FILE* file, char* var, Compiler_info* info) {

    if (!info || !var)         { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (info->num_tables != 1) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < info->tables [info->num_tables - 1].num_vars; i++) {

        if (!strcmp (info->tables [info->num_tables - 1].list [i].name, var)) {

            fprintf (file, "[%zd]", info->tables [info->num_tables - 1].list [i].ptr);

            return SUCCESS;
        }
    }


    LOG_ERROR (BAD_ARGS); return BAD_ARGS;
}


Return_code  print_local_variable_ptr (FILE* file, char* var, Compiler_info* info) {

    if (!info || !var)         { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (info->num_tables == 1) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < info->tables [info->num_tables - 1].num_vars; i++) {

        if (!strcmp (info->tables [info->num_tables - 1].list [i].name, var)) {

            fprintf (file, "[%zd + RBP]", info->tables [info->num_tables - 1].list [i].ptr);

            return SUCCESS;
        }
    }


    LOG_ERROR (BAD_ARGS); return BAD_ARGS;
}


Return_code  compile_block  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node)               { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_BLOCK) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    repeat_name_table (info);


    if (!node->right_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    compile_node (file, node->right_son, info);


    pop_name_table (info);


    return SUCCESS;
}


Return_code  repeat_name_table  (Compiler_info* info) {

    if (!info) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    copy_table (&info->tables [info->num_tables], &info->tables [info->num_tables - 1]);


    info->num_tables += 1;


    return SUCCESS;
}


Return_code  make_name_table  (Compiler_info* info) {

    if (!info) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    copy_table (&info->tables [info->num_tables], &info->tables [0]); //копируем из таблицы глобальных переменных


    info->num_tables += 1;


    return SUCCESS;
}


Return_code  copy_table  (Name_table* dest, Name_table* src) {

    if (!dest || !src) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < src->num_vars; i++) {

        dest->list [i].name = strdup (src->list [i].name);
        dest->list [i].ptr  = src->list [i].ptr;
    }

    dest->num_vars = src->num_vars;


    dest->free_ptr = src->free_ptr;


    return SUCCESS;
}


Return_code  pop_name_table  (Compiler_info* info) {

    if (!info) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    name_table_to_zero (&info->tables [info->num_tables - 1]);


    info->num_tables -= 1;


    return SUCCESS;
}


Return_code  name_table_to_zero  (Name_table* table) {

    if (!table) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < table->num_vars; i++) {

        table->list [i].name = nullptr;
        table->list [i].ptr  = 0;
    }

    table->num_vars = 0;


    table->free_ptr = 0;


    return SUCCESS;
}


Return_code  compile_nfunc  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node || !info)      { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_NFUNC) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\n%s:\n", node->element.value.val_str);


    make_name_table (info);


    size_t num_pars = add_and_count_parameters (node, info);


    for (size_t i = 0; i < num_pars; i++) {

        fprintf (file, "pop [%zd + RBP]\n", num_pars - i - 1);
    }


    if (!node->right_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    compile_node (file, node->right_son, info);


    pop_name_table (info);


    return SUCCESS;
}


size_t  add_and_count_parameters  (Node* node, Compiler_info* info) {

    if (!node || !info)               { LOG_ERROR (BAD_ARGS); return 0; }
    if (node->atom_type != LAT_NFUNC) { LOG_ERROR (BAD_ARGS); return 0; }


    size_t num_pars = 0;
    Node* cur = node->left_son; //par

    while (cur != nullptr) {

        compile_par (cur, info);

        cur = cur->right_son;

        num_pars += 1;
    }


    return num_pars;
}


Return_code  compile_par  (Node* node, Compiler_info* info) {

    if (!node || !info)             { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_PAR) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    compile_info_add_local_variable (info, node->element.value.val_str);


    return SUCCESS;
}


Return_code  compile_if  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node || !info)   { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_IF) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!node->left_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    compile_node (file, node->left_son, info);


    fprintf (file, "push 0\n");
    fprintf (file, "jne if_%zd:\n",   info->num_if);
    fprintf (file, "jump else_%zd:\n", info->num_if);


    if (!node->right_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    compile_branch (file, node->right_son, info);


    fprintf (file, "end_if_%zd:\n", info->num_if);


    info->num_if += 1;


    return SUCCESS;
}


Return_code  compile_branch  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node || !info)       { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_BRANCH) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if (!node->left_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    fprintf (file, "if_%zd:\n", info->num_if);
    compile_node (file, node->left_son, info);
    fprintf (file, "jump end_if_%zd:\n", info->num_if);
    fprintf (file, "else_%zd:\n", info->num_if);



    if (node->right_son) {

        if (!node->right_son->left_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
        compile_node (file, node->right_son->left_son, info);
    }


    return SUCCESS;
}


Return_code  compile_while  (FILE* file, Node* node, Compiler_info* info) {

    if (!file || !node || !info)      { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (node->atom_type != LAT_WHILE) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!node->left_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    compile_node (file, node->left_son, info);


    fprintf (file, "push 0\n");
    fprintf (file, "jne while_%zd:\n",     info->num_while);
    fprintf (file, "jump end_while_%zd:\n", info->num_while);


    if (!node->right_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    fprintf (file, "while_%zd:\n", info->num_while);
    compile_node (file, node->right_son, info);


    fprintf (file, "end_while_%zd:\n", info->num_while);


    info->num_while += 1;


    return SUCCESS;
}


Return_code  print_std  (FILE* file, const char* std_file_name) {

    if (!file || !std_file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    FILE* stdf = fopen (std_file_name, "r");


    size_t stdf_len = get_file_len (stdf);
    char buff [stdf_len] = ""; fread (buff, 1, stdf_len, stdf);
    fclose (stdf);


    fprintf (file, "%s", buff);


    return SUCCESS;
}


