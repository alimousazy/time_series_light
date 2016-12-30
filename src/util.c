#include "util.h"
int util_process_line(char *item, char **parts, char *sep, int n_parts) {
  int i ;
  char *cursoer = NULL;
  for(i = 0; i < n_parts ; i++) {
    char *str; 
     if( i == 0) {
       str = strtok_r(item, ":", &cursoer);
     } else  {
       str = strtok_r(NULL, ":", &cursoer);
     }
     if(!str) {
//printf("Can't break up string\n");
       break;
     }
     parts[i] = str;
  }
  return i;
}
