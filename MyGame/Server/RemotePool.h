#pragma once

#include "World.h"
#include "TDs.h"


namespace TL
{
	class Ball
	{
	public:
		Ball(IBall* ball = nullptr)
			: m_ball(ball)
		{}

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
		}

		void process(TimeStamp cur_ts, TimeStamp prev_ts)
		{

		}

		TL::State<TD::GeneralState> m_tl_generalState;
		TL::Event<TD::Shoot> m_tl_shoot;

		IBall* m_ball;
	};
};



class RemoteEntity
{
public:
	virtual NetworkType getNetworkType() = 0;
	int getId() {
		return m_id;
	}

	int m_id = -1;
};

class RemoteEntityPool
{
public:
};



class RemotePlayer : public RemoteEntity
{
public:
	RemotePlayer()
	{}

	NetworkType getNetworkType() {
		return NetworkType::Player;
	}

	IBall* m_entity = nullptr;
	TL::Ball m_tl_ball;

	SLNet::RakNetGUID m_guid;
	SLNet::SystemAddress m_addr;

	float m_simulation_speed = 1.0f;

	TimeStamp m_prev_ts;
	void process(TimeStamp cur_ts)
	{
		if (m_prev_ts == cur_ts)
			return;
		m_tl_ball.process(m_prev_ts, cur_ts);
		m_prev_ts = cur_ts;
	}

	static RemotePlayer* New()
	{
		RemotePlayer* remotePlayer = new RemotePlayer;
		remotePlayer->m_entity = new Ball();
		remotePlayer->m_tl_ball.m_ball = remotePlayer->m_entity;
		return remotePlayer;
	}

	static void Destroy(RemotePlayer* player) {
		delete player->m_entity;
		delete player;
	}
};

class RemotePlayerPool : public RemoteEntityPool
{
public:
	static const inline int maxPlayers = 10;

	RemotePlayerPool() {
		m_players = std::vector<RemotePlayer*>(maxPlayers, nullptr);
	}

	void process();

	void Add(int id, RemotePlayer* player)
	{
		m_players[id] = player;
	}

	RemotePlayer* GetAt(int id) {
		return m_players[id];
	}

	void Remove(int id)
	{
		m_players[id] = nullptr;
	}

	bool isSlotFree(int id) {
		return m_players[id] == nullptr;
	}

	std::vector<RemotePlayer*> m_players;
};

