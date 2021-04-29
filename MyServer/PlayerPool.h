#pragma once

#include "main.h"
#include "TDs.h"

class Entity
{
public:
	virtual NetworkType getNetworkType() = 0;
	int getId() {
		return m_id;
	}

	int m_id = -1;
};



namespace TL
{
	class Ball
	{
	public:
		Ball() {}

		void add(TD::Id id, TimeStamp ts, TD::ITimeData* td) {
			switch (id)
			{
			case TD::Id::GeneralState:
				m_tl_generalState.setData(ts, td);
			case TD::Id::Shoot:
				m_tl_shoot.setData(ts, td);
			default:
				return;
			}

			td->getTimeline().m_curTimestamp = ts;
		}

		TL::State<TD::GeneralState> m_tl_generalState;
		TL::Event<TD::Shoot> m_tl_shoot;
	};
};




class Player : public Entity
{
public:
	Player(SLNet::RakNetGUID guid, SLNet::SystemAddress addr)
		: m_guid(guid), m_addr(addr)
	{}

	NetworkType getNetworkType() override {
		return NetworkType::Player;
	}

	SLNet::RakNetGUID m_guid;
	SLNet::SystemAddress m_addr;
	
	TL::Ball m_tl_ball;

	btVector3 m_position;

	std::list<Entity*> m_streamEntities;
	bool doesPlayerHaveStreamedEntity(Entity* entity) {
		for (auto it : m_streamEntities) {
			if (it == entity) {
				return true;
			}
		}
		return false;
	}

	bool isPlayerInStreamArea(Player* player2) {
		return m_position.distance2(player2->m_position) < 100.0 * 100.0;
	}

	void playerInStreamArea(std::function<void(Player*)> lambda);
};

class Pool
{

};


class PlayerPool : public Pool
{
public:
	static const inline int maxPlayers = 10;

	PlayerPool()
	{
		m_players = std::vector<Player*>(maxPlayers, nullptr);
	}

	void Process();

	void Add(int id, Player* player)
	{
		m_players[id] = player;
	}

	void Remove(int id)
	{
		m_players[id] = nullptr;
	}

	bool isSlotFree(int id) {
		return m_players[id] == nullptr;
	}

	Player* getPlayerWithAddress(SLNet::SystemAddress addr) {
		for (auto it : m_players) {
			if (it != nullptr && it->m_addr == addr) {
				return it;
			}
		}
		return nullptr;
	}

	int getFreeSlot() {
		int id = 0;
		for (auto it : m_players) {
			if (isSlotFree(id)) {
				return id;
			}
			id++;
		}
		if (id == maxPlayers)
			return -1;
		return id;
	}

	std::vector<Player*> m_players;
};