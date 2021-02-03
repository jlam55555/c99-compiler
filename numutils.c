#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "numutils.h"

void print_number(struct number num){
    if(num.type==INT_T || num.type==LONG_T || num.type==LONGLONG_T)
        fprintf(stdout, "INTEGER\t %lld\t", num.int_val);
    else
        fprintf(stdout, "REAL\t %Lg\t", num.real_val);
    if(num.sign==UNSIGNED_T)
        fprintf(stdout, "UNSIGNED,");
    switch(num.type){
        case INT_T:         fprintf(stdout, "INT\n"); break;
        case LONG_T:        fprintf(stdout, "LONG\n"); break;
        case LONGLONG_T:    fprintf(stdout, "LONGLONG\n"); break;    
        case DOUBLE_T:      fprintf(stdout, "DOUBLE\n"); break;
        case LONGDOUBLE_T:  fprintf(stdout, "LONGDOUBLE\n"); break;
        case FLOAT_T:       fprintf(stdout, "FLOAT\n"); break;
        default:            break;
    }

}