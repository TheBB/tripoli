#include <stdio.h>
#include <string.h>

#include "interface.h"


static emacs_env *env;

void set_environment(emacs_env *__env)
{
    env = __env;
}

void em_provide(char *feature_name)
{
    emacs_value provide_fcn = em_intern("provide");
    emacs_value args[] = {env->intern(env, feature_name)};
    env->funcall(env, provide_fcn, 1, args);
}

void em_message(char *msg)
{
    emacs_value message_fcn = em_intern("message");
    emacs_value args[] = {env->make_string(env, msg, strlen(msg))};
    env->funcall(env, message_fcn, 1, args);
}

emacs_value em_intern(char *name)
{
    return env->intern(env, name);
}

bool em_null(emacs_value val)
{
    return !env->is_not_nil(env, val);
}

emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args)
{
    return env->funcall(env, func, nargs, args);
}
