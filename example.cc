#include "socketstream.hh"
#include "net_utility.hh"
#include <iostream>
#include <string>
using namespace std;

int main()
{
	string line;
	socketstream ss;

	sockets_init();
	ss.socket(make_connected_socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
						"www.google.com", "80"));
	if (!ss) return 1;
	ss <<
		"GET / HTTP/1.1" << crlf <<
		"Host: www.google.com" << crlf <<
		"Connection: Close" << crlf <<
		crlf;
	while (getline(ss, line) && line != "\r") {
		line.pop_back();
		cout << line << endl;
	}
	sockets_quit();
	return 0;
}
