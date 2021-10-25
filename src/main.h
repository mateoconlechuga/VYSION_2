#ifndef MAIN_H
#define MAIN_H

#include "optix/optix.h"

#define MIN_OPTIX_VERSION 2

#ifdef OPTIX_VERSION
    #if OPTIX_VERSION < MIN_OPTIX_VERSION
        #error OPTIX version too low to build. Aborting.
    #endif
#else
    #error Variable OPTIX_VERSION not found. Aborting.
#endif

int main(void);

#endif