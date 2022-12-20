#ifndef ELEMENT_atom_HPP_INCLUDED
#define ELEMENT_atom_HPP_INCLUDED






#undef  Element_value
#define Element_value Element_atom_value

#undef  Element
#define Element Element_atom

#undef  _poisoned_Element_value
#define _poisoned_Element_value _poisoned_Element_atom_value

#undef  Element_structure
#define Element_structure Element_atom_structure

#undef  ELEMENT_SIZE
#define ELEMENT_SIZE ELEMENT_atom_SIZE

#undef  _ELEMENT_PRINT_FORMAT
#define _ELEMENT_PRINT_FORMAT "%p"


union Atom {

    Op_code        val_op_code;
    double         val_double;
    char*          val_str;
};


typedef Atom Element_value;


Element_value const _poisoned_Element_value = { .val_str = nullptr };


typedef struct Element_structure Element;
struct         Element_structure  {

    Element_value value;
    bool          poisoned;
};


const size_t ELEMENT_SIZE = sizeof (Element);









#endif