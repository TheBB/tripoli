#include <stdio.h>
#include <string.h>

#include "interface.h"


void set_environment(emacs_env *env)
{
    __env = env;
}


void em_provide(char *feature_name)
{
    emacs_value provide_fcn = em_intern("provide");
    emacs_value args[] = {__env->intern(__env, feature_name)};
    __env->funcall(__env, provide_fcn, 1, args);
}


void em_message(char *msg)
{
    emacs_value message_fcn = em_intern("message");
    emacs_value args[] = {__env->make_string(__env, msg, strlen(msg))};
    __env->funcall(__env, message_fcn, 1, args);
}


emacs_value em_intern(char *name)
{
    return __env->intern(__env, name);
}
