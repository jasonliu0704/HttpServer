#ifndef CALCFRAMER_HPP
#define CALCFRAMER_HPP

#include <vector>
#include <string>
#include <queue>
#include <unordered_map>
using namespace std;

class Framer {
public:
// Here one message is one instruction
	// append new incoming buffer tp request buffer
	void append(std::string chars);

	// Does the current reqeust buffer contain at least one complete message?
	// complete message is determined by EOF
	bool hasRequest() const;

	// Returns the first instruction of the current message
	string topMessage() const;

	// Removes the first instruction of the current message
	void popMessage();

	// prints the string to an ostream (useful for debugging)
	//void printToStream(std::ostream& stream) const;

	// string split
	vector<string> split(const string &s, char delim);

protected:
	static const char LF = '\n';
	static const char CR = '\r';
	string cur_msg = "";
	// The dynamic buffer
	queue<string> request_buffer;
	int req_count = 0;

};

#endif // CALCFRAMER_HPP
