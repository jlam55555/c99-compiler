#include "astnode.h"
#include "asttypes.h"

void mergedeclspec(union astnode *spec1, union astnode *spec2) {
	struct astnode_declspec ds1 = spec1->declspec,
		ds2 = spec2->declspec;
	/*merge typespecs*/
	if (ds1.ts) {
		
	} else {

	}
	/*merge typequals*/
	/*merge scspecs*/
	free(spec2);
}