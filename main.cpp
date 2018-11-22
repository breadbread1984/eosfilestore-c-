#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <boost/program_options.hpp>
#include "Storage.hpp"

using namespace std;
using namespace boost::program_options;

int main(int argc,char ** argv)
{
	vector<string> txid_filename;
	string filename;
	options_description desc;
	desc.add_options()
		("help,h","print this message")
		("get,g",value<vector<string> >(&txid_filename)->multitoken(),"download file, format \"--get <txid> <filepath>\"")
		("push,p",value<string>(&filename),"upload file, format \"--push <filepath>\"")
		("cost,c",value<string>(&filename),"estimate the cost if we upload the file, format\"--cost <filepath>\"");
	variables_map vm;
	store(parse_command_line(argc,argv,desc),vm);
	notify(vm);
	
	if(vm.count("help") || 0 == vm.count("get") && 0 == vm.count("push") && 0 == vm.count("cost")) {
		cout<<desc;
		return EXIT_SUCCESS;
	}
	
	Storage storage("127.0.0.1",8000,"127.0.0.1",6666,"eosfilestore");
	
	if(1 == vm.count("get")) {
		bool retval = storage.get(txid_filename[0],txid_filename[1]);
		if(false == retval) {
			cout<<"failed to get file"<<endl;
			return EXIT_FAILURE;
		}
	} else if(1 == vm.count("push")) {
		bool retval;
		string txid;
		boost::tie(retval,txid) = storage.push(filename);
		if(false == retval) {
			cout<<"failed to push file to blockchain"<<endl;
			return EXIT_FAILURE;
		} else
			cout<<txid<<endl;
	} else if(1 == vm.count("cost")) {
		int num;
		float net,cpu;
		boost::tie(num,net,cpu) = storage.cost(filename);
		cout<<"will take "<<num<<" transactions to upload"<<endl
			<<"will cost "<<net<<" KB net bandwidth"<<endl
			<<"will cost "<<cpu<<" ms cpu time"<<endl;
	} else {
		cout<<"invalid command!"<<endl;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

