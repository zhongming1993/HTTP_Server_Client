#include <netinet/in.h>
#include "http_message.h"

http_message::http_message()
{
	http_version = "1.0";
}

int http_message::get_version()
{
	if (http_version == "1.0")
		return 0;
	if (http_version == "1.1")
		return 1;
	else return -1;
}

void http_message::set_version(string version)
{
	if (version == "1.1" || version == "1.0")
		http_version = version;
	else
		cerr << "http version " << version << " not supported" << endl;
}

http_request::http_request()
{
	method = "GET";
}

string http_request::get_method()
{
	return method;
}

void http_request::set_method(string my_method)
{
	method = my_method;
}

string http_request::get_url()
{
	return url;
}

void http_request::set_url(string my_url)
{
	url = my_url;
}


vector<char> http_request::encode()
{
	string encode_string;
	switch (get_version())
	{
		case 0: 
			encode_string += "1.0\r\n";
			break;
		case 1:
			encode_string += "1.1\r\n";
			break;
		default:
			encode_string += "1.0\r\n";
	}
	encode_string = encode_string + "method: " + get_method() + "\r\n";
	encode_string = encode_string + "url: " + get_url() + "\r\n";
	vector<char> result(encode_string.begin(), encode_string.end());
	return result;
}

void http_request::decode(vector<char> wire)
{
	string element;
	int index = 0;
	while (wire[index] != '\r')
	{
		element += wire[index];
		index++;
	}
	set_version(element);

	element.clear();
	index += 2;
	while (wire[index] != '\r')
	{
		element += wire[index];
		index++;
	}

	string tag = "method: ";
	size_t position = element.find(tag);
	if (position != string::npos)
	{
		element = element.substr(position + tag.size(), element.size() - tag.size());
		method = element;
	}
	else
		cerr << "http_request decoded without method" << endl;

	element.clear();
	index += 2;
	while (wire[index] != '\r')
	{
		element += wire[index];
		index++;
	}
	tag = "url: ";
	position = element.find(tag);
	if (position != string::npos)
	{
		element = element.substr(position + tag.size(), element.size() - tag.size());
		url = element;
	}
	else
		cerr << "http_request decoded without url" << endl;
}

http_response::http_response()
{
	status_code = "";
}

string http_response::get_status_code()
{
	return status_code;
}

void http_response::set_status_code(string my_status_code)
{
	status_code = my_status_code;
}

vector<char> http_response::get_data()
{
	return data;
}

void http_response::add_data(vector<char> new_data)
{
	for (auto i: new_data)
		data.push_back(i);
}

vector<char> http_response::encode()
{
	string encode_string;
	switch (get_version())
	{
		case 0: 
			encode_string += "1.0\r\n";
			break;
		case 1:
			encode_string += "1.1\r\n";
			break;
		default:
			encode_string += "1.0\r\n";
	}

	encode_string = encode_string + "status_code: " + get_status_code() + "\r\n";
	string data_string(data.begin(),data.end());
	encode_string = encode_string + "data: " +  data_string + "\r\n";
	vector<char> result(encode_string.begin(), encode_string.end());
	return result;
}

void http_response::decode(vector<char> wire)
{
	string element;
	int index = 0;
	while (wire[index] != '\r')
	{
		element += wire[index];
		index++;
	}
	set_version(element);

	element.clear();
	index += 2;
	while (wire[index] != '\r')
	{
		element += wire[index];
		index++;
	}

	string tag = "status_code: ";
	size_t position = element.find(tag);
	if (position != string::npos)
	{
		element = element.substr(position + tag.size(), element.size() - tag.size());
		status_code = element;
	}
	else
		cerr << "http_request decoded without status_code" << endl;

	element.clear();
	index += 2;
	while (index < (int)wire.size() && wire[index] != '\r')
	{
		element += wire[index];
		index++;
	}
	tag = "data: ";
	position = element.find(tag);
	if (position != string::npos)
	{
		element = element.substr(position + tag.size(), element.size() - tag.size());
		for (size_t i = 0; i < element.size(); i++)
			data.push_back(element[i]);
	}
	else
		cerr << "http_request decoded without data" << endl;
}