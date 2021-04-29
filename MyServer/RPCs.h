#pragma once

#include "main.h"

class RPC
{
public:
	static void ClientJoin(SLNet::BitStream* bitStream, SLNet::Packet* packet);
	static void Chat(SLNet::BitStream* bitStream, SLNet::Packet* packet);
};
