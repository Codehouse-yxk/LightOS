
#ifndef GLOBAL_H
#define GLOBAL_H

#include "kernel.h"
#include "const.h"

extern GdtInfo gGdtInfo;

extern IdtInfo gIdtInfo;

extern void (* const RunTask)(Task* pt);


#endif  //GLOBAL_H