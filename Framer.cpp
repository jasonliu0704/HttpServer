#include <assert.h>
#include <iostream>
#include "Framer.h"

using namespace std;

void Framer::append(string chars)
{
	// push imcoming buffer to request buffer
	for(auto& c: chars){
		if(c == LF){
			// mark end of request
			//cout << "cur_msg: " << cur_msg << ", length: " << cur_msg.size()<< endl;
			if(cur_msg.empty()){
				request_buffer.push("EOR");
				req_count++;
			}else{
			// end of each line
				request_buffer.push(cur_msg);
			}
			cur_msg = "";
		}else{
			if(c != CR)cur_msg.push_back(c);
		}
	}
}

bool Framer::hasRequest() const
{
	return (!request_buffer.empty() && req_count);
}

string Framer::topMessage() const
{
	return request_buffer.front();
}

void Framer::popMessage()
{
	if(topMessage().compare("EOR") == 0)req_count--;
	request_buffer.pop();
}

/*
void Framer::printToStream(ostream& stream) const
{
	// PUT YOUR CODE HERE
}*/
