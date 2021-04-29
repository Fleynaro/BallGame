#include "Timeline.h"

void readId(SLNet::BitStream& data, NetworkType& type, int& id)
{
	bool isPlayer = data.ReadBit();
	if (isPlayer) {
		type = NetworkType::Player;
		data.ReadBits((unsigned char*)& id, 3);
	}
	else {
		type = NetworkType::SharedObject;
		data.ReadBits((unsigned char*)& id, 3);
	}
}

void writeId(SLNet::BitStream& data, NetworkType type, int id)
{
	switch (type)
	{
	case NetworkType::Player:
		data.Write1();
		data.WriteBits((unsigned char*)& id, 3);
		break;
	case NetworkType::SharedObject:
		data.Write0();
		data.WriteBits((unsigned char*)& id, 3);
		break;
	}
}

TimeStamp getTimestamp()
{
	return GetTickCount64() / 60 * 2;
}
