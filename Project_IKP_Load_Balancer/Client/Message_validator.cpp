#include "message_validator.h"
#include <string.h>
#include <ctype.h>

int is_valid_message(const char* message) {
    int has_alphanumeric = 0;

    for (size_t i = 0; i < strlen(message); ++i) {
        char c = message[i];

        if (c == '?' || c == '!' || c == ':' || c == ';') {
            return 0; 
        }

        if (isalnum(c) && c != ' ') {
            has_alphanumeric = 1;
        }
    }

    return has_alphanumeric;
}