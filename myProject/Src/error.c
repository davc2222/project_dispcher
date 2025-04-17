#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @function to write error nessages to terminal.
 *
 * receive bool to indecate succses or fail 
 * and string for the discription of the failure
 * return void
 * terminate the program if condition false
 */

void my_assert(bool condition, const char *error_message)
{
    if (!condition)
    {
        fprintf(stderr, "%s\n", error_message);
        exit(EXIT_FAILURE);
    }
}