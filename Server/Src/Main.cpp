#include "Nikson.h"
#include "Tokens.h"
#include "Server.h"
#include <vector>
#include <sstream>
int main() 
{
	using namespace std;
	using namespace Nikson;
	
	Nikson::Winsock ws;
	Nikson::Server server(13);
	server.run();
}