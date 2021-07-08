#include "control.h"

struct vysion_context *vysion_current_context;

//functions
void vysion_SetContext(struct vysion_context *context) {
    if (context) vysion_current_context = context;
}