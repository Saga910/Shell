#include <bits/types/FILE.h>
#include <dc_posix/dc_stdio.h>
#include <dc_util/strings.h>
#include <dc_posix/dc_string.h>
#include "input.h"

/**
 * Read the command line from the user.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param stream The stream to read from (eg. stdin)
 * @param line_size the maximum characters to read.
 * @return The command line that the user entered.
 */
char *read_command_line(const struct dc_posix_env *env, struct dc_error *err, FILE *stream, size_t *line_size){
    char *ans;
    unsigned long size;

    size = (unsigned long) *line_size + 1;

    ans = dc_malloc(env, err, size * sizeof(char));

    dc_getline(env, err, &ans, line_size, stream);

    dc_str_trim(env, ans);

    *line_size = dc_strlen(env, ans);

    return ans;
}
