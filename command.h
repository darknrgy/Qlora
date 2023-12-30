#ifndef COMMAND_H
#define COMMAND_H

struct Command
{
	const char* name;
	void (*callback)(char*);

	Command(const char* name, void (*callback)(char*)) {
		this->name = name;
		this->callback = callback;
	}
};

#endif
