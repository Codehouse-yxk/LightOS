
#ifndef GLOBAL_H
#define GLOBAL_H

#include "kernel.h"
#include "const.h"

extern GdtInfo gGdtInfo;

extern IdtInfo gIdtInfo;

extern void (*const RunTask)(volatile Task* p);

extern void (*const LoadTask)(volatile Task* p);


#endif  //GLOBAL_H