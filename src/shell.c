#include "shell.h"
#include "shell_impl.h"
#include "state.h"
#include "shell_impl.h"

/**
 * Run the shell FSM.
 *
 * @param env the posix environment.
 * @param error the error object
 * @param in the keyboard (stdin) file
 * @param out the keyboard (stdout) file
 * @param err the keyboard (stderr) file
 *
 * @return the exit code from the shell.
 */
int run_shell(const struct dc_posix_env *env, struct dc_error *error, FILE *in, FILE *out, FILE *err){
    int ret_val = 0;

    static struct dc_fsm_transition transitions[] = {
            {DC_FSM_INIT, INIT_STATE, init_state},
            {INIT_STATE, READ_COMMANDS, read_commands},
            {INIT_STATE, ERROR, handle_error},
            {READ_COMMANDS, RESET_STATE, reset_state},
            {READ_COMMANDS, SEPARATE_COMMANDS, separate_commands},
            {READ_COMMANDS, ERROR, handle_error},
            {SEPARATE_COMMANDS, PARSE_COMMANDS, parse_commands},
            {SEPARATE_COMMANDS, ERROR, handle_error},
            {PARSE_COMMANDS, EXECUTE_COMMANDS, execute_commands},
            {PARSE_COMMANDS, ERROR, handle_error},
            {EXECUTE_COMMANDS, RESET_STATE, reset_state},
            {EXECUTE_COMMANDS, EXIT, do_exit},
            {EXECUTE_COMMANDS, ERROR, handle_error},
            {RESET_STATE, READ_COMMANDS, read_commands},
            {EXIT, DESTROY_STATE, destroy_state},
            {ERROR, RESET_STATE, reset_state},
            {ERROR, DESTROY_STATE, destroy_state},
            {DESTROY_STATE, DC_FSM_EXIT, NULL}

    };

    struct dc_fsm_info *info;

    info = dc_fsm_info_create(env, error, "shell");

    if(dc_error_has_no_error(error)){
        int from;
        int to;
        ret_val = dc_fsm_run(env, error, info, &from, &to, in, transitions);
        dc_fsm_info_destroy(env,&info);
    }

    return ret_val;
}
