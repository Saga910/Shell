#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_unistd.h>
#include <dc_util/filesystem.h>
#include <dc_util/path.h>
#include <dc_posix/dc_string.h>
#include "builtins.h"

/**
 * Change the working directory.
 * ~ is converted to the users home directory.
 * - no arguments is converted to the users home directory.
 * The command->exit_code is set to 0 on success or error->errno_code on failure.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param command the command information
 * @param errstream the stream to print error messages to
 */
void builtin_cd(const struct dc_posix_env *env, struct dc_error *err, struct command *command, FILE *errstream){
    char *path;
    int val = 0;

    if(command->argv[1] == NULL) {
        char *s;

        dc_expand_path(env, err, &s, "~/");

        dc_chdir(env, err, s);

        path = dc_strdup(env, err, s);
    } else{

        dc_chdir(env, err, command->argv[1]);

        path = dc_strdup(env, err, command->argv[1]);
    }

    if(dc_error_has_error(err)){
        if(dc_error_is_errno(err, EACCES)) {
            fprintf(errstream, "%s: Permission denied\n", path);
        }
        if(dc_error_is_errno(err, ELOOP)) {
            fprintf(errstream, "%s: Too many symbolic links encountered\n", path);
        }
        if(dc_error_is_errno(err, ENAMETOOLONG)){
            fprintf(errstream, "%s: File name too long\n", path);
        }
        if(dc_error_is_errno(err, ENOENT)){
            fprintf(errstream, "%s: does not exist\n", path);
        }
        if(dc_error_is_errno(err, ENOTDIR)){
            fprintf(errstream, "%s: is not a directory\n", path);
        }
        command->exit_code = 1;
    } else{
        command->exit_code = 0;
    }
}

