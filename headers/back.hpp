#ifndef BACK_HPP_INCLUDED
#define BACK_HPP_INCLUDED


//--------------------------------------------------


#include <stdio.h>
#include <stdlib.h>

#include "../lib/types/Return_code.hpp"
#include "../lib/types/Tree_lang.hpp"


#include "back/constants.hpp"
#include "back/enums.hpp"
#include "back/structures.hpp"

#include "common/constants.hpp"
#include "common/enums.hpp"

//--------------------------------------------------


Return_code back_end (const char* tree_file_name = LANG_DEFAULT_TREE_FILE_NAME, const char* asm_file_name = LANG_DEFAULT_ASM_FILE_NAME);

Return_code build_tree       (FILE* file, Tree* tree);
Return_code build_node       (FILE* file, Node** node_ptr);
Return_code build_type       (FILE* file, Node* node);
Return_code build_value      (FILE* file, Node* node);
Return_code build_str        (FILE* file, Node* node);
Return_code build_double     (FILE* file, Node* node);
Return_code build_null       (FILE* file, Node* node);
Return_code build_op         (FILE* file, Node* node);
Return_code skip_block_open  (FILE* file);
Return_code skip_block_close (FILE* file);
Return_code skip_spaces      (FILE* file);


Return_code node_ctor (Node* node);

Return_code var_info_ctor (Var_info* info);

Return_code name_table_ctor           (Name_table* table);
Return_code repeat_name_table         (Compiler_info* info);
Return_code make_name_table           (Compiler_info* info);
Return_code copy_table                (Name_table* dest, Name_table* src);
Return_code pop_name_table            (Compiler_info* info);
Return_code name_table_to_zero        (Name_table* table);
bool        in_table                  (Name_table* table, char* var);
Return_code table_set_variable_ptr    (Name_table* table, char* var, size_t ptr);
Return_code print_variable_ptr        (FILE* file, char* var, Compiler_info* info);
Return_code print_global_variable_ptr (FILE* file, char* var, Compiler_info* info);
Return_code print_local_variable_ptr  (FILE* file, char* var, Compiler_info* info);

Return_code compiler_info_ctor               (Compiler_info* info);
Return_code compile_info_add_global_variable (Compiler_info* info, char* var);
Return_code compile_info_add_local_variable  (Compiler_info* info, char* var);


Return_code compile_tree   (FILE* file, Tree* tree);
Return_code compile_node   (FILE* file, Node* node, Compiler_info* info);
Return_code compile_defs   (FILE* file, Node* node, Compiler_info* info);
Return_code compile_const  (FILE* file, Node* node);
Return_code compile_op     (FILE* file, Node* node, Compiler_info* info);
Return_code compile_var    (FILE* file, Node* node, Compiler_info* info);
Return_code compile_ret    (FILE* file, Node* node, Compiler_info* info);
Return_code compile_seq    (FILE* file, Node* node, Compiler_info* info);
Return_code compile_ass    (FILE* file, Node* node, Compiler_info* info);
Return_code compile_call   (FILE* file, Node* node, Compiler_info* info);
Return_code compile_arg    (FILE* file, Node* node, Compiler_info* info);
Return_code compile_nvar   (FILE* file, Node* node, Compiler_info* info);
Return_code compile_block  (FILE* file, Node* node, Compiler_info* info);
Return_code compile_nfunc  (FILE* file, Node* node, Compiler_info* info);
Return_code compile_par    (            Node* node, Compiler_info* info);
Return_code compile_if     (FILE* file, Node* node, Compiler_info* info);
Return_code compile_branch (FILE* file, Node* node, Compiler_info* info);
Return_code compile_while  (FILE* file, Node* node, Compiler_info* info);

size_t add_and_count_parameters (Node* node, Compiler_info* info);

Return_code print_std (FILE* file, const char* std_file_name = LANG_DEFAULT_STD_FILE_NAME);


//--------------------------------------------------


#endif

