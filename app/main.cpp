#include <iostream>

#include "interface_generator.h"
#include "cxxopts/cxxopts.hpp"
#include "cxxopts/cxxopts_extension.hpp"

namespace
{
	constexpr char* predefined_file = "data/predefined.hpp";
	constexpr char* map_file = "data/field.json";
	constexpr char* template_file = "data/interface_template.hpp";
	constexpr char* impl_template_file = "data/impl_template.hpp";
	constexpr char* func_template_file = "data/function_template.hpp";


	const glasssix::command_option option_help{ u8"h", u8"help", u8"Show help string." };
	const glasssix::command_option option_version{ u8"v", u8"version", u8"Show version string." };
	const glasssix::command_option option_input{ u8"i", u8"input", u8"Specify interface file.", cxxopts::value<std::string>()->no_implicit_value() };
	const glasssix::command_option option_output{ u8"o", u8"output", u8"Specify output path.", cxxopts::value<std::string>()->default_value(u8"./") };
	const glasssix::command_option option_depend{ u8"d", u8"dpend", u8"Specify dpend path.", cxxopts::value<std::string>()->default_value(u8"./") };
}

int main(int argc, char** argv) try
{
	cxxopts::Options options{ u8"interface_generator_app" };
	auto result = (options | option_help | option_version | option_input | option_output | option_depend).parse(argc, argv);

	if (result.count(option_help.name) != 0U)
	{
		auto str = options.help();
		std::fwrite(str.data(), sizeof(decltype(str)::value_type), str.size(), stdout);
		std::exit(EXIT_SUCCESS);
	}

	if (result.count(option_version.name) != 0U)
	{
		constexpr char str[] = u8"built with x86_64 (MSVC19) 20210930.";
		std::fwrite(str, 1, sizeof(str), stdout);
		std::exit(EXIT_SUCCESS);
	}

	if (result.count(option_input.name) == 0U)
	{
		std::string_view str = u8R"(The input interface file must be specified by "-i" or "--input".)";
		std::fwrite(str.data(), sizeof(decltype(str)::value_type), str.size(), stdout);
		std::exit(EXIT_SUCCESS);
	}

	auto input_file = result[option_input.name].as<std::string>();
	auto output_path = result[option_output.name].as<std::string>();
	auto depend_path = result[option_depend.name].as<std::string>();

	// get program dir
	const std::string_view bin_path(argv[0]);
	auto index = std::max(bin_path.find_last_of('\\') + 1, bin_path.find_last_of('/') + 1);
	const std::string dirPath(bin_path.substr(0, index));

	// generator
	glasssix::ymer::interface_generator handle;

	if (!handle.load_field_map(dirPath + map_file))
	{
		fprintf(stderr, "File not found. [%s%s]\n", dirPath.c_str(), map_file);
		std::exit(EXIT_SUCCESS);
	}
	if (!handle.load_predefined(dirPath + predefined_file))
	{
		fprintf(stderr, "File not found. [%s%s]\n", dirPath.c_str(), predefined_file);
		std::exit(EXIT_SUCCESS);
	}
	if (!handle.load_template(dirPath + template_file, dirPath + impl_template_file, dirPath + func_template_file))
	{
		fprintf(stderr, "File not found. [%s] or [%s] or [%s]\n", template_file, impl_template_file, func_template_file);
		std::exit(EXIT_SUCCESS);
	}

	handle.set_include_path(std::vector<std::string>{ {"./"}, { depend_path } });
	auto absolute_path = handle.set_output_path(output_path);
	fprintf(stdout, u8"Output path: %s\n", absolute_path.data());

	if (handle.run(input_file))
	{
		fprintf(stdout, "Code generated successfully.\n");
	}
	else
	{
		fprintf(stderr, u8"\033[31mFailed to generate interface.\033[0m\n");
	}

	return 0;
}
catch (const std::exception& ex)
{
	fprintf(stderr, "%s\n", ex.what());
}
