#include "socketstream.hh"
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
	swoope::socketstream server, client;
	string line;

	if (argc != 2) return 1;
	server.open(argv[1], 4);
	server.accept(client);
	cout << "Connection from " << client.remote_address() << endl;
	if (client.is_open()) {
		while (getline(client, line)) {
			cout << line << endl;
			client << line << endl;
		}
		client.shutdown(ios_base::out);
		client.close();
	}
	server.close();
	return 0;
}

