#include "parser.hpp"

typedef std::vector<std::string> StringVector;
typedef std::map<std::string, StringVector> MapStringToVector;
typedef std::map<std::string, MapStringToVector> locationmap;

// -------------------------
//		  HELPERS
// -------------------------

int stringToInt(const std::string &str)
{
	std::istringstream iss(str);
	int num;
	if (!(iss >> num))
		throw std::runtime_error("Invalid integer: " + str);
	char remaining;
	if (iss >> remaining || str.length() > 9)
		throw std::runtime_error("Invalid integer: " + str);
	return (num);
}

std::string ft_trim(const std::string &s)
{
	std::string::size_type start = 0;
	while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
		++start;

	std::string::size_type end = s.size();
	while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
		--end;

	std::string result = s.substr(start, end - start);
	// if (result[result.size() - 1] != '/')
	// 	result += '/';
	return (result);
}

std::string ft_replace(const std::string &originale, const std::string &daSostituire, const std::string &daInserire)
{
	std::string risultato;
	risultato.reserve(originale.length());
	size_t pos = 0;
	size_t foundPos;
	size_t len = daSostituire.length();

	while ((foundPos = originale.find(daSostituire, pos)) != std::string::npos)
	{
		risultato.append(originale, pos, foundPos - pos);
		risultato += daInserire;
		pos = foundPos + len;
	}
	risultato.append(originale, pos, originale.length() - pos);
	return (risultato);
}

bool isNotValidIP(const std::string &ip)
{
	std::vector<std::string> parts;
	std::istringstream ss(ip);
	std::string part;

	while (std::getline(ss, part, '.'))
		parts.push_back(part);
	if (parts.size() != 4)
		return (true);
	for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it)
	{
		const std::string &p = *it;
		if (p.empty())
			return (true);
		for (std::string::const_iterator c_it = p.begin(); c_it != p.end(); ++c_it)
		{
			if (!isdigit(*c_it))
				return (true);
		}
		if ((p.size() > 1 && p[0] == '0') || p.size() > 3)
			return (true);
		int num = stringToInt(p);
		if (num < 0 || num > 255)
			return (true);
	}
	return (false);
}

int atoi_ip(const std::string &host)
{
	std::istringstream ss(host);
	std::string byte;
	int ip = 0;
	int pos = 24;

	if (host == "localhost")
		return (2130706433);
	if (isNotValidIP(host))
		throw std::runtime_error(host + " is not a valid ip");
	while (std::getline(ss, byte, '.'))
	{
		ip = ip | std::atoi(byte.c_str()) << pos;
		pos -= 8;
	}
	return (ip);
}

void parseListen(std::string &input, std::pair<int, int> &tup)
{
	size_t colon_pos = input.find(':');

	if (colon_pos == std::string::npos)
	{
		tup.first = 0;
		tup.second = stringToInt(input);
	}
	else
	{
		tup.first = atoi_ip(input.substr(0, colon_pos));
		tup.second = stringToInt(input.substr(colon_pos + 1));
	}
	if (tup.second < 1 || tup.second > 65535)
		throw std::runtime_error("Server port must be between 1 - 65535");
}

void checkServerNames(std::vector<Server> &servers)
{
	if (!servers.size())
		throw std::runtime_error("The program requires at least 1 server.");
	for (size_t i = 1; i < servers.size(); ++i)
	{
		Server &s = servers[i];
		for (size_t j = 0; j < s.listen.size(); ++j)
		{
			for (size_t ii = 0; ii < i; ++ii)
			{
				Server &t = servers[ii];
				if (std::find(t.listen.begin(), t.listen.end(), s.listen[j]) != t.listen.end())
				{
					for (size_t k = 0; k < t.server_name.size(); ++k)
						s.server_name.erase(std::remove(s.server_name.begin(), s.server_name.end(), t.server_name[k]),
						                    s.server_name.end());
				}
			}
		}
	}
}

void printServers(const std::vector<Server> &servers)
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		const Server &s = servers[i];
		std::cout << "=== Server " << i << " ===\n";

		std::cout << "server_name: ";
		for (size_t j = 0; j < s.server_name.size(); ++j)
			std::cout << s.server_name[j] << " ";
		std::cout << "\n";

		std::cout << "index: ";
		for (size_t j = 0; j < s.index.size(); ++j)
			std::cout << s.index[j] << " ";
		std::cout << "\n";

		std::cout << "listen: ";
		for (size_t j = 0; j < s.listen.size(); ++j)
			std::cout << "(" << s.listen[j].first << ", " << s.listen[j].second << ") ";
		std::cout << "\n";

		std::cout << "root: " << s.root << "\n";
		std::cout << "upload_dir: " << s.upload_dir << "\n";
		std::cout << "autoindex: " << (s.autoindex ? "on" : "off") << "\n";
		std::cout << "methods: " << s.methods << "\n";
		std::cout << "client_max_body_size: " << s.client_max_body_size << "\n";

		std::cout << "cgi_extension: ";
		for (size_t j = 0; j < s.cgi_extension.size(); ++j)
			std::cout << s.cgi_extension[j] << " ";
		std::cout << "\n";

		std::cout << "error_page:\n";
		for (std::map<int, std::string>::const_iterator it = s.error_page.begin(); it != s.error_page.end(); ++it)
			std::cout << "  [" << it->first << "] => " << it->second << "\n";

		if (s.location.size())
			std::cout << "Locations:\n";
		else
			std::cout << "no locations\n";
		for (std::map<std::string, Location>::const_iterator lit = s.location.begin(); lit != s.location.end(); ++lit)
		{
			const std::string &path = lit->first;
			const Location &loc = lit->second;

			std::cout << "  Location: " << path << "\n";
			std::cout << "    autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
			std::cout << "    methods: " << loc.methods << "\n";
			std::cout << "    return_path: (" << loc.return_path.first << ", " << loc.return_path.second << ")\n";
			std::cout << "    upload_dir: " << loc.upload_dir << "\n";
			std::cout << "    client_max_body_size: " << loc.client_max_body_size << "\n";
			std::cout << "    root: " << loc.root << "\n";

			std::cout << "    index: ";
			for (size_t j = 0; j < loc.index.size(); ++j)
				std::cout << loc.index[j] << " ";
			std::cout << "\n";

			std::cout << "    cgi_extension: ";
			for (size_t j = 0; j < loc.cgi_extension.size(); ++j)
				std::cout << loc.cgi_extension[j] << " ";
			std::cout << "\n";

			std::cout << "    error_page:\n";
			for (std::map<int, std::string>::const_iterator eit = loc.error_page.begin(); eit != loc.error_page.end();
			     ++eit)
				std::cout << "      [" << eit->first << "] => " << eit->second << "\n";
		}

		std::cout << "\n";
	}
}


// -------------------------
//		  CHECKERS
// -------------------------

void checkListen(std::vector<std::string> vect, std::vector<std::pair<int, int> > &listen)
{
	std::vector<std::pair<int, int> > res;
	for (size_t i = 0; i < vect.size(); ++i)
	{
		std::pair<int, int> ll;
		parseListen(vect[i], ll);
		res.push_back(ll);
	}
	listen = res;
}

void checkServer_name(std::vector<std::string> vect, std::vector<std::string> &server_name)
{
	server_name = vect;
}

void checkRoot(std::vector<std::string> vect, std::string &root)
{
	if (vect.size() > 1)
		throw std::runtime_error("\"root\" directive is duplicate");
	root = vect[0];
}

void checkIndex(std::vector<std::string> vect, std::vector<std::string> &index)
{
	index = vect;
}

void checkClient_max_body_size(std::vector<std::string> vect, int &client_max_body_size)
{
	if (vect.size() != 1)
		throw std::runtime_error("\"client_max_body_size\" directive must have 1 argument");
	client_max_body_size = stringToInt(vect[0]);
}

void checkMethods(std::vector<std::string> vect, int &methods)
{
	methods = 0;
	for (size_t i = 0; i < vect.size(); ++i)
	{
		if (vect[i] != "GET" && vect[i] != "POST" && vect[i] != "DELETE")
			throw std::runtime_error("Unknown method, valid only: GET POST DELETE");
	}
	if (std::find(vect.begin(), vect.end(), "GET") != vect.end())
		methods += 1;
	if (std::find(vect.begin(), vect.end(), "POST") != vect.end())
		methods += 2;
	if (std::find(vect.begin(), vect.end(), "DELETE") != vect.end())
		methods += 4;
}

void checkAutoindex(std::vector<std::string> vect, bool &autoindex)
{
	if (vect.size() > 1)
		throw std::runtime_error("\"autoindex\" directive is duplicate");
	if (vect[0] == "on")
		autoindex = true;
	else if (vect[0] == "off")
		autoindex = false;
	else
		throw std::runtime_error("invalid value \"" + vect[0] +
		                         "\" in \"autoindex\" directive, it must be \"on\" or \"off\"");
}

void checkUpload_dir(std::vector<std::string> vect, std::string &upload_dir)
{
	if (vect.size() != 1)
		throw std::runtime_error("invalid number of arguments in \"upload_dir\" directive");
	upload_dir = vect[0];
}

void checkCgi_extension(std::vector<std::string> vect, std::vector<std::string> &cgi_extension)
{
	for (size_t i = 0; i < vect.size(); ++i)
	{
		if (vect[i] != "py" && vect[i] != "php")
			throw std::runtime_error("Unknown cgi_extension, valid only: py php");
	}
	cgi_extension = vect;
}

void checkError_page(std::vector<std::string> vect, std::map<int, std::string> &error_page, int key)
{
	std::ostringstream error_msg;
	error_msg << "\"error_page\" value \"" << key << "\" must be between 300 and 599";
	if (key < 300 || key > 599)
		throw std::runtime_error(error_msg.str());
	error_page[key] = vect[0];
}

void checkReturn(std::vector<std::string> vect, std::pair<int, std::string> &return_path, int key)
{
	std::ostringstream error_msg;
	error_msg << "\"return\" value \"" << key << "\" must be between 0 and 999";
	if (key < 0 || key > 999)
		throw std::runtime_error(error_msg.str());
	std::pair<int, std::string> res;
	res.first = key;
	if (vect.size() == 2)
		res.second = vect[1];
	else
		res.second = "";
	return_path = res;
}

// -------------------------
//		  PARSER
// -------------------------

void validatelocation(locationmap &m, Server &serverx)
{
	for (locationmap::const_iterator it = m.begin(); it != m.end(); ++it)
	{
		serverx.location[it->first].autoindex = serverx.autoindex;
		serverx.location[it->first].methods = serverx.methods;
		serverx.location[it->first].return_path = std::pair<int, std::string>();  // da 0 a 999
		serverx.location[it->first].return_path.first = -1;
		serverx.location[it->first].upload_dir = serverx.upload_dir;
		serverx.location[it->first].cgi_extension = serverx.cgi_extension;
		serverx.location[it->first].root = serverx.root;
		serverx.location[it->first].index = serverx.index;
		serverx.location[it->first].client_max_body_size = serverx.client_max_body_size;
		serverx.location[it->first].error_page = serverx.error_page;
		for (MapStringToVector::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
		{  // chiave jt->first; valore jt->second
			if (jt->first == "root")
				checkRoot(jt->second, serverx.location[it->first].root);
			else if (jt->first == "index")
				checkIndex(jt->second, serverx.location[it->first].index);
			else if (jt->first == "methods")
				checkMethods(jt->second, serverx.location[it->first].methods);
			else if (jt->first == "autoindex")
				checkAutoindex(jt->second, serverx.location[it->first].autoindex);
			else if (jt->first == "upload_dir")
				checkUpload_dir(jt->second, serverx.location[it->first].upload_dir);
			else if (jt->first == "cgi_extension")
				checkCgi_extension(jt->second, serverx.location[it->first].cgi_extension);
			else if (jt->first == "client_max_body_size")
				checkClient_max_body_size(jt->second, serverx.location[it->first].client_max_body_size);
			else if (jt->first.length() == 3 && isdigit(jt->first[0]) && isdigit(jt->first[1]) && isdigit(jt->first[2]))
				checkError_page(jt->second, serverx.location[it->first].error_page, stringToInt(jt->first));
			else if (jt->first == "return")
			{
				if (it->second.size() != 1)
					throw std::runtime_error("error: \"return\" should be the only directive");
				if (jt->second.size() < 1 || jt->second.size() > 2)
					throw std::runtime_error("error: \"return\" takes one or two arguments");
				checkReturn(jt->second, serverx.location[it->first].return_path, stringToInt(jt->second[0]));
			}
			else if (jt->first == "server_name")
				throw std::runtime_error("server_name cannot be overridden inside a location block");
			else if (jt->first == "listen")
				throw std::runtime_error("ip:port cannot be overridden inside a location block");
			else
				throw std::runtime_error("Is not a valid error_page number: " + it->first);
		}
	}
}

void validateserver(std::map<std::string, std::vector<std::string> > m, Server &serverx)
{
	std::vector<std::pair<int, int> > default_listen;
	std::pair<int, int> ll;
	ll.first = 0;
	ll.second = 80;
	default_listen.push_back(ll);

	serverx.listen = default_listen;                     // Default HTTP port
	serverx.server_name = std::vector<std::string>();    // No domain configured (empty array)
	serverx.root = "";                                   // No root directory configured (empty string)
	serverx.index = std::vector<std::string>();          // Common default page
	serverx.upload_dir = "";                             // No upload directory
	serverx.autoindex = false;                           // Directory listing disabled
	serverx.cgi_extension = std::vector<std::string>();  // No CGI extensions enabled
	serverx.methods = 7;                                 // Allowed all methods
	serverx.client_max_body_size = 1048576;              // 1MB (common default value)
	serverx.error_page = std::map<int, std::string>();   // No custom error pages
	for (MapStringToVector::const_iterator it = m.begin(); it != m.end(); ++it)
	{  // chiave it->first; valore it->second
		if (it->first == "listen")
			checkListen(it->second, serverx.listen);
		else if (it->first == "server_name")
			checkServer_name(it->second, serverx.server_name);
		else if (it->first == "root")
			checkRoot(it->second, serverx.root);
		else if (it->first == "index")
			checkIndex(it->second, serverx.index);
		else if (it->first == "client_max_body_size")
			checkClient_max_body_size(it->second, serverx.client_max_body_size);
		else if (it->first == "methods")
			checkMethods(it->second, serverx.methods);
		else if (it->first == "autoindex")
			checkAutoindex(it->second, serverx.autoindex);
		else if (it->first == "upload_dir")
			checkUpload_dir(it->second, serverx.upload_dir);
		else if (it->first == "cgi_extension")
			checkCgi_extension(it->second, serverx.cgi_extension);
		else if (it->first == "return")
			throw std::runtime_error("Return should be inside a location");
		else if (it->first.length() == 3 && isdigit(it->first[0]) && isdigit(it->first[1]) && isdigit(it->first[2]))
			checkError_page(it->second, serverx.error_page, stringToInt(it->first));
		else
			throw std::runtime_error("Is not a valid error_page number: " + it->first);
	}
	if (serverx.upload_dir == "")
		throw std::runtime_error("Every server needs an upload directory");
}

void removeLocationInPlace(std::string &input)
{
	size_t pos = input.find("location ", 0);
	size_t start_pos, end_pos;

	while (input.find("location ", pos) != std::string::npos && (input.find("location ", pos) < input.find("}", pos) || input.find("}", pos) < input.find(";", pos)))
	{
		start_pos = input.find("location ", pos);  // to do: assicurarsi che prima ci sia " ;}"
		end_pos = input.find_first_not_of(' ', start_pos);
		end_pos = input.find('{', end_pos);
		if (input.substr(start_pos, end_pos - start_pos + 1).find(";") == std::string::npos)
		{
			end_pos = input.find('}', end_pos);
			input.erase(start_pos, end_pos - start_pos + 1);
		}
		pos = start_pos + 1;
	}
}

std::string removeComments(const std::string &input)
{
    std::istringstream iss(input);
    std::ostringstream oss;
    std::string line;

    while (std::getline(iss, line))
	{
        size_t pos = line.find('#');
        if (pos != std::string::npos)
            line.erase(pos);
        if (!line.empty())
            oss << line << ' ';
    }
    return oss.str();
}

void splitStringToMap(const std::string &input, std::map<std::string, std::vector<std::string> > &result)
{
	size_t semicolonPos = input.find(';');

	std::string stringToProcess;
	if (semicolonPos == std::string::npos)
		throw std::runtime_error("expecting \";\" before end of section");
	stringToProcess = input.substr(0, semicolonPos);
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = 0;

	while (end < stringToProcess.length())
	{
		end = stringToProcess.find_first_of(" ", start);
		if (end == std::string::npos)
		{
			end = stringToProcess.length();
		}

		if (end > start)
		{
			std::string token = stringToProcess.substr(start, end - start);
			if (!token.empty())
			{
				tokens.push_back(token);
			}
		}

		start = stringToProcess.find_first_not_of(" ", end);
		if (start == std::string::npos)
		{
			break;
		}
	}

	if (tokens.empty())
		throw std::runtime_error("unexpected \";\"");

	std::vector<std::string> newValues;

	static const std::string permittedDirectives[] = {"listen", "server_name", "root", "index", "client_max_body_size", "methods", "autoindex", "upload_dir", "cgi_extension", "error_page", "return"};
	static const std::vector<std::string> permittedList(permittedDirectives, permittedDirectives + 11);
	if (std::find(permittedList.begin(), permittedList.end(), tokens[0]) == permittedList.end())
		throw std::runtime_error("Unknown directive: " + tokens[0]);
	if (tokens[0] == "error_page")
	{
		if (tokens.size() < 3)
			throw std::runtime_error("invalid number of arguments in \"error_page\" directive");
		for (size_t i = 1; i < tokens.size() - 1; ++i)
		{
			if (result.find(tokens[i]) == result.end())
			{
				newValues.push_back(tokens.back());
				result[tokens[i]] = newValues;
			}
		}
		return;
	}

	std::string key = tokens[0];
	for (size_t i = 1; i < tokens.size(); ++i)
		newValues.push_back(tokens[i]);
	if (!newValues.size())
		throw std::runtime_error(key + " directive requires an argument");
	if (tokens[0] == "return" && result.size())
		throw std::runtime_error("error: \"return\" should be the only directive");

	// Se la chiave esiste già
	if (result.find(key) != result.end())
	{
		// Aggiung i nuovi valori a quelli esistenti
		for (std::vector<std::string>::const_iterator it = newValues.begin(); it != newValues.end(); ++it)
			result[key].push_back(*it);
	}
	else
		result[key] = newValues;  // Se la chiave non esiste
}


void parseinserver(std::string &file, Server &serverx)
{
	size_t i;

	std::map<std::string, std::vector<std::string> > mappa;
	locationmap mappa_location;
	const size_t npos = std::string::npos;
	while (file.find_first_not_of(' ') != std::string::npos && file[file.find_first_not_of(' ')] != '}')
	{
		std::string	directive;
		if (file.find_first_not_of(" ") != npos)
			file = file.substr(file.find_first_not_of(" "));
		directive = file.substr(0,file.find(" "));
		if (directive == "location")
		{
			file = file.substr(8);
			std::map<std::string, std::vector<std::string> > mappa2;
			size_t pos1;
			size_t pos2;
			std::vector<size_t> candidates;
			pos1 = file.find_first_not_of(" ");
			pos2 = file.find("{");
			if (pos2 == npos || file.substr(0, pos2).find(";") != npos)
				throw std::runtime_error("location section need an argument and \"{\"");
			if (pos2 == npos || pos1 == pos2)
				throw std::runtime_error("location section need an argument and \"{\"");
			std::string key = ft_trim(file.substr(pos1, pos2 - 1));
			file = file.substr(pos2 + 1);
			pos1 = file.find(";");
			pos2 = file.find("}");
			if (pos2 == npos)
				throw std::runtime_error("location section needs to end with \"}\"");
			while (file.find_first_not_of(' ') != std::string::npos && file[file.find_first_not_of(' ')] != '}')
			{
				splitStringToMap(file, mappa2);
				i = file.find_first_of(";");
				file = file.substr(i + 1);
			}
			if (mappa_location.find(key) != mappa_location.end())
				throw std::runtime_error("location is duplicate: " + key);
			if (key.find(" ") != npos)
				throw std::runtime_error("location directive takes one argument and one block {}");
			if (key.find(";") != npos)
				throw std::runtime_error("directive \"location\" has no opening \"{\"");
			file = file.substr(file.find_first_of("}") + 1);
			mappa_location[key] = mappa2;
		}
		else
		{
			splitStringToMap(file, mappa);
			i = file.find_first_of(";");
			file = file.substr(i + 1);
		}
	}
	validateserver(mappa, serverx);
	validatelocation(mappa_location, serverx);
}

void parse(std::string file, std::vector<Server> &main_vector)
{
	size_t i = 0;
	size_t j = 0;
	const size_t npos = std::string::npos;
	while (file.find_first_not_of(" ") != npos)
	{
		i = file.find_first_not_of(" ");
		if (i == std::string::npos || file.compare(i, 6, "server"))
			throw std::runtime_error("The only directive in file should be \"server\", unknown directive in file: " + file.substr(file.find_first_not_of(" "), file.find_first_of(" ", file.find_first_not_of(" ")) - file.find_first_not_of(" ")));
		file = file.substr(i + 6);
		i = file.find_first_not_of(" ");
		if (i == std::string::npos || file.compare(i, 1, "{"))
			throw std::runtime_error("location directive need {");
		file = file.substr(i + 1);
		main_vector.resize(main_vector.size() + 1);
		parseinserver(file, main_vector[j]);
		j++;
		i = file.find_first_not_of(" ");
		if (file[i] != '}')
			throw std::runtime_error(" unexpected end of file, expecting \"}\"");
		file = file.substr(i + 1);
	}
	checkServerNames(main_vector);
	//printServers(main_vector);
}


void readFileAsString(std::ifstream &file, std::vector<Server> &main_vector)
{
	std::ostringstream contenutoStream;
	contenutoStream << file.rdbuf();

	std::string contenutoFiltrato = removeComments(contenutoStream.str());
	contenutoFiltrato = ft_replace(contenutoFiltrato, "\t", " ");
	contenutoFiltrato = ft_replace(contenutoFiltrato, "\r", " ");
	contenutoFiltrato = ft_replace(contenutoFiltrato, "\n", " ");
	parse(contenutoFiltrato, main_vector);
}