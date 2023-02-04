#include "yaml_wrapper.hpp"

void YAML::TetsRead()
{
	YAML::Node config = YAML::LoadFile("config.yaml");

	if (config["Word"]) {
		std::cout << "Word is: " << config["Word"] << "\n";
	}

	const std::string username = config["height"].as<std::string>();
	const std::string password = config["width"].as<std::string>();

	std::cout << username << ", " << password << '\n';

	config["Word"] = "Changed";
	std::ofstream fout("config.yaml");
	fout << config;
	fout.close();
}

void YAML::TestWrite()
{
	YAML::Node node;  // starts out as null
	node["key"] = "value";  // it now is a map node
	node["seq"].push_back("first element");  // node["seq"] automatically becomes a sequence
	node["seq"].push_back("second element");

	node["mirror"] = node["seq"][0];  // this creates an alias
	node["seq"][0] = "1st element";  // this also changes node["mirror"]
	node["mirror"] = "element #1";  // and this changes node["seq"][0] - they're really the "same" node

	node["self"] = node;  // you can even create self-aliases
	node[node["mirror"]] = node["seq"];  // and strange loops :)

	std::ofstream fout("config2.yaml");
	fout << node;
	fout.close();
}
