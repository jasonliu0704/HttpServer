# cse124-project1

Basic skeleton code for CSE 124's Project 1

Starter code copyright (c) 2017, 2018 George Porter.  All Rights Reserved.

## To build

make


Clarifications on Project 1
1) Assume that 'GET' and 'HTTP/1.1' are case-sensitive. Any other case like 'get' should be considered 400 Client Error

2) Every URL after GET needs to start with '/' else it would be a 400 error

3) Any optional headers or header values (say "Connection: open") that are not supported by the protocol, can just be ignored.

4) The header need to be in a strict format: "<Header_Key>:<Header_Value>" eg. "Host<space>:localhost" is a different header than "Host:localhost", as the key in first case is "Host<space>" and not "Host".

Update: Here the <Header_Value> can have leading or trailing spaces, i.e. "<Header_Key>:<space(s)><Header_Value><space(s)>" is valid.

5) For simplicity, the tests will have the exact same keys as provided in the Project description. So, yes, it is case-sensitive. For example "Host" may not be matched to "host" for which the server can return Error.
