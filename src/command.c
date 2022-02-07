#include <dc_posix/dc_regex.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_wordexp.h>
#include "command.h"
#include "shell.h"
/**
 * Parse the command. Take the command->line and use it to fill in all of the fields.
 *
 * @param env the posix environment.
 * @param err the error object.
 * @param state the current state, to set the fatal_error and access the command line and regex for redirection.
 * @param command the command to parse.
 */

void parse_command(const struct dc_posix_env *env, struct dc_error *err, struct state *state, struct command *command){
    regex_t regex;
    regmatch_t match;
    int val = 0;
    int matches;

    val = dc_regcomp(env, err, &regex, "[ \t\f\v]<.*", REG_EXTENDED);
    state->in_redirect_regex = &regex;
    if(val != 0) {
        char *message;
        size_t t;

        t = dc_regerror(env, val, &regex, NULL, 0);
        message = dc_malloc(env, err, t + 1);
        dc_regerror(env, val, &regex, message, t + 1);
        fprintf(stderr, "%s", message);
        dc_free(env, err, dc_strlen(env, message) + 1);
    }

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    matches = dc_regexec(env, &regex, "a", 1, &match, 0);

    dc_regfree(env, &regex);
    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }



    val = dc_regcomp(env, err, &regex, "[ \t\f\v][1^2]?>[>]?.*", REG_EXTENDED);
    state->out_redirect_regex = &regex;
    if(val != 0){
        char *message;
        size_t t;

        t = dc_regerror(env, val, &regex, NULL, 0);
        message = dc_malloc(env, err, t + 1);
        dc_regerror(env, val, &regex, message, t + 1);
        fprintf(stderr, "%s", message);
        dc_free(env, err, dc_strlen(env, message) + 1);
    }
    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    val = dc_regcomp(env, err, &regex, "[ \t\f\v]2>[>]?.*", REG_EXTENDED);
    state->err_redirect_regex = &regex;
    if(val != 0){
        char *message;
        size_t t;

        t = dc_regerror(env, val, &regex, NULL, 0);
        message = dc_malloc(env, err, t + 1);
        dc_regerror(env, val, &regex, message, t + 1);
        fprintf(stderr, "%s", message);
        dc_free(env, err, dc_strlen(env, message) + 1);
    }
    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

}

/**
 *
 * @param env
 * @param command
 */
void destroy_command(const struct dc_posix_env *env, struct command *command){
    free(command->stdout_file);
    command->stdout_file = NULL;

    free(command->stderr_file);
    command->stderr_file = NULL;

    free(command->stdin_file);
    command->stdin_file = NULL;

    free(command->command);
    command->command = NULL;

    for (size_t i =0; i < (command->argc); i++){
        if(command->argv[i]){
            free(command->argv[i]);
            command->argv[i] = NULL;
        }
    }

    free(command->line);
    command->line = NULL;

    command->argc = 0;

    command->exit_code = 0;

    command->stdout_overwrite = false;

    command->stderr_overwrite = false;
}
