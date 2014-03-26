#pragma once

#include <iostream>

// Receives information about the events occurring in the image and writes
// them to the standard output.
struct EventLogger
{
	// Fields which can be set to indicate an event.
	bool train;
	bool leaving;
	bool entering;
	bool ontrack;
	bool barrier;

	EventLogger() : train(false), leaving(false), entering(false), ontrack(false), barrier(false) {}

	// Outputs the events in a human-readable form
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

	// Outputs the events in a machine-readable form
	void WriteForMachine(std::ostream& stream, const char* filename)
	{
		if (!filename)
			throw std::runtime_error("filename not specified for machine-readable logging.");

		stream << filename << " :";

		// Some sanity checks:
		// a) if a train is on the tracks, nothing else can be
		// b) if a train is passing, nothing can be leaving the crossing because how would it cross it?
		// c) if there is a train, the barrier must be lowered.
		if (!train && ontrack)
			WriteEvent(stream, 1);
		if (entering)
			WriteEvent(stream, 2);
		if (!train && leaving)
			WriteEvent(stream, 3);
		if (barrier || train)
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