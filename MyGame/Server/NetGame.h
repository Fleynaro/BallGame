#pragma once

#include "Main.h"
#include "Timeline.h"
#include "RemotePool.h"


unsigned char GetPacketIdentifier(SLNet::Packet* p);

class NetGame
{
public:
	static inline NetGame* m_instance = nullptr;

	enum class GameState
	{
		Not,
		Connecting,
		Connected
	};
	GameState m_state = GameState::Not;

	RemotePlayerPool* m_playerPool;

	NetGame() {
		m_playerPool = new RemotePlayerPool;
	}

	void init()
	{
		m_rakClient = SLNet::RakPeerInterface::GetInstance();

		SLNet::SocketDescriptor socketDescriptor(0, 0);
		socketDescriptor.socketFamily = AF_INET;
		m_rakClient->Startup(8, &socketDescriptor, 1);
		m_rakClient->SetOccasionalPing(true);
		m_rakClient->AttachPlugin(&m_rpc);

		registerRPCs();
	}

	void connectToServer(std::string ip, std::string port)
	{
		SLNet::ConnectionAttemptResult car = m_rakClient->Connect(ip.c_str(), stoi(port), m_password.c_str(), m_password.length());
		RakAssert(car == SLNet::CONNECTION_ATTEMPT_STARTED);
		m_state = GameState::Connecting;
		printf("try to connect to %s:%s...\n", ip.c_str(), port.c_str());
	}

	
	SLNet::RakPeerInterface* m_rakClient;
	std::string m_password;
	SLNet::RPC4 m_rpc;

	int m_playerId = -1;

	void send(DefaultMessageIDTypes type, TD::ITimeData* td, PacketPriority priorioty, PacketReliability realibility, int channel = 0)
	{
		if (m_state != GameState::Connected)
			return;

		SLNet::BitStream bs;
		bs.Write((BYTE)type);
		td->serialize(bs);
		m_rakClient->Send(&bs, priorioty, realibility, channel, SLNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}

	void registerRPCs();

	void proccess()
	{
		m_playerPool->process();
	}


	void updateNetwork()
	{
		proccess();


		SLNet::Packet* p;
		
		for (p = m_rakClient->Receive(); p; m_rakClient->DeallocatePacket(p), p = m_rakClient->Receive())
		{
			// We got a packet, get the identifier with our handy function
			unsigned char packetIdentifier = GetPacketIdentifier(p);

			// Check if this is a network message packet
			switch (packetIdentifier)
			{
			case ID_DISCONNECTION_NOTIFICATION:
				// Connection lost normally
				printf("ID_DISCONNECTION_NOTIFICATION\n");
				break;
			case ID_ALREADY_CONNECTED:
				// Connection lost normally
				printf("ID_ALREADY_CONNECTED with guid %" PRINTF_64_BIT_MODIFIER "u\n", p->guid.g);
				break;
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
				break;
			case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n");
				break;
			case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				printf("ID_REMOTE_CONNECTION_LOST\n");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
				printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
				break;
			case ID_CONNECTION_BANNED: // Banned from this server
				printf("We are banned from this server.\n");
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				printf("Connection attempt failed\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				// Sorry, the server is full.  I don't do anything here but
				// A real app should tell the user
				printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
				break;

			case ID_INVALID_PASSWORD:
				printf("ID_INVALID_PASSWORD\n");
				break;

			case ID_CONNECTION_LOST:
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally
				// terminated
				printf("ID_CONNECTION_LOST\n");
				break;

			case ID_CONNECTION_REQUEST_ACCEPTED:
				packet_connectionSucceeded(p);
				break;
			case ID_CONNECTED_PING:
			case ID_UNCONNECTED_PING:
				printf("Ping from %s\n", p->systemAddress.ToString(true));
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

	void packet_connectionSucceeded(SLNet::Packet* p)
	{
		printf("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());
		printf("My external address is %s\n", m_rakClient->GetExternalID(p->systemAddress).ToString(true));

		SLNet::BitStream bsSend;
		bsSend.WriteCompressed("NickName");
		m_rpc.Call("ClientJoin", &bsSend, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_rakClient->GetSystemAddressFromIndex(0), false);
	}
};