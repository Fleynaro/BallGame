#include "RPCs.h"
#include "Game.h"



void RPC::InitGame(SLNet::BitStream* bitStream, SLNet::Packet* packet)
{
	printf("RPC - InitGame");
	bitStream->Read(NetGame::m_instance->m_playerId);
	NetGame::m_instance->m_state = NetGame::GameState::Connected;
}


#include "RemotePool.h"
void RPC::StreamIn(SLNet::BitStream* bitStream, SLNet::Packet* packet)
{
	int id;
	NetworkType type;
	readId(*bitStream, type, id);

	if (type == NetworkType::Player) {
		if (g_game->m_netGame->m_playerPool->isSlotFree(id))
		{
			RemotePlayer* player = RemotePlayer::New();
			g_game->m_netGame->m_playerPool->Add(id, player);

			printf("Player %i stream in", id);
		}
	}
}

void RPC::StreamOut(SLNet::BitStream* bitStream, SLNet::Packet* packet)
{
	int id;
	NetworkType type;
	readId(*bitStream, type, id);

	if (type == NetworkType::Player) {
		if (!g_game->m_netGame->m_playerPool->isSlotFree(id))
		{
			RemotePlayer* player = g_game->m_netGame->m_playerPool->GetAt(id);
			RemotePlayer::Destroy(player);
			g_game->m_netGame->m_playerPool->Remove(id);

			printf("Player %i stream out", id);
		}
	}
}
