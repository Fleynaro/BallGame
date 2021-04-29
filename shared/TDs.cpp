
#include "TDs.h"

TD::ITimeData* TD::getTimeDataInstanceById(Id id)
{
	switch (id)
	{
	case TD::Id::GeneralState:
		return new TD::GeneralState;
	case TD::Id::Shoot:
		return new TD::Shoot;
	}

	return nullptr;
}
