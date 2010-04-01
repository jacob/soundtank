#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include.h"



/*a function to parse a string into substrings separating by spaces*/
/*note that i don't use this because it has a bug i never fixed*/
int string_to_argv(char* commandline, char*** argv){
  int numargs,i,token_size;
  char** new_argv;
  char* curr;
  char* token;
  char debch;

  numargs = 0;

  /*first find how many arguments there are*/
  curr = commandline; 
  while (*curr == ' ') ++curr;  /*ignore leading whitespace*/
  if (strlen(curr)) ++numargs;

  for ( ; *curr != '\0'; ++curr){
    
    debch = *curr;

    if (*curr == ' '){

      ++numargs;
      while (*curr == ' ') ++curr;

    }

  }

 

  /*allocate argv array*/
  if (!(new_argv = (char**)malloc(numargs*sizeof(char*)))) return -1;

  /*fill argv array*/
  i = 0;
  curr = commandline; 
  while (*curr == ' ') ++curr;  /*ignore leading whitespace*/
  for ( ; *curr != '\0'; ++curr){
    
    if (*curr == ' '){

      while (*curr == ' ') ++curr;

    }

    if (*curr != '\0'){

      token = curr;
      token_size = 0;

      while ((*curr != ' ')&&(*curr != '\0')){
	++token_size;
	++curr;
      }

      
      if (!(new_argv[i] = (char*)malloc((2+token_size)*sizeof(char)))){
	for (--i ;i>=0;--i) free(new_argv[i]);
	free(new_argv);
	return -1;
      }

      memcpy(new_argv[i],token,token_size);
      //strncpy((void*)new_argv[i],(void*)token,token_size);
      
      /*must manually add null char when using strncypy*/
      new_argv[i][1+strlen(new_argv[i])] = '\0';
      
      printf("DEBUG:arg=%s,len=%d\n",new_argv[i],token_size);


      ++i;
      
    }

  }

  *argv = new_argv;
  return numargs;
}









  
/*a function to parse a string into substrings separating by spaces*/
/*this function does no memory allocation*/
int string_to_argv_noalloc(const char* commandline, char** argv, int argv_size){
  int i,token_size;
  const char* curr;
  const char* token;


  /*fill argv array*/
  i = 0;
  curr = commandline; 

  for ( ; *curr != '\0'; ){
    
    if (*curr == ' '){

      while (*curr == ' ') ++curr;

    }

    if (*curr != '\0'){

      token = curr;
      token_size = 0;

      while ((*curr != ' ')&&(*curr != '\0')){
	++token_size;
	++curr;
      }

      if (i<argv_size){

	memcpy(argv[i],token,token_size);
	//strncpy((void*)argv[i],(void*)token,token_size);
      
	/*must manually add null char when using strncypy*/
	argv[i][token_size] = '\0';
      
	++i;

	/*printf("DEBUG:arg=%s,len=%d\n",argv[i],token_size);*/


      }

      
    }

  }

  return i;
}
  


char* pathname_get_path(const char* pathname){
  /*a bloatware function to allocate a string w/ the path part of a pathname*/
  int i,last_slash;
  char *ret;

  /*find last slash*/
  last_slash = -1;
  for (i=(strlen(pathname))-1;i>=0;--i){
    if (pathname[i] == '/'){
      last_slash = i;
      break;
    }
  }
  
  if (last_slash == -1) return strdup(pathname);
  if (last_slash == 0) return strdup("/");

  /*allocate new string*/
  if (!(ret = (char*)malloc((last_slash + 1)  * sizeof(char))))    return 0;

  snprintf(ret,last_slash + 1,"%s",pathname);

  return ret;
}

char* pathname_get_name(const char* pathname){
  /*a bloatware function to allocate a string w/ the name part of a pathname*/
  int i,last_slash;
  char *ret;
 
  /*find last slash*/
  last_slash = -1;
  for (i=(strlen(pathname))-1;i>=0;--i){
    if (pathname[i] == '/'){
      last_slash = i;
      break;
    }
  }
  
  if (last_slash == -1) return strdup(pathname);

  if (!(ret = strdup(&pathname[last_slash + 1]))) return 0;

  return ret;
}


int string_is_number(const char* string){
  int i;

  for (i=0;i<strlen(string);++i){
    if ((!(isdigit(string[i])))&&(string[i] != '.'))
      return 0;
  }

  return 1;
}

char* string_to_lower(const char* string){
  char *lower_string;
  int i;

  if (!(lower_string = (char*)malloc(1+strlen(string)*sizeof(char)))){
      printf("memory error\n");
      return 0 ;
  }

  for (i=0;i<strlen(string);++i)
    lower_string[i] = (char)tolower(string[i]);
  lower_string[strlen(string)] = '\0';

  return lower_string;
}


/*moved from unique name when that was deleted*/
int get_token_from_pathname(const char* pathname, int offset, char** result){
  /*copies section of pathname from offset to the next slash to result, returns length of section*/
  int result_len,i;

  result_len = 0;
 
  for (i=0;i<strlen(pathname);++i){
    if ((pathname[offset+i] != '\0')&&(pathname[offset+i] != '/')) ++result_len;
    else break;
  } 

  if (!(*result = (char*)malloc((result_len + 1) * sizeof(char)))){
    return -1;
  }

  strncpy(*result,&pathname[offset],result_len);
  (*result)[result_len] = '\0';

  return result_len;
}
