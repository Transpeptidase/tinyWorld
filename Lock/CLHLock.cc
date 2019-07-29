#include "CLHLock.h"

thread_local CLHLock::QNode* CLHLock::myPred = nullptr;
thread_local CLHLock::QNode* CLHLock::myNode = new bool(false);
