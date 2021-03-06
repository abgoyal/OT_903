
#include <stddef.h>



/* Optionally include an entry pointing to the first used entry.  */
#ifdef ITERATE
# define FIRST(name)	name##_ent *first;
# define NEXT(name)	struct name##_ent *next;
#else
# define FIRST(name)
# define NEXT(name)
#endif


/* Defined separately.  */
extern size_t next_prime (size_t seed);


/* Table entry type.  */
#define _DYNHASHENTTYPE(name) \
  typedef struct name##_ent						      \
  {									      \
    unsigned long int hashval;						      \
    TYPE data;								      \
    NEXT (name)								      \
  } name##_ent
#define DYNHASHENTTYPE(name) _DYNHASHENTTYPE (name)
DYNHASHENTTYPE (NAME);


/* Type of the dynamic hash table data structure.  */
#define _DYNHASHTYPE(name) \
typedef struct								      \
{									      \
  unsigned long int size;						      \
  unsigned long int filled;						      \
  name##_ent *table;							      \
  FIRST	(name)								      \
} name
#define DYNHASHTYPE(name) _DYNHASHTYPE (name)
DYNHASHTYPE (NAME);



#define _FUNCTIONS(name) \
/* Initialize the hash table.  */					      \
extern int name##_init (name *htab, unsigned long int init_size);	      \
									      \
/* Free resources allocated for hash table.  */				      \
extern int name##_free (name *htab);					      \
									      \
/* Insert new entry.  */						      \
extern int name##_insert (name *htab, unsigned long int hval, TYPE data);     \
									      \
/* Insert new entry, possibly overwrite old entry.  */			      \
extern int name##_overwrite (name *htab, unsigned long int hval, TYPE data);  \
									      \
/* Find entry in hash table.  */					      \
extern TYPE name##_find (name *htab, unsigned long int hval, TYPE val);
#define FUNCTIONS(name) _FUNCTIONS (name)
FUNCTIONS (NAME)


#ifdef ITERATE
# define _XFUNCTIONS(name) \
/* Get next element in table.  */					      \
extern TYPE name##_iterate (name *htab, void **ptr);
# define XFUNCTIONS(name) _XFUNCTIONS (name)
XFUNCTIONS (NAME)
#endif

#ifndef NO_UNDEF
# undef DYNHASHENTTYPE
# undef DYNHASHTYPE
# undef FUNCTIONS
# undef _FUNCTIONS
# undef XFUNCTIONS
# undef _XFUNCTIONS
# undef NAME
# undef TYPE
# undef ITERATE
# undef COMPARE
# undef FIRST
# undef NEXT
#endif
