#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <string>
#include <boost/tuple/tuple.hpp>

using namespace std;
using namespace boost;

class Storage {
	static unsigned int maxPayloadSize;
	
	string node_addr;
	unsigned short node_port;
	string wallet_addr;
	unsigned short wallet_port;
	string contract;
public:
	Storage(string node_addr = "localhost", unsigned short node_port = 8000, string wallet_addr = "localhost", unsigned short wallet_port = 6666, string contract_account = "eosfilestore");
	virtual ~Storage();
	bool get(string txid,string filename);
	boost::tuple<bool,string> push(string filename);
	boost::tuple<int,float,float> cost(string filename);
protected:
	boost::tuple<bool,string> fetchTx(string txid);
	vector<string> prepareChunks(string filename);
};

#endif
