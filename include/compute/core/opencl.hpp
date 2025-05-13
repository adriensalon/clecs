#pragma once

#if defined(__APPLE__)
#include <OpenCL/cl.h>
#include <OpenCL/cl_platform.h>
#else
#include <CL/cl.h>
#include <CL/cl_d3d11.h>
#include <CL/cl_platform.h>
#endif
