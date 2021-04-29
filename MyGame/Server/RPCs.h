#pragma once


#include "SlikeNet.h"

class RPC
{
public:
	static void InitGame(SLNet::BitStream* bitStream, SLNet::Packet* packet);
	static void StreamIn(SLNet::BitStream* bitStream, SLNet::Packet* packet);
	static void StreamOut(SLNet::BitStream* bitStream, SLNet::Packet* packet);
};