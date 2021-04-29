#include "RemotePool.h"
#include "Server/NetGame.h"




void RemotePlayerPool::process()
{
	static TimeStamp prev_ts = 0;
	TimeStamp cur_ts = GetTickCount64();

	for (RemotePlayer* player : m_players)
	{
		if (player == nullptr) continue;


		if (prev_ts)
		{
			TimeStamp new_ts = player->m_prev_ts;

			new_ts += float(cur_ts - prev_ts) * player->m_simulation_speed;

			player->process(new_ts);
		}
	}

	prev_ts = cur_ts;
}
