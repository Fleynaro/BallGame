#pragma once

#include "MessageIdentifiers.h"

#include "slikenet/peerinterface.h"
#include "slikenet/statistics.h"
#include "slikenet/types.h"
#include "slikenet/BitStream.h"
#include "slikenet/PacketLogger.h"
#include "slikenet/RPC4Plugin.h"
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#include "slikenet/types.h"
#ifdef _WIN32
#include "slikenet/Kbhit.h"
#include "slikenet/WindowsIncludes.h" // Sleep
#else
#include "slikenet/Kbhit.h"
#include <unistd.h> // usleep
#endif
#include "slikenet/Gets.h"
#include "slikenet/linux_adapter.h"
#include "slikenet/osx_adapter.h"

#if LIBCAT_SECURITY==1
#include "slikenet/SecureHandshake.h" // Include header for secure handshake
#endif