#pragma once

#include "main.h"
#include "Timeline.h"
#include "PlayerPool.h"

unsigned char GetPacketIdentifier(SLNet::Packet* p);


class NetGame
{
public:
	static inline NetGame* m_instance = nullptr;

	PlayerPool* m_playerPool;

	NetGame() {
		m_playerPool = new PlayerPool;
	}

	void init()
	{
		m_rakServer = SLNet::RakPeerInterface::GetInstance();
		m_rakServer->SetIncomingPassword(m_password.c_str(), m_password.length());
		m_rakServer->SetTimeoutTime(30000, SLNet::UNASSIGNED_SYSTEM_ADDRESS);
		m_rakServer->AllowConnectionResponseIPMigration(false);


		SLNet::SocketDescriptor socketDescriptors[2];
		socketDescriptors[0].port = m_port;
		socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4
		socketDescriptors[1].port = m_port;
		socketDescriptors[1].socketFamily = AF_INET6; // Test out IPV6
		bool b = m_rakServer->Startup(4, socketDescriptors, 2) == SLNet::RAKNET_STARTED; //start server (not connect functions because this is a server)
		m_rakServer->SetMaximumIncomingConnections(10);
		
		if (!b)
		{
			printf("Failed to start dual IPV4 and IPV6 ports. Trying IPV4 only.\n");

			// Try again, but leave out IPV6
			b = m_rakServer->Startup(4, socketDescriptors, 1) == SLNet::RAKNET_STARTED;
			if (!b)
			{
				puts("Server failed to start.  Terminating.");
				exit(1);
			}
		}


		m_rakServer->SetOccasionalPing(true);
		m_rakServer->SetUnreliableTimeout(1000);

		m_rakServer->AttachPlugin(&m_rpc);

		registerRPCs();
	}
	
	SLNet::RakPeerInterface* m_rakServer;
	std::string m_password;
	SLNet::RPC4 m_rpc;
	unsigned short m_port = 50000;


	void send(Player* player, SLNet::BitStream& bs, PacketPriority priorioty, PacketReliability realibility, int channel = 0)
	{
		m_rakServer->Send(&bs, priorioty, realibility, channel, player->m_addr, false);
	}

	void registerRPCs();

	void process()
	{
		updateNetwork();

		m_playerPool->Process();
	}


	void updateNetwork()
	{
		SLNet::Packet* p;

		for (p = m_rakServer->Receive(); p; m_rakServer->DeallocatePacket(p), p = m_rakServer->Receive())
		{
			// We got a packet, get the identifier with our handy function
			unsigned char packetIdentifier = GetPacketIdentifier(p);

			// Check if this is a network message packet
			switch (packetIdentifier)
			{
			case ID_DISCONNECTION_NOTIFICATION:
				// Connection lost normally
				printf("ID_DISCONNECTION_NOTIFICATION from %s\n", p->systemAddress.ToString(true));;
				break;
			case ID_NEW_INCOMING_CONNECTION:
				// Somebody connected.  We have their IP now
				printf("ID_NEW_INCOMING_CONNECTION from %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());
				break;
			case ID_RPC_REMOTE_ERROR:
				printf("rpc packet error");
				break;
			case ID_RPC_PLUGIN:
				printf("rpc packet came");
				break;

			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
				break;

			case ID_CONNECTED_PING:
			case ID_UNCONNECTED_PING:
				printf("Ping from %s\n", p->systemAddress.ToString(true));
				break;

			case ID_CONNECTION_LOST:
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally
				// terminated
				printf("ID_CONNECTION_LOST from %s\n", p->systemAddress.ToString(true));;
				break;


			case ID_PLAYER_STATE:
				packet_playerState(p);
				break;

			case ID_PLAYER_INTERACT:
				packet_playerInteract(p);
				break;

			default:
				// It's a client, so just show the message
				printf("%s\n", p->data);
				break;
			}
		}
	}

	void packet_playerState(SLNet::Packet* p);

	void packet_playerInteract(SLNet::Packet* p)
	{
		printf("packet_playerInteract from %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());

	}
};