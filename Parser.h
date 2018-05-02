#ifndef CALCPARSER_HPP
#define CALCPARSER_HPP

#include <string>
#include <stdint.h>
#include <unordered_map>
#include <assert.h>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>
#include "Framer.h"
using namespace std;

// http data structures for request and response
typedef struct Request {
	bool valid = true;
	bool hasHost = false;
	bool close = false;
	string URL;
} HttpRequest;

typedef struct Response{
	string response_msg;
	bool close = false;
	char* file_path;
	bool send_file = false;
	off_t file_size = 0;
} HttpResponse;



class Parser {
public:
	string getFileType (const string file_name);
	HttpRequest parseRequest(Framer* framer);
	HttpResponse parseResponse(HttpRequest& req, char* doc_root);
	vector<string> split(const string &s, char delim);
	bool isValidURL(const string url);
	string CRLF = "\r\n";
	string server = "SimpleHttpServer";
	string version = "HTTP/1.1";
	unordered_map<string, string> file_type_map = {
	  {"jpg", "image/jpeg"},
	  {"png", "image/png"},
	  {"html", "text/html"}
	};
	// http status code map
	unordered_map<string, string> status_map {
		{"200", "OK"},
		{"400", "Client Error"},
		{"403", "Forbidden"},
		{"404", "Not Found"}
	};

};

#endif // CALCPARSER_HPP
