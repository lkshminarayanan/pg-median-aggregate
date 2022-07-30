#ifndef COMPARATORS_H
#define COMPARATORS_H

#include <postgres_ext.h>

void	   *get_comparator(Oid datum_type);

#endif   /* COMPARATORS_H */
