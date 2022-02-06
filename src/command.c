#include <dc_posix/dc_regex.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_wordexp.h>
#include "command.h"
/**
 * Parse the command. Take the command->line and use it to fill in all of the fields.
 *
 * @param env the posix environment.
 * @param err the error object.
 * @param state the current state, to set the fatal_error and access the command line and regex for redirection.
 * @param command the command to parse.
 */

void parse_command(const struct dc_posix_env *env, struct dc_error *err, struct state *state, struct command *command){

}

regex_t *parse(const struct dc_posix_env *env, struct dc_error *err, int a){
//    int val = 0;
//    regmatch_t ls;
//
//    if(a == 0){
//        val = dc_regexec(env, &regex, ".err", 1, &ls, 0);
//    } else if(a == 1){
//        val = dc_regexec(env, &regex, ">>", 1, &ls, 0);
//    } else if(a == 2){
//        val = dc_regexec(env, &regex, ">>", 1, &ls, 0);
//    }
//
//    if(val != 0){
//        char *message;
//        size_t t;
//        t = dc_regerror(env, val, &regex, NULL, 0);
//        message = dc_malloc(env, err, t + 1);
//        dc_regerror(env, val, &regex, message, t + 1);
//        fprintf(stderr, "%s", message);
//        dc_free(env, err, dc_strlen(env, message) + 1);
//    }
//
//    return &regex;
}

/**
 *
 * @param env
 * @param command
 */
void destroy_command(const struct dc_posix_env *env, struct command *command){

}