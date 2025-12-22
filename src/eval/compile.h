#ifndef _ZENO_SPEC_SRC_EVAL_COMPILE_H
#define _ZENO_SPEC_SRC_EVAL_COMPILE_H

#include "src/eval/bytecode.h"

BytecodeFunction* compile_function(struct FunctionItem const* function);

#endif
