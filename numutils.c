#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "numutils.h"

void print_number(struct number num){
    if(num.type==INT_T || num.type==LONG_T || num.type==LONGLONG_T)
        fprintf(stdout, "INTEGER %lld ", num.int_val);
    else
        fprintf(stdout, "REAL %Lg ", num.real_val);
    if(num.sign==UNSIGNED_T)
        fprintf(stdout, "UNSIGNED,");
    switch(num.type){
        case INT_T:         fprintf(stdout, "INT"); break;
        case LONG_T:        fprintf(stdout, "LONG"); break;
        case LONGLONG_T:    fprintf(stdout, "LONGLONG"); break;    
        case DOUBLE_T:      fprintf(stdout, "DOUBLE"); break;
        case LONGDOUBLE_T:  fprintf(stdout, "LONGDOUBLE"); break;
        case FLOAT_T:       fprintf(stdout, "FLOAT"); break;
        default:            break;
    }

}