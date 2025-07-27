#pragma once
#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct Location
{
	bool autoindex;
	int methods;
	std::pair<int, std::string> return_path;  // da 0 a 999
	std::string upload_dir;
	std::vector<std::string> cgi_extension;
	std::string root;
	std::vector<std::string> index;
	int client_max_body_size;
	std::map<int, std::string> error_page;
};

struct Server
{
	std::vector<std::string> server_name;
	std::vector<std::string> index;
	std::vector<std::pair<int, int> > listen;  // tra 1-65535
	std::string root;
	std::string upload_dir;
	bool autoindex;
	std::vector<std::string> cgi_extension;
	int methods;
	int client_max_body_size;
	std::map<int, std::string> error_page;  // must be between 300 and 599
	std::map<std::string, Location> location;
};

typedef std::pair<int, int> t_host;
typedef std::map<t_host, std::vector<Server> > t_serversMap;

void readFileAsString(std::ifstream& file, std::vector<Server>& main_vector);
int atoi_ip(const std::string& host);
void printServers(const std::vector<Server>& servers);
std::string ft_trim(const std::string& s);
