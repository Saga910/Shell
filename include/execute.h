#ifndef DC_SHELL_EXECUTE_H
#define DC_SHELL_EXECUTE_H

/*
 * This file is part of dc_shell.
 *
 *  dc_shell is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with dc_shell.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "command.h"
#include <dc_posix/dc_posix_env.h>
#include <stdio.h>

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
void execute(const struct dc_posix_env *env, struct dc_error *err, struct command *command, char **path);

/**
 *
 * @param err
 * @return
 */
int handle_run_error(struct dc_error *err);

/**
 *
 * @param env
 * @param err
 * @param command
 */
void redirect(const struct dc_posix_env *env, struct dc_error *err, struct command *command);

/**
 *
 * @param env
 * @param err
 * @param command
 * @param path
 */
void run(const struct dc_posix_env *env, struct dc_error *err, struct command *command, char **path);

#endif // DC_SHELL_EXECUTE_H
