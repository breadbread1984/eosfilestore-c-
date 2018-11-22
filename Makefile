CXXFLAGS=-I/usr/include `pkg-config --cflags libcrypto++` -std=c++14 -g2
LIBS=-L/usr/lib/x86_64-linux-gnu/ `pkg-config --libs libcrypto++` -lboost_program_options -lboost_filesystem -lboost_system -lboost_regex -lpthread
OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: cli eosfilestore

cli: $(OBJS)
	$(CXX) $^ $(LIBS) -o ${@}

.PHONY: eosfilestore

install: eosfilestore
	cleos --wallet-url http://127.0.0.1:6666 -u http://127.0.0.1:8001 wallet import --private-key 5JzUiVhF1MYB4U391RmpsFTdeH9vBHhubmiT6tGzQacfBgqUmo2
	cleos --wallet-url http://127.0.0.1:6666 -u http://127.0.0.1:8001 system newaccount eosio.stake eosfilestore EOS4uK3ZvDBZ9tRB9QUPZuGmwszn8XRtcoVeR4uwbVcEcF42eXNSX --stake-cpu "10000000.0000 SYS" --stake-net "10000.0000 SYS" --buy-ram "10000.0000 SYS"
	cleos --wallet-url http://127.0.0.1:6666 -u http://127.0.0.1:8001 set contract eosfilestore eosfilestore -p eosfilestore

eosfilestore:
	make -C eosfilestore

clean:
	make -C eosfilestore clean
	$(RM) cli $(OBJS)

