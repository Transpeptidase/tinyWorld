#include "MCSLock.h"


thread_local MCSLock::QNode MCSLock::myNode;
