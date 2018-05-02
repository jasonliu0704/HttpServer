#include <string>
#include <stdint.h>
#include <unordered_map>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include "Parser.h"
using namespace std;


vector<string> Parser::split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

// check given URL is valid. ie no more than one .
bool Parser::isValidURL(const string url){
  return url.find("..") == string::npos;
}

// return the file type for the file name
string Parser::getFileType (const string file_name) {
  size_t idx = file_name.rfind('.');
  if( idx != string::npos){
    string file_extension = file_name.substr(idx+1, file_name.size()-idx-1);
    return file_type_map[file_extension];
  }
  return "";
}

// parse response
HttpResponse Parser::parseResponse(HttpRequest& req, char* doc_root){
  HttpResponse res;
  // check connection close
  res.close = req.close;
  string res_msg = "";
  // valid req
  cout << "ParseReponse status: close - " << res.close << " valid - " << req.valid << endl;
  if(req.valid && req.hasHost){
    // find file
    string concat_path = string(doc_root) + req.URL;
    cout << "expected path: " << concat_path << endl;
    //printf("request path0: %s\n", (doc_root + req.URL).c_str());
    const char* rel_path = concat_path.c_str();
    printf("request path1: %s\n", rel_path);
    char* abs_path = NULL;
    abs_path = realpath(rel_path, abs_path);
    printf("abs path: %s\n", abs_path);
    // file permission check
    struct stat fileStat;
    int stat_res = stat(abs_path, &fileStat);
    res.file_path = abs_path;

    // file permission check first then file path
    // since abs_path is NULL when permission denied
    if(stat_res != -1 && !(fileStat.st_mode & S_IROTH)){
    // no permission, 403
      res_msg += (version + " 403 " + status_map["403"] + CRLF);
      res_msg += ("Server:" + server + CRLF);
      res_msg += (CRLF);
      res.response_msg = res_msg;
      return res;
    }

    // bad file path, 404
    if(!isValidURL(req.URL) || abs_path == NULL || stat_res < 0){
      res_msg += (version + " 404 " + status_map["404"] + CRLF);
      res_msg += ("Server:" + server + CRLF);
      res_msg += (CRLF);
      res.response_msg = res_msg;
      return res;
    }


    // all valid, send the file, 200
    res_msg += (version + " 200 " + status_map["200"] + CRLF);
    res_msg += ("Server:" + server + CRLF);
    // set up last Modified Header
    time_t t = fileStat.st_mtime;
    struct tm time;
    localtime_r(&t, &time);
    char timebuf[80];
    cout << "Valid Header: Context-type " << getFileType(res.file_path) <<
    ", Content-Length:" << fileStat.st_size << endl;
    // time format
    // Last-Modified: <day-name>, <day> <month> <year> <hour>:<minute>:<second> GMT
    strftime(timebuf, sizeof(timebuf),
    "Last-Modified: %A, %d %m %Y %H:%M:%S GMT\0", &time);
    res_msg += (string(timebuf) + CRLF);
    res_msg += ("Content-Type:" + getFileType(res.file_path) + CRLF);
    res_msg += ("Content-Length:" + to_string(fileStat.st_size) + CRLF);
    res_msg += (CRLF);
    res.response_msg = res_msg;
    res.file_size = fileStat.st_size;
    res.send_file = true;
    return res;
  }else{
  // invalid request, return 400
    res_msg += (version + " 400 " + status_map["400"] + CRLF);
    res_msg += ("Server:" + server + CRLF);
    res_msg += (CRLF);
    res.response_msg = res_msg;
    return res;
  }
}

HttpRequest Parser::parseRequest(Framer* framer)
{
	HttpRequest req;
  string msg = framer->topMessage();
  framer->popMessage();
  // parseing until EOR
  while(msg.compare("EOR") != 0){
    vector<string> parts = split(msg, ' ');
    // initial line
    if(parts.size() == 3 && parts[0].compare("GET") == 0 && parts[1][0] == '/' &&
    parts[2].compare("HTTP/1.1") == 0){
      cout << "parseRequest: valid message -- " << msg << endl;
      // special index.html mapping
      if(msg.compare("GET / HTTP/1.1") == 0){
        req.URL = "/index.html";
      }else{
        req.URL = parts[1];
      }
      cout << "parserRequest: URL - " << req.URL << endl;
    }else if(parts.size() > 0 && parts[0].find(":") != string::npos){
    // valid header
    cout << "parseRequest: valid header -- " << msg << endl;

      if(parts[0].find("Host:") == 0){
        req.hasHost = true;
      }else if(msg.compare("Connection:close") == 0 || msg.compare("Connection: close") == 0){
        req.close = true;
      }
    }else{
    cout << "parseRequest: invalid -- " << msg << endl;

    // validation error
      req.valid = false;
      return req;
    }
    msg = framer->topMessage();
    framer->popMessage();
  }
	return req;
}
