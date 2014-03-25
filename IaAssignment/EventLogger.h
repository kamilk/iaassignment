#pragma once

#include <iostream>

struct EventLogger
{
	const char* filename;
	bool train;
	bool leaving;
	bool entering;
	bool ontrack;
	bool barrier;

	EventLogger(const char* filename = nullptr) : filename(filename), train(false), 
		leaving(false), entering(false), ontrack(false), barrier(false) {}

	void WriteForHuman(std::ostream& stream)
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

	void WriteForMachine(std::ostream& stream)
	{
		if (!filename)
			throw std::runtime_error("filename not specified for machine-readable logging.");

		stream << filename << " :";

		if (!train && ontrack)
			WriteEvent(stream, 1);
		if (entering)
			WriteEvent(stream, 2);
		if (!train && leaving)
			WriteEvent(stream, 3);
		if (barrier)
			WriteEvent(stream, 4);
		if (train)
			WriteEvent(stream, 5);

		stream << std::endl;
	}

private:
	void WriteEvent(std::ostream& stream, int eventNumber)
	{
		stream << " Event " << eventNumber;
	}
};