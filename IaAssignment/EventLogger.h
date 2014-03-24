#pragma once

#include <iostream>

struct EventLogger
{
	bool train;
	bool leaving;
	bool entering;
	bool ontrack;

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

		stream << std::endl;
	}
};