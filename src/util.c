#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdio.h>
#include <bits/types/FILE.h>
#include "util.h"
#include "command.h"

/**
 * Get the prompt to use.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return value of the PS1 environ var or "$ " if PS1 not set.
 */
char *get_prompt(const struct dc_posix_env *env, struct dc_error *err){
    char *prompt = dc_getenv(env,"PS1");
    const char *def = "$ ";

    if(!prompt){
        return dc_strdup(env,err, def);
    } else{
        return dc_strdup(env, err, prompt);
    }
}

/**
 * Get the PATH environ var.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return the PATH environ var
 */
char *get_path(const struct dc_posix_env *env, struct dc_error *err){
    char *path = dc_getenv(env, "PATH");

    if(!path){
        return NULL;
    } else{
        return dc_strdup(env, err, path);
    }
}
/**
 * Separate a path (eg. PATH environ var) into separate directories.
 * Directories are separated with a ':' character.
 * Any directories with ~ are converted to the users home directory.
 *
 * @param env the posix environment.
 * @param err the error object.
 * @param path_str the string to separate.
 * @return The directories that make up the path.
 */
char **parse_path(const struct dc_posix_env *env, struct dc_error *err, const char *path_str){
    char *s = dc_strdup(env, err, path_str);
    char *tok, *path;
    char **list;
    size_t num, i;

    path = s;
    num = count(path_str, ':');
    i = 0;
    list = dc_calloc(env, err, num + 2, sizeof(char *));
    while ((tok = dc_strtok_r(env, path, ":", &path)) != NULL){
        list[i] = dc_strdup(env, err, tok);
        i++;
    }

    list[i] = NULL;
    dc_free(env, s, sizeof(char *));

    return list;
}

/**
 * Counts the number of elements to put in the path array.
 * @param str path string.
 * @param c delimiter.
 * @return the number of elements divided by c.
 */
size_t count(const char *str, int  c){
    size_t num;

    num = 0;

    for(const char *temp = str; *temp; temp++){
        if(*temp == c){
            num++;
        }
    }

    return num;
}


/**
 * Reset the state for the next read, freeing any dynamically allocated memory.
 *
 * @param env the posix environment.
 * @param err the error object
 */
void do_reset_state(const struct dc_posix_env *env, struct dc_error *err, struct state *state){
    dc_free(env, state->current_line, state->current_line_length);
    state->current_line = NULL;
    state->current_line_length = 0;

    state->fatal_error = false;

    dc_error_reset(err);

    dc_free(env, state->command, sizeof(struct command));
    state->command = NULL;



//    dc_free(env, state->command->line, dc_strlen(env, state->command->line) + 1);
//    state->command->line = NULL;

//    for (size_t i =0; i < state->command->argc; i++){
//        if(state->command->argv[i]){
//            dc_free(env, state->command->argv[i], dc_strlen(env, state->command->line) +1);
//        }
//    }
//
//    dc_free(env, state->command->command, dc_strlen(env, state->command->command) +1);
//
//    state->command->exit_code = 0;
//    state->command->stderr_overwrite = false;
//    state->command->stdout_overwrite = false;
//
//    dc_free(env, state->command->stderr_file, dc_strlen(env, state->command->stdin_file) +1);
//    dc_free(env, state->command->stderr_file, dc_strlen(env, state->command->stdout_file) +1);
//    dc_free(env, state->command->stderr_file, dc_strlen(env, state->command->stderr_file) +1);

}

/**
 * Display the state values to the given stream.
 *
 * @param env the posix environment.
 * @param state the state to display.
 * @param stream the stream to display the state on,
 */
void display_state(const struct dc_posix_env *env, const struct state *state, FILE *stream){
    char *str;
    struct dc_error *err;

    str = state_to_string(env, err, state);
    fprintf(stream, "%s\n", str);
    dc_free(env,str, sizeof(char *));
}


/**
 * Display the state values to the given stream.
 *
 * @param env the posix environment.
 * @param state the state to display.
 * @param stream the stream to display the state on,
 */
char *state_to_string(const struct dc_posix_env *env,  struct dc_error *err, const struct state *state){
    size_t len;
    char *line;

    if(state->current_line == NULL){
        len = dc_strlen(env, "current_line = NULL");
    }else{
        len = dc_strlen(env, "current_line = ");
        len += state->current_line_length;
        len += dc_strlen(env, ", current_line_length = ");
    }

    len += dc_strlen(env, ", fatal_error = ");
    line = dc_malloc(env, err, len + 1);

    if(state->current_line == NULL){
        sprintf(line, "current_line = NULL, fatal_error = %d", state->fatal_error);
    } else{
        sprintf(line, "current_line = \"%s\", fatal_error = %d", state->current_line, state->fatal_error);
    }

    return line;
}
