#include "PlayerPool.h"
#include "NetGame.h"

void Player::playerInStreamArea(std::function<void(Player*)> lambda)
{
	for (Player* player2 : NetGame::m_instance->m_playerPool->m_players)
	{
		if (player2 == nullptr || m_id == player2->m_id) continue;
		if (isPlayerInStreamArea(player2)) {
			lambda(player2);
		}
	}
}

void PlayerPool::Process()
{
	for (Player* player : m_players)
	{
		if (player == nullptr) continue;

		TL::Ball& tl = player->m_tl_ball;

		TimeFrame frame;
		frame = tl.m_tl_generalState.newFrame();
		if (tl.m_tl_generalState.has(frame)) {
			auto gState = (TD::GeneralState*)frame->second;
			player->m_position = gState->m_data.position;

			SLNet::BitStream bsSend;
			writeId(bsSend, player->getNetworkType(), player->getId());
			gState->serialize(bsSend);

			player->playerInStreamArea([&](Player* player2) {
				SLNet::BitStream bsSend;
				bsSend.Write((BYTE)ID_PLAYER_STATE);
				writeId(bsSend, player2->getNetworkType(), player2->getId());
				gState->serialize(bsSend);
				NetGame::m_instance->send(player2, bsSend, HIGH_PRIORITY, RELIABLE_ORDERED);
			});
		}

		
		player->playerInStreamArea([&](Player* player2) {
			if (!player->doesPlayerHaveStreamedEntity(player2))
			{
				player->m_streamEntities.push_back(player2);

				//new player in stream area
				SLNet::BitStream bsSend;
				writeId(bsSend, player2->getNetworkType(), player2->getId());
				NetGame::m_instance->m_rpc.Call("StreamIn", &bsSend, HIGH_PRIORITY, RELIABLE_ORDERED, 0, player->m_addr, false);
			}
		});

		for (Entity* entity : player->m_streamEntities)
		{
			if (entity->getNetworkType() == NetworkType::Player) {
				Player* player2 = (Player*)entity;
				if (!player->isPlayerInStreamArea(player2))
				{
					player->m_streamEntities.remove(player2);

					//remove player from stream objs list
					SLNet::BitStream bsSend;
					writeId(bsSend, player2->getNetworkType(), player2->getId());
					NetGame::m_instance->m_rpc.Call("StreamOut", &bsSend, HIGH_PRIORITY, RELIABLE_ORDERED, 0, player->m_addr, false);
				}
			}
		}
	}
}
