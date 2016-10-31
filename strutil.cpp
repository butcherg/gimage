

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream> 


std::string tostr(double t)
{ 
   std::ostringstream os; 
   os<<t; 
   return os.str(); 
} 



std::vector<std::string> split(std::string s, std::string delim)
{
	std::vector<std::string> v;
	size_t pos=0;
	size_t start;
	while (pos < s.length()) {
		start = pos;
		pos = s.find(delim,pos);
		if (pos == s.npos) {
			v.push_back(s.substr(start,s.length()-start));
			return v;
		}
		v.push_back(s.substr(start, pos-start));
		pos += delim.length();
	}
	return v;
}


std::map<std::string, std::string> parseparams(std::string params)
{
	std::map<std::string, std::string> p;
	std::vector<std::string> l = split(params,",");
	for (std::vector<std::string>::iterator it=l.begin(); it!=l.end(); ++it) {
		std::string name, val;
		std::vector<std::string> nameval = split(*it,"=");
		p[nameval[0]] = nameval[1];
	}
	return p;
}


