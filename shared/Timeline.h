#pragma once

#include "SlikeNet.h"


enum class NetworkType
{
	Player,
	SharedObject
};
void readId(SLNet::BitStream& data, NetworkType& type, int& id);
void writeId(SLNet::BitStream& data, NetworkType type, int id);


using TimeStamp = uint64_t;
TimeStamp getTimestamp();


namespace TD {
	class ITimeData;
};
namespace TL {
	class ITimeLine;
};
using TimeLineData = std::map<TimeStamp, ::TD::ITimeData*>;
using TimeFrame = TimeLineData::iterator;


namespace TD
{
	class ITimeData
	{
	public:
		ITimeData() = default;

		void setTimeline(::TL::ITimeLine* tl) {
			m_tl = tl;
		}

		::TL::ITimeLine& getTimeline() {
			return *m_tl;
		}

		virtual void serialize(SLNet::BitStream& data) = 0;
		virtual void deserialize(SLNet::BitStream& data) = 0;
	private:
		::TL::ITimeLine* m_tl = nullptr;
	};

	class Event : public ITimeData
	{
	public:
		Event() {}

		virtual int getDuration() {
			return 3;
		}
	};

	class State : public ITimeData
	{
	public:
		State() {}

		virtual State* interpolate(TimeFrame currentFrame, TimeFrame nextFrame, TimeStamp timestamp) = 0;
	};
};

namespace TL
{
	class ITimeLine
	{
	public:
		ITimeLine() = default;

		void setData(TimeStamp timestamp, TD::ITimeData* data)
		{
			m_data[timestamp] = data;
			data->setTimeline(this);
		}

		bool has(TimeLineData::iterator it) {
			return it != m_data.end() && it->second != nullptr;
		}

		TimeFrame rightOf(TimeStamp timestamp) {
			return m_data.lower_bound(timestamp);
		}

		TimeFrame newFrame() {
			auto frame = rightOf(m_curTimestamp);
			if (has(frame)) {
				m_curTimestamp = frame->first + 1;
			}
			return frame;
		}

		TimeStamp m_curTimestamp = 0;
	protected:
		TimeLineData m_data;
	};
		
	template<typename T = ::TD::Event>
	class Event : public ITimeLine
	{
	public:
		Event() {}

		T* getAt(TimeStamp timestamp)
		{
			auto it = --m_data.lower_bound(timestamp);
			if (it == m_data.end())
				return nullptr;

			if (timestamp > it->first + ((TD::Event*)it->second)->getDuration())
				return nullptr;

			return (T*)it->second;
		}
	};

	template<typename T = ::TD::State>
	class State : public ITimeLine
	{
	public:
		State() {}

		std::unique_ptr<T> getAt(TimeStamp timestamp)
		{
			auto nextFrame = rightOf(timestamp);
			auto currentFrame = nextFrame;
				
			currentFrame--;
			if (currentFrame == m_data.end())
				return NULL;

			return std::unique_ptr<T>(
				(T*)((TD::State*)currentFrame->second)->interpolate(currentFrame, nextFrame, timestamp)
			);
		}
	};

	class Simulation
	{
	public:
		Simulation() = default;

		void setStartFrame(TimeStamp frame) {
			m_currentFrame = frame;
		}

		void nextFrame() {
			m_currentFrame++;
		}

		virtual bool process() = 0;
	protected:
		TimeStamp m_currentFrame = 0;
	};

	class Optimization
	{
	public:
		Optimization() = default;
	};
};