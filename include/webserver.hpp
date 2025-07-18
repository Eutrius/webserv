#include <string>
#include <vector>
#include <map>

struct Location {
    bool						autoindex;
    std::vector<std::string>	allow;
    std::map<int, std::string>	return_path; // da 0 a 999
    std::string					upload_dir;
    std::vector<std::string>	cgi_extension;
    std::string					root;
    std::string					index;
};

struct Server {
    std::vector<std::string>		server_name;
    std::string				    	index;
    int								listen; // tra 1-65535
    std::string						root;
    bool							autoindex;
    std::string						host;
    std::vector<std::string>		cgi_extension;
    std::vector<std::string>		allow;
    size_t							client_max_body_size;
    std::map<int, std::string>		error_page;     //must be between 300 and 599
    std::map<std::string, Location>	location;
};
