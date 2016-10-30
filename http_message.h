#include <iostream>
#include <string>
#include <vector>

using namespace std;

class http_message
{
private:
	// http_version can be "1.0" or "1.1"
	string http_version;
public:
	http_message();
	// get the version of http
	int get_version();

	void set_version(string version);

	virtual vector<char> encode() = 0;

	virtual void decode(vector<char> wire) = 0;
};


class http_request: public http_message
{
private:
	string method;

	string url;
public:
	http_request();

	string get_method();

	void set_method(string my_method);

	string get_url();

	void set_url(string my_url);

	virtual vector<char> encode();

	virtual void decode(vector<char> wire);
};

class http_response: public http_message
{
private:
	string status_code;

	vector<char> data;

public:
	http_response();

	string get_status_code();

	void set_status_code(string my_status_code);

	vector<char> get_data();

	void add_data(vector<char> new_data);

	virtual vector<char> encode();

	virtual void decode(vector<char> wire);
};