#include <cmath>
#include <iostream>
#include <sstream>
#include <boost/process.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <crypto++/base64.h>
#include "Storage.hpp"

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::process;
using namespace boost::property_tree;
using namespace CryptoPP;

unsigned int Storage::maxPayloadSize = 10000;

Storage::Storage(string addr, unsigned short p, string waddr, unsigned short wp, string contract_account)
:node_addr(addr),node_port(p),wallet_addr(waddr),wallet_port(wp),contract(contract_account)
{
}

Storage::~Storage()
{
}

bool Storage::get(string txid,string filename)
{
	bool flag;
	string retval, encoded, decoded;
	boost::tie(flag,encoded) = fetchTx(txid);
	if(false == flag) return false;
	//decode base64 into bytearray
	Base64Decoder decoder;
	decoder.Put((byte*)encoded.data(),encoded.size());
	decoder.MessageEnd();
	word64 size = decoder.MaxRetrievable();
	if(size) {
		decoded.resize(size);
		decoder.Get((byte*)decoded.data(),decoded.size());
	}
	//write into file
	std::ofstream out(filename,ios::binary);
	if(false == out.is_open()) return false;
	out.write(decoded.data(),decoded.size());
	return true;
}

boost::tuple<bool,string> Storage::push(string filename)
{
	vector<string> chunks = prepareChunks(filename);
	if(0 == chunks.size()) return boost::make_tuple(false,"");
	regex trx_id("\"trx_id\":\\s*\"(.*)\"");
	match_results<string::const_iterator> what;
#ifndef NDEBUG
	int count = 0;
#endif
	string nxt_txid = "";
	for(auto itr = chunks.rbegin() ; itr != chunks.rend() ; itr++) {
		bool succeeded;
		//loop push action until all chunks are successfully sent
		string line;
		do {
			vector<string> arguments;
			arguments.push_back("--print-response");
			arguments.push_back("--wallet-url");
			arguments.push_back("http://" + wallet_addr + ":" + lexical_cast<string>(wallet_port));
			arguments.push_back("-u");
			arguments.push_back("http://" + node_addr + ":" + lexical_cast<string>(node_port));
			arguments.push_back("push");
			arguments.push_back("action");
			arguments.push_back(contract);
			arguments.push_back("upload");
			arguments.push_back("{\"chunk\":\"" + *itr + "\",\"nxt_txid\":\"" + nxt_txid + "\"}");
			arguments.push_back("-p");
			//FIXME: currently we send data with identity of contract account
			arguments.push_back(contract);
			ipstream out;
			child c(search_path("cleos"),arguments,std_err > out,std_out > null);
			string txid = "";
			while(c.running() && getline(out,line))
				if(regex_search(line,what,trx_id)) {
					txid = string(what[1].begin(),what[1].end());
					break;
				}
			c.wait();
			//check whether the transaction id is acquired
			succeeded = (EXIT_SUCCESS == c.exit_code() && "" != txid);
#ifndef NDEBUG
			cout<<"("<<((succeeded)?++count:count)<<"/"<<chunks.size()<<") "<<txid<<" "<<((succeeded)?"succeed":"fail")<<endl;
#endif
			nxt_txid = (succeeded)?txid:nxt_txid;
		} while(false == succeeded);
	}
	return boost::make_tuple(true,nxt_txid);
}

boost::tuple<int,float,float> Storage::cost(string filename)
{
	//estimate cost of num of tx, net and cpu
	vector<string> chunks = prepareChunks(filename);
	int num = chunks.size();
	float net = static_cast<float>(1272 * num) / 1000;
	float cpu = static_cast<float>(20000 * num) / 1000;
	return boost::make_tuple(num,net,cpu);
}

boost::tuple<bool,string> Storage::fetchTx(string txid)
{
	//get transaction content
	vector<string> arguments;
	arguments.push_back("--print-response");
	arguments.push_back("-u");
	arguments.push_back("http://" + node_addr + ":" + lexical_cast<string>(node_port));
	arguments.push_back("get");
	arguments.push_back("transaction");
	arguments.push_back(txid);
	ipstream out;
	child c(search_path("cleos"),arguments,std_out > out,std_err > null);
	string json, line;
	while(c.running() && getline(out,line)) json += line;
	c.wait();
	if(EXIT_SUCCESS != c.exit_code()) {
		return boost::make_tuple(false,"");
	}
#ifndef NDEBUG
	cout<<"fetching "<<txid<<endl;
#endif
	//parse the json
	ptree pt;
	stringstream ss(json);
	read_json(ss,pt);
	ptree & data = pt.get_child("trx").get_child("trx").get_child("actions").begin()->second.get_child("data");
	string chunk = data.get_child("chunk").get_value<string>();
	string nxt_txid = data.get_child("nxt_txid").get_value<string>();
	if("" != nxt_txid) {
		//check whether the data stream continues
		bool flag;
		string retval;
		boost::tie(flag,retval) = fetchTx(nxt_txid);
		if(false == flag) {
			cout<<"interrupted by error"<<endl;
			return boost::make_tuple(false,"");
		}
		chunk += retval;
	}
	return boost::make_tuple(true,chunk);
}

vector<string> Storage::prepareChunks(string filename)
{
	//read binary file into string
	std::ifstream in(filename,ios::binary);
	if(false == in.is_open()) {
		return vector<string>();
	}
	stringstream buffer;
	buffer << in.rdbuf();
	string decoded = buffer.str();
	//encode the string
	string encoded;
	Base64Encoder encoder;
	encoder.Put((byte*)decoded.data(),decoded.size());
	encoder.MessageEnd();
	word64 size = encoder.MaxRetrievable();
	if(size) {
		encoded.resize(size);
		encoder.Get((byte*)encoded.data(), encoded.size());
	}
	//remove all end of line character
	encoded.erase(remove(encoded.begin(),encoded.end(),'\n'),encoded.end());
	//split encoded into multiple chunks
	vector<string> chunks;
	int nchunks = ceil(static_cast<float>(encoded.size()) / maxPayloadSize);
	for(int i = 0 ; i < nchunks ; i++)
		chunks.push_back(encoded.substr(i * maxPayloadSize,maxPayloadSize));
#ifndef NDEBUG
	cout<<"chunk number: "<<chunks.size()<<endl;
#endif
	return chunks;
}

