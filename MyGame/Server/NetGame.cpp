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
#include "Game.h"

void NetGame::registerRPCs()
{
	m_rpc.RegisterFunction("InitGame", &RPC::InitGame);
	m_rpc.RegisterFunction("StreamIn", &RPC::StreamIn);
	m_rpc.RegisterFunction("StreamOut", &RPC::StreamOut);
}

void NetGame::packet_playerState(SLNet::Packet* p)
{
	SLNet::BitStream bs(p->data, p->length, false);

	int id;
	NetworkType type;
	readId(bs, type, id);

	if (type == NetworkType::Player) {
		if (!g_game->m_netGame->m_playerPool->isSlotFree(id))
		{
			RemotePlayer* player = g_game->m_netGame->m_playerPool->GetAt(id);

			TD::Id td_id;
			bs.Read(td_id);

			TD::ITimeData* td = TD::getTimeDataInstanceById(td_id);
			if (td == nullptr)
				return;

			td->deserialize(bs);
			player->m_tl_ball.add(td_id, 0, td);
		}
	}
}