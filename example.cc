#include "socketstream.hh"
#include <iostream>
#include <string>

int main()
{
	swoope::socketstream client;
	char* buf = new char[1024];

	client.open("www.google.com", "80");
	if (!client.is_open()) return 1;
	client.unsetf(std::ios_base::unitbuf);
	client <<
		"HEAD / HTTP/1.1\r\n" <<
		"Host: www.google.com\r\n\r\n";
	client.flush();
	client.shutdown(swoope::socketstream::out);
	while (client.read(buf, 1024), client.gcount()) {
		client.clear();
		std::cout.write(buf, client.gcount());
	}
	delete[] buf;
	return 0;
}
