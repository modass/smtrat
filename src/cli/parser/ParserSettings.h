#pragma once

#include <smtrat-common/settings/Settings.h>
#include <smtrat-common/settings/SettingsParser.h>

namespace smtrat {
namespace parser {

struct ParserSettings {
	bool read_dimacs;
	bool read_opb;
	std::string input_file;
	bool disable_uf_flattening;
	bool disable_theory;
};

template<typename T>
void registerParserSettings(T& parser) {
	namespace po = boost::program_options;
	auto& settings = settings::Settings::getInstance();
	auto& s = settings.get<ParserSettings>("parser");

	parser.add("Parser settings").add_options()
		("dimacs", po::bool_switch(&s.read_dimacs), "parse input file as dimacs file")
		("opb", po::bool_switch(&s.read_opb), "parse input file as OPB file")
		("input-file", po::value<std::string>(&s.input_file), "path of the input file")
		("disable-uf-flattening", po::bool_switch(&s.disable_uf_flattening), "disable flattening of nested uninterpreted functions")
		("disable-theory", po::bool_switch(&s.disable_theory), "disable theory construction")
	;
}

}

inline const auto& settings_parser() {
	static const auto& s = settings::Settings::getInstance().get<parser::ParserSettings>("parser");
	return s;
}

}
