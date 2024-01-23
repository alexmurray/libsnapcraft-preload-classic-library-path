/* -*- c-indentation-style: linux; c-ts-mode-indent-style: linux; c-ts-mode-indent-offset: 8; -*- */
#define _GNU_SOURCE 1

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char *(*real_getenv)(const char *) = NULL;

static const char *env_vars[] = {
        "LD_LIBRARY_PATH",
        // TODO: PYTHONPATH etc.
        NULL
};

static int name_in_env_vars(const char *name) {
        const char **var;
        for (var = env_vars; *var != NULL; var++) {
                if (strcmp(*var, name) == 0) {
                        return 1;
                }
        }
        return 0;
}

static int self_in_snap(void) {
        char path[1024] = {0};

        int res = readlink("/proc/self/exe", path, sizeof(path));
        if (res > 0) {
                // since we can't easily know where snap's are mounted on the system,
                // instead we just check if the path contains "/snap/".
                return strstr(path, "/snap/") != NULL;
        }
        return 0;
}

char *getenv(const char *name) {
        printf("getenv(%s)\n", name);
        if (!real_getenv) {
                real_getenv = dlsym(RTLD_NEXT, "getenv");
                assert(real_getenv != NULL);
        }
        if (name != NULL && name_in_env_vars(name) && !self_in_snap()) {
                return NULL;
        } else {
                return real_getenv(name);
        }
}
