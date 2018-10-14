#ifndef __PSXLE_C_WRAPPER_H__
#define __PSXLE_C_WRAPPER_H__

#include "psxle_interface.h"

extern "C" {
    PSXLEInterface*PSXLE_new() { return new PSXLEInterface(); }
    //void foo() { }
}

#endif
