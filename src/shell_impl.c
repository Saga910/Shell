#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_regex.h>
#include <dc_posix/dc_string.h>
#include <dc_error/error.h>
#include "shell_impl.h"
#include "state.h"
#include "util.h"
#include "input.h"
#include "builtins.h"
#include "execute.h"

/**
 * Set up the initial state:
 *  - in_redirect_regex  "[ \t\f\v]<.*"
 *  - out_redirect_regex "[ \t\f\v][1^2]?>[>]?.*"
 *  - err_redirect_regex "[ \t\f\v]2>[>]?.*"
 *  - path the PATH environ var separated into directories
 *  - prompt the PS1 environ var or "$" if PS1 not set
 *  - max_line_length the value of _SC_ARG_MAX (see sysconf)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return READ_COMMANDS or INIT_ERROR
 */

int init_state(const struct dc_posix_env *env, struct dc_error *err, void *arg){
    struct state *s;
    s = (struct state*) arg;
    regex_t regex;
    char **list;
    char *path;
    char *prompt;
    int val = 0;

    val = dc_regcomp(env, err, &regex, "[ \t\f\v]<.*", REG_EXTENDED);
    s->in_redirect_regex = &regex;
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
        s->fatal_error = true;
        return ERROR;
    }


    val = dc_regcomp(env, err, &regex, "[ \t\f\v][1^2]?>[>]?.*", REG_EXTENDED);
    s->out_redirect_regex = &regex;
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
        s->fatal_error = true;
        return ERROR;
    }

    val = dc_regcomp(env, err, &regex, "[ \t\f\v]2>[>]?.*", REG_EXTENDED);
    s->err_redirect_regex = &regex;
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
        s->fatal_error = true;
        return ERROR;
    }

    path = get_path(env, err);
    list = parse_path(env, err, path);

    if(dc_error_has_error(err)){
        s->fatal_error = true;
        return ERROR;
    }
    prompt = get_prompt(env, err);

    s->path = list;
    s->prompt = prompt;
    s->max_line_length = (size_t) dc_sysconf(env, err, _SC_ARG_MAX);
    s->current_line = NULL;
    s->command = NULL;
    s->current_line_length = 0;


    return READ_COMMANDS;
}

/**
 * Free any dynamically allocated memory in the state and sets variables to NULL, 0 or false.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return DC_FSM_EXIT
 */
int destroy_state(const struct dc_posix_env *env, struct dc_error *err, void *arg){
    struct state *s;
    s = (struct state*) arg;

    do_reset_state(env, err, s);

    s->prompt = NULL;
    s->path = NULL;
    s->max_line_length = 0;
    s->in_redirect_regex = NULL;
    s->out_redirect_regex = NULL;
    s->err_redirect_regex = NULL;
    s->current_line = NULL;

//    destroy_command(env, s->command);

    return DC_FSM_EXIT;
}

/**
 * Reset the state for the next read (see do_reset_state).
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return READ_COMMANDS
 */
int reset_state(const struct dc_posix_env *env, struct dc_error *err, void *arg){
    struct state *s;
    s = (struct state*)arg;

    do_reset_state(env, err, s);

    return READ_COMMANDS;
}

/**
 * Prompt the user and read the command line (see read_command_line).
 * Sets the state->current_line and current_line_length.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return SEPARATE_COMMANDS
 */
int read_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg){
    struct state *s;
    s = (struct state *)arg;
    char *path;
    char *input;
    size_t l;
    size_t length = 0;
    path = dc_getcwd(env, err, NULL, 0);

    if(dc_error_has_error(err)){
        s->fatal_error = true;
        return ERROR;
    }

    fprintf(s->stdout, "[%s] %s", path, s->prompt);

    if(dc_error_has_error(err)){
        s->fatal_error = true;
        return ERROR;
    }

    input = read_command_line(env, err, s->stdin, &length);

    if(dc_error_has_error(err)){
        s->fatal_error = true;
        return ERROR;
    }

    s->current_line = dc_strdup(env, err, input);

    if(dc_error_has_error(err)){
        s->fatal_error = true;
        dc_free(env, input, dc_strlen(env, input));
        return ERROR;
    }

    l = dc_strlen(env, input);

    if(l == 0){
        return RESET_STATE;
    }

    s->current_line_length = l;

    if(dc_error_has_error(err)){
        s->fatal_error = true;
        return ERROR;
    }

    return SEPARATE_COMMANDS;
}

/**
 * Separate the commands. In the current implementation there is only one command.
 * Sets the state->command.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return PARSE_COMMANDS or SEPARATE_ERROR
 */
int separate_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg){
    struct state *s;
    s = (struct state *) arg;

    s->command = dc_calloc(env, err, 1, sizeof(*s->command));

    if(dc_error_has_error(err)){
        s->fatal_error = true;
        return ERROR;
    }
    char *cl;
    cl = s->current_line;
    s->command->line = dc_strdup(env, err, cl);

    if(dc_error_has_error(err)){
        s->fatal_error = true;
        return ERROR;
    }

    return PARSE_COMMANDS;
}

/**
 * Parse the commands (see parse_command)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return EXECUTE_COMMANDS or PARSE_ERROR
 */
int parse_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg){
    struct state *s;
    s = (struct state *)arg;

//    parse_command(env, err, s, s->command);

    if(dc_error_has_error(err)){
        s->fatal_error = true;
        return ERROR;
    }

    return EXECUTE_COMMANDS;
}


/**
 * Run the command (see execute).
 * If the command->command is cd run builtin_cd
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return EXIT (if command->command is exit), RESET_STATE or EXECUTE_ERROR
 */
int execute_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg){
    struct state *s;
    s = (struct state *) arg;
    char *path = get_path(env, err);
    char **list = parse_path(env, err, path);

    if(dc_strcmp(env, s->command->command, "cd") == 0){
//        builtin_cd(env, err, );
    } else if (dc_strcmp(env, s->command->command, "exit") == 0){
        return EXIT;
    } else{
//        execute(env, err, s->command, list);
        if(dc_error_has_error(err)){
            s->fatal_error = true;
            return EXIT;
        }
    }



    if(dc_error_has_error(err)){
        s->fatal_error = true;
        return EXIT;
    }

    return RESET_STATE;
}


/**
 * Handle the exit command (see do_reset_state)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return DESTROY_STATE
 */
int do_exit(const struct dc_posix_env *env, struct dc_error *err, void *arg){
    struct state *s;
    s = (struct state *) arg;

    do_reset_state(env, err, s);

    return DESTROY_STATE;
}

/**
 * Print the error->message to stderr and reset the error (see dc_err_reset).
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return RESET_STATE or DESTROY_STATE (if state->fatal_error is true)
 */
int handle_error(const struct dc_posix_env *env, struct dc_error *err, void *arg){
    struct state *s;
    s = (struct state *) arg;

    char *p;
    p = s->current_line;

    if(s->current_line == NULL){
        fprintf(s->stderr, "internal error (%d) %s\n", err->err_code, err->message);
    } else{
        fprintf(s->stderr, "internal error (%d) %s: \"%s\"\n", err->err_code, err->message, p);
    }

    if(s->fatal_error == true){
        return DESTROY_STATE;
    }

    return RESET_STATE;
}
