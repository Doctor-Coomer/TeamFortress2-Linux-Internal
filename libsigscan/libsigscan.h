/*
 * libsigscan.h - Simple C/C++ library for signature scanning on Linux.
 * See: https://github.com/8dcc/libsigscan
 * Copyright (C) 2024 8dcc
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBSIGSCAN_H_
#define LIBSIGSCAN_H_ 1

#include <stddef.h> /* NULL */

enum ESigscanPidType {
    SIGSCAN_PID_INVALID = -2, /* Invalid PID, should be ignored */
    SIGSCAN_PID_SELF    = -1, /* We want to search in our own modules */
};

/*
 * Start and end address of a readable memory chunk in the process. See the
 * `sigscan_get_module_bounds' function.
 */
typedef struct SigscanModuleBounds {
    void* start;
    void* end;
    struct SigscanModuleBounds* next;
} SigscanModuleBounds;

/*----------------------------------------------------------------------------*/

/*
 * Get the PID of the first process that matches `process_name'.
 */
int sigscan_pidof(const char* process_name);

/*
 * Parse the `/proc/PID/maps' file to get the start and end addresses of the
 * specified module. The `regex' argument should be either NULL (don't filter,
 * return all modules) or a regular expression with ERE syntax. For more
 * information on ERE vs. BRE, see:
 * https://www.gnu.org/software/sed/manual/html_node/BRE-vs-ERE.html
 *
 * Returns a linked list of `SigscanModuleBounds' structures, which must be
 * freed by the caller using `sigscan_free_module_bounds'.
 *
 * The function assumes the format of maps is always:
 *   0000DEADBEEF-0000ABADCAFE rwxp 000123AB 100:00 12345  /lib/my path/foo.so
 *
 * Each line is expected to match at least 4 of the 5 fields in the `sscanf'
 * format below. The last one (pathname) is optional and the line will be
 * skipped if empty.
 */
SigscanModuleBounds* sigscan_get_module_bounds(int pid, const char* regex);

/*
 * Free a linked list of `SigscanModuleBounds' structures.
 */
void sigscan_free_module_bounds(SigscanModuleBounds* bounds);

/*
 * Search for `ida_pattern' in the modules loaded by the process with the
 * specified `pid', whose name matches `regex'.
 *
 * If `pid' is SIGSCAN_PID_SELF, it searches in the current process.
 * If `regex' is NULL, it searches in all loaded modules.
 */
void* sigscan_pid_module(int pid, const char* regex, const char* ida_pattern);

/*
 * Search for `ida_pattern' in all the loaded modules of `pid'.
 */
static inline void* sigscan_pid(int pid, const char* ida_pattern) {
    return sigscan_pid_module(pid, NULL, ida_pattern);
}

/*
 * Search for `ida_pattern' in all the modules loaded by the current process,
 * whose name matches `regex'.
 */
static inline void* sigscan_module(const char *regex, const char *ida_pattern) {
    return sigscan_pid_module(SIGSCAN_PID_SELF, regex, ida_pattern);
}

/*
 * Search for `ida_pattern' in all the modules loaded by the current process.
 */
static inline void* sigscan(const char* ida_pattern) {
    return sigscan_pid_module(SIGSCAN_PID_SELF, NULL, ida_pattern);
}

#endif /* LIBSIGSCAN_H_ */
