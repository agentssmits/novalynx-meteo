#ifndef HANDLERS_H
#define HANDLERS_H

#include "defines.h"

void *cientHandler();
void intHandler(int dummy);
void pipeHandler(int dummy);

extern int keepRunning;
extern short int clientCount;
extern char wxBuff[wxBuffSize];

extern int newData;
#endif
