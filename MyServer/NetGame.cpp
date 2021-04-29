#include "NetGame.h"


unsigned char GetPacketIdentifier(SLNet::Packet* p)
{
	if (p == 0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(p->length > sizeof(SLNet::MessageID) + sizeof(SLNet::Time));
		return (unsigned char)p->data[sizeof(SLNet::MessageID) + sizeof(SLNet::Time)];
	}
	else
		return (unsigned char)p->data[0];
}


#include "RPCs.h"

void NetGame::registerRPCs()
{
	m_rpc.RegisterFunction("ClientJoin", &RPC::ClientJoin);
	m_rpc.RegisterFunction("Chat", &RPC::Chat);
}



#include "TDs.h"

void NetGame::packet_playerState(SLNet::Packet* p)
{
	printf("packet_playerState from %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());

	Player* player = NetGame::m_instance->m_playerPool->getPlayerWithAddress(p->systemAddress);
	if (player == nullptr)
		return;

	SLNet::BitStream bs(p->data, p->length, false);

	//bs.IgnoreBits(8);

	TD::Id id;
	bs.Read(id);

	TD::ITimeData* td = TD::getTimeDataInstanceById(id);
	if (td == nullptr)
		return;

	td->deserialize(bs);
	player->m_tl_ball.add(id, 0, td);
}
