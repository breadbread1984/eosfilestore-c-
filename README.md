# eosfilestore c++
the project modifies the [eosfilestore](https://github.com/grigio/eosfilestore) contract and implement an on chain storage controller. the controller is inspired from the above mentioned project as well.

### how to build

build everything with command

```bash
make
```

### how to install

modify the makefile according to your specific configuration and execute command

```bash
make install
```

### how to try

1. upload file to EOS blockchain with command

```bash
./cli --push <path/to/file> 
```

the last line of message output by cli is the first transaction from where you can download the whole file uploaded.

2. download file from EOS blockchain with command

```bash
./cli --get <transaction id> <path/to/save> 
```

3. estimate the cost of uploading a specific file

```bash
./cli --cost <path/to/file>
```

