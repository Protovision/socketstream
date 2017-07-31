#include "socketstream.hh"
#include <iostream>
#include <string>

int main()
{
	swoope::socketstream client;
	std::string line;

	client.open("www.google.com", "80");
	if (!client) return 1;
	client.unsetf(std::ios_base::unitbuf);
	client <<
		"HEAD / HTTP/1.1\r\n" <<
		"Host: www.google.com\r\n\r\n";
	client.flush();
	client.shutdown(swoope::socketstream::out);
	while (std::getline(client, line)) {
		if (line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		std::cout << line << std::endl;
	}
	return 0;
}
