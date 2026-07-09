#include "cvi_tracer.h"

#include "tracer.h"

void CVI_SYS_TraceBegin(const char *name) { Tracer::TraceBegin(name); }

void CVI_SYS_TraceCounter(const char *name, signed int value) { Tracer::TraceCounter(name, value); }

void CVI_SYS_TraceEnd() { Tracer::TraceEnd(); }