




int string_to_argv(char* commandline, char*** argv);
int string_to_argv_noalloc(const char* commandline, char** argv, int argv_size);
char* pathname_get_path(const char* pathname);
char* pathname_get_name(const char* pathname);

int string_is_number(const char* string);
char* string_to_lower(const char* string);

int get_token_from_pathname(const char* pathname, int offset, char** result);
  /*copies section of pathname from offset to the next slash to
    result, returns length of section*/
