#pragma once

#include <iostream>

struct EventLogger
{
	bool train;
	bool leaving;
	bool entering;
	bool ontrack;
	bool barrier;

	EventLogger() : train(false), leaving(false), entering(false), ontrack(false), barrier(false) {}

	void Write(std::ostream& stream)
	{
		if (entering)
			stream << "ENTERING!!!" << std::endl;
		if (!train && leaving)
			stream << "LEAVING!!!" << std::endl;
		if (!train && ontrack)
			stream << "ONTRACK!!!" << std::endl;
		if (train)
			stream << "TRAIN!!!" << std::endl;
		if (barrier)
			stream << "BARRIER!!!" << std::endl;

		stream << std::endl;
	}
};