#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_fcntl.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdio.h>
#include <sys/wait.h>
#include "execute.h"

/**
 * Create a child process, exec the command with any redirection, set the exit code.
 * If there is an err executing the command print an err message.
 * If the command cannot be found set the command->exit_code to 127.
 *
 * @param env the posix environment.
 * @param err the err object
 * @param command the command to execute
 * @param path the directories to search for the command
 */
void execute(const struct dc_posix_env *env, struct dc_error *err, struct command *command, char **path){
    pid_t f;
    int s = 0;

    f = fork();

    if(f == 0){
        redirect(env, err, command);

        if(dc_error_has_error(err)){
            dc_exit(env, 126);
        }
        run(env, err, command, path);
        s = handle_run_error(err);
        dc_exit(env, s);
    } else {
        waitpid(f, &s, 0);
        if(WIFEXITED(s)){
            int num;
            num = WEXITSTATUS(s);
            command->exit_code = num;
        }
    }

}


int handle_run_error(struct dc_error *err){

    if(dc_error_is_errno(err, E2BIG)){
        fprintf(stderr, "Argument list too long");
        return 1;
    }
    else if(dc_error_is_errno(err, EACCES)){
        fprintf(stderr, "Permission denied");
        return 2;
    }
    else if(dc_error_is_errno(err,EINVAL)){
        fprintf(stderr, "Invalid argument");
        return 3;
    }
    else if(dc_error_is_errno(err,ELOOP)){
        fprintf(stderr, "Too many symbolic links encountered");
        return 4;
    }
    else if(dc_error_is_errno(err,ENAMETOOLONG)){
        fprintf(stderr, "File name too long");
        return 5;
    }
    else if(dc_error_is_errno(err,ENOENT)){
        fprintf(stderr, "No such file or directory");
        return 127;
    }
    else if(dc_error_is_errno(err,ENOTDIR)){
        fprintf(stderr, "Not a directory");
        return 6;
    }
    else if(dc_error_is_errno(err,ENOEXEC)){
        fprintf(stderr, "Exec format error");
        return 7;
    }
    else if(dc_error_is_errno(err,ENOMEM)){
        fprintf(stderr, "Out of memory");
        return 8;
    }
    else if(dc_error_is_errno(err,ETXTBSY)){
        fprintf(stderr, "Text file busy");
        return 9;
    } else{
        return 125;
    }
}

void redirect(const struct dc_posix_env *env, struct dc_error *err, struct command *command){

    if(command->stdin_file != NULL){
        int d = dc_open(env, err, command->stdin_file, O_RDWR);

        if(dc_error_has_error(err)){
            dc_close(env, err, d);
        }

        dc_dup2(env, err, d, STDIN_FILENO);

        dc_close(env, err, d);

    } else if(command->stdout_file != NULL){
        int d;

        if(command->stdout_overwrite == true){
            d = dc_open(env, err, command->stdout_file, O_APPEND | O_CREAT);
            if(dc_error_has_error(err)){
                dc_close(env, err, d);
            }
        } else{
            d = dc_open(env, err, command->stdout_file, O_TRUNC | O_CREAT);
            if(dc_error_has_error(err)){
                dc_close(env, err, d);
            }
        }

        dc_dup2(env, err, d, STDOUT_FILENO);

        dc_close(env, err, d);

    } else if(command->stderr_file != NULL){
        int d;

        if(command->stderr_overwrite == true){
            d = dc_open(env, err, command->stderr_file, O_APPEND | O_CREAT);
            if(dc_error_has_error(err)){
                dc_close(env, err, d);
            }

        } else{
            d = dc_open(env, err, command->stderr_file, O_TRUNC | O_CREAT);
            if(dc_error_has_error(err)){
                dc_close(env, err, d);
            }

        }
        dc_dup2(env, err, d, STDERR_FILENO);

        dc_close(env, err, d);
    }
}

void run(const struct dc_posix_env *env, struct dc_error *err, struct command *command, char **path){

    if((dc_strchr(env, command->command, '/') != NULL)){
        command->argv[0] = dc_strdup(env, err, command->command);
        dc_execv(env, err, command->command, command->argv);
    } else{
        if(path[0] == NULL){
            DC_ERROR_RAISE_ERRNO(err, ENOENT);
        } else{
            for(char *str = *path; str; *++path){
                char *cmd = dc_strdup(env,err, dc_strcat(env, dc_strcat(env, str, "/"), command->command));
                command->argv[0] = dc_strdup(env, err, cmd);
                dc_execv(env, err, cmd, command->argv);
                if(err->errno_code != ENOENT){
                    break;
                }
            }
        }
    }
}



