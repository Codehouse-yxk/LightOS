

#include "global.h"

GdtInfo gGdtInfo = {0};

IdtInfo gIdtInfo = {0};

void (*const RunTask)(volatile Task* p) = NULL;

void (*const LoadTask)(volatile Task* p) = NULL;