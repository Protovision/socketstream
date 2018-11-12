#include "socketstream.hh"
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
	swoope::socketstream client;
	string line;

	if (argc != 3) return 1;
	client.open(argv[1], argv[2]);
	if (!client.is_open()) return 1;
	cout << "Connected to " << client.remote_address() << endl;
	while (getline(cin, line)) {
		client << line << endl;
		if (!getline(client, line)) break;
		cout << line << endl;
	}
	client.shutdown(ios_base::out);
	client.close();
	return 0;
}
