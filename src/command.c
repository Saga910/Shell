#include <dc_posix/dc_regex.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_wordexp.h>
#include "command.h"
#include "shell_impl.h"
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
    int match_s;
    const char *cmd;
    cmd = dc_strdup(env, err, command->line);
    size_t length = 0;

    val = dc_regcomp(env, err, &regex, "[ \t\f\v]2>[>]?.*", REG_EXTENDED);
    state->err_redirect_regex = &regex;
    error_r(env, err, val, regex);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }
    char *left, *error = NULL;
    match_s = dc_regexec(env, &regex, cmd, 1, &match, 0);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    if(match_s == 0){
        char *string;

        regoff_t prog = match.rm_eo - match.rm_so;
        string = dc_malloc(env, err, ((size_t) (prog + 1)));
        dc_strncpy(env, string, &cmd[match.rm_so], (size_t) prog);
        string[prog] = '\0';
        length = dc_strlen(env,string);
        char *token = dc_strtok(env,string, ">");
        while (token != NULL)
        {
            error = dc_strdup(env, err, token);
            token = dc_strtok(env, NULL, ">");
        }
        error = trim(error);
        free(string);
    }

    size_t l = dc_strlen(env,cmd);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    left = dc_malloc(env, err, l - length);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    dc_strncpy(env,left, cmd, l - length);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    if(error != NULL){
        state->command->stderr_overwrite = true;
    }

    val = dc_regcomp(env, err, &regex, "[ \t\f\v][1^2]?>[>]?.*", REG_EXTENDED);
    state->out_redirect_regex = &regex;
    error_r(env, err, val, regex);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    char *out;
    match_s = dc_regexec(env, &regex, left, 1, &match, 0);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    if(match_s == 0){
        char *string;

        regoff_t prog = match.rm_eo - match.rm_so;
        string = dc_malloc(env, err, ((size_t) (prog + 1)));
        dc_strncpy(env, string, &left[match.rm_so], (size_t) prog);
        string[prog] = '\0';
        length = dc_strlen(env,string);
        char *token = dc_strtok(env,string, ">>");
        while (token != NULL)
        {
            out = dc_strdup(env, err, token);
            token = dc_strtok(env, NULL, ">>");
        }
        out = trim(out);
        free(string);
    }
    l = dc_strlen(env,left);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    char *second = dc_malloc(env, err, l - length);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    strncpy(second, left, l -length);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }


    val = dc_regcomp(env, err, &regex, "[ \t\f\v]<.*", REG_EXTENDED);
    state->in_redirect_regex = &regex;

    error_r(env, err, val, regex);

    if(dc_error_has_error(err)){
        state->fatal_error = true;
    }

    char *in;
    match_s =  dc_regexec(env, &regex, second, 1, &match, 0);

    if(match_s == 0){
        char *string;

        regoff_t prog = match.rm_eo - match.rm_so;
        string = dc_malloc(env, err, ((size_t) (prog + 1)));
        dc_strncpy(env, string, &left[match.rm_so], (size_t) prog);
        string[prog] = '\0';
        length = dc_strlen(env,string);
        char *token = dc_strtok(env,string, "<");
        while (token != NULL)
        {
            in = dc_strdup(env, err, token);
            token = dc_strtok(env, NULL, "<");
        }
        in = trim(in);
        free(string);
    }
    char *com_line;
    l = dc_strlen(env,second);
    com_line = dc_malloc(env, err,l - length);
    dc_strncpy(env, com_line, second, l-length);



}

char* trim(const char* str)
{
    static char str1[99];
    int count = 0, j, k;

    while (str[count] == ' ') {
        count++;
    }

    for (j = count, k = 0;
         str[j] != '\0'; j++, k++) {
        str1[k] = str[j];
    }
    str1[k] = '\0';

    return str1;
}


/**
 *
 * @param env
 * @param command
 */
void destroy_command(const struct dc_posix_env *env, struct command *command){
    if(command != NULL){
        command->stdout_file = NULL;
        command->stderr_file = NULL;
        command->stdin_file = NULL;

        free(command->command);
        command->command = NULL;

        for (size_t i =0; i < (command->argc); i++){
            if(command->argv[i]){
                free(command->argv[i]);
                command->argv[i] = NULL;
            }
        }

        command->argc = 0;

        dc_free(env, command->line, sizeof(command->line));
        command->line = NULL;

        command->exit_code = 0;

        command->stdout_overwrite = false;

        command->stderr_overwrite = false;
    }
}
