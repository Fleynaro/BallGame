#include "RPCs.h"
#include "NetGame.h"



void RPC::ClientJoin(SLNet::BitStream* bitStream, SLNet::Packet* packet)
{
	SLNet::RakString data;
	int offset = bitStream->GetReadOffset();
	bool read = bitStream->ReadCompressed(data);
	RakAssert(read);

	auto addr = packet->systemAddress;


	//init

	Player* player = NetGame::m_instance->m_playerPool->getPlayerWithAddress(addr);
	if (player != nullptr) {
		//
		printf("reject connection: player is at pool");
		return;
	}

	int newId = NetGame::m_instance->m_playerPool->getFreeSlot();
	if (newId == -1) {
		//
		printf("reject connection: no free slot");
		return;
	}

	player = new Player(packet->guid, addr);
	player->m_id = newId;
	NetGame::m_instance->m_playerPool->Add(newId, player);


	printf("ClientJoin: name - %s, ip - %s\n", data.C_String(), addr.ToString(true));
	

	SLNet::BitStream bsSend;
	bsSend.Write(newId);
	NetGame::m_instance->m_rpc.Call("InitGame", &bsSend, HIGH_PRIORITY, RELIABLE_ORDERED, 0, player->m_addr, false);
}

void RPC::Chat(SLNet::BitStream* bitStream, SLNet::Packet* packet)
{

}
