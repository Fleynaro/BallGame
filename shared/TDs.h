#pragma once

#include "Timeline.h"
#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btQuaternion.h>


namespace TD
{
	enum class Id
	{
		GeneralState,
		Shoot
	};

	TD::ITimeData* getTimeDataInstanceById(Id id);

	class GeneralState : public TD::State
	{
	public:
		struct
		{
			btVector3 position;
			btQuaternion rotation;
			btVector3 velocity;
			btVector3 angularVelocity;
		} m_data;

		TD::State* interpolate(TimeLineData::iterator currentFrame, TimeLineData::iterator nextFrame, TimeStamp timestamp) override
		{
			if (!getTimeline().has(nextFrame))
				return nullptr;

			float linearDelta = float(timestamp - currentFrame->first) / float(nextFrame->first - currentFrame->first); //from 0.0 to 1.0
			auto curState = (GeneralState*)currentFrame->second;
			auto nextState = (GeneralState*)nextFrame->second;

			GeneralState* ipState = new GeneralState;
			ipState->m_data.position = curState->m_data.position + (nextState->m_data.position - curState->m_data.position) * linearDelta;
			ipState->m_data.rotation = curState->m_data.rotation.slerp(nextState->m_data.rotation, linearDelta);
			ipState->m_data.velocity = curState->m_data.velocity + (nextState->m_data.velocity - curState->m_data.velocity) * linearDelta;
			ipState->m_data.angularVelocity = curState->m_data.angularVelocity + (nextState->m_data.angularVelocity - curState->m_data.angularVelocity) * linearDelta;
			return ipState;
		}

		void serialize(SLNet::BitStream& data) override
		{
			data.Write((BYTE)Id::GeneralState);
			data.Write((PCHAR)& m_data, sizeof(m_data));
		}

		void deserialize(SLNet::BitStream& data) override
		{
			data.Read((PCHAR)& m_data, sizeof(m_data));
		}
	};

	class Shoot : public TD::Event
	{
	public:
		struct
		{
			btVector3 shootDir;
			float size;
			float mass;
			float speed;
		} m_data;

		int getDuration() override {
			return 1;
		}

		void serialize(SLNet::BitStream& data) override
		{
			data.Write((BYTE)Id::Shoot);
			data.Write((PCHAR)& m_data, sizeof(m_data));
		}

		void deserialize(SLNet::BitStream& data) override
		{
			data.Read((PCHAR)& m_data, sizeof(m_data));
		}
	};
};