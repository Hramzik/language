#ifndef FRONT_HPP_INCLUDED
#define FRONT_HPP_INCLUDED


//--------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/types/Return_code.hpp"
#include "../lib/types/Tree_lang.hpp"


#include "front/constants.hpp"
#include "front/enums.hpp"
#include "front/structures.hpp"

#include "common/constants.hpp"
#include "common/enums.hpp"


//--------------------------------------------------


Return_code front_end (const char* file_name = LANG_DEFAULT_CODE_FILE_NAME, const char* tree_file_name = LANG_DEFAULT_TREE_FILE_NAME);


Return_code read_word_list     (FILE* file, Words* words);
bool        is_special         (char c);
bool        is_start_digitlike (char* str);

Return_code analyse_words          (Words* words);
Return_code analyse_special        (Word* word);
Return_code analyse_const          (Word* word);
Return_code analyse_name           (Word* word);
Return_code analyse_calls_and_vars (Words* words);


Return_code build_tree             (Tree* tree, Words* words);
Return_code build_defs             (Node** node_ptr, Word** word, Word* max_word);
Return_code build_nvar             (Node** node_ptr, Word** word, Word* max_word);
Return_code build_logical_sum      (Node** node_ptr, Word** word, Word* max_word);
Return_code build_logical_product  (Node** node_ptr, Word** word, Word* max_word);
Return_code build_relation_sum     (Node** node_ptr, Word** word, Word* max_word);
Return_code build_sum              (Node** node_ptr, Word** word, Word* max_word);
Return_code build_product          (Node** node_ptr, Word** word, Word* max_word);
Return_code build_primary          (Node** node_ptr, Word** word, Word* max_word);
Return_code build_single           (Node** node_ptr, Word** word, Word* max_word);
Return_code build_variable         (Node** node_ptr, Word** word, Word* max_word);
Return_code build_const            (Node** node_ptr, Word** word, Word* max_word);
Return_code build_call             (Node** node_ptr, Word** word, Word* max_word);
Return_code build_arg              (Node** node_ptr, Word** word, Word* max_word);
Return_code build_nfunc            (Node** node_ptr, Word** word, Word* max_word);
Return_code build_block            (Node** node_ptr, Word** word, Word* max_word);
Return_code build_seq              (Node** node_ptr, Word** word, Word* max_word);
Return_code build_action           (Node** node_ptr, Word** word, Word* max_word);
Return_code skip_semicolon         (Word** word, Word* max_word);
Return_code build_ass              (Node** node_ptr, Word** word, Word* max_word);
Return_code build_if               (Node** node_ptr, Word** word, Word* max_word);
Return_code build_while            (Node** node_ptr, Word** word, Word* max_word);
Return_code build_ret              (Node** node_ptr, Word** word, Word* max_word);
Return_code build_par              (Node** node_ptr, Word** word, Word* max_word);


Return_code write_tabs                (FILE* file, size_t depth);
Return_code STANDART_write_tree       (FILE* file, Tree* tree);
Return_code STANDART_write_node       (FILE* file, Node* node, size_t depth);
Return_code STANDART_write_type       (FILE* file, Node* node);
Return_code STANDART_write_value      (FILE* file, Node* node);
Return_code STANDART_write_blank_node (FILE* file,             size_t depth);
const char* STANDART_atom_type_to_str (Atom_type type);
const char* STANDART_op_code_to_str   (Op_code code);




Return_code words_ctor       (Words* words, size_t len);
Return_code words_dtor       (Words* words);
const char* word_type_to_str (Word_type word_type);

Return_code function_list_ctor (Function_list* list);
Return_code variable_list_ctor (Variable_list* list);
Return_code collect_fuctions   (Function_list* list, Words* words);
Return_code collect_variables  (Variable_list* list, Words* words);

bool        in_function_list   (Function_list* list, char* function);
bool        in_variable_list   (Variable_list* list, char* variable);


Node* create_node (Atom_type type, Atom value = {.val_str = nullptr } );

//--------------------------------------------------


#endif

