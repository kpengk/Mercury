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
	const glasssix::command_option option_input{ u8"i", u8"input", u8"Specify interface file.", cxxopts::value<std::string>()->no_implicit_value() };
	const glasssix::command_option option_output{ u8"o", u8"output", u8"Specify output path.", cxxopts::value<std::string>()->default_value(u8"./") };
}

int main(int argc, char** argv) try
{
	cxxopts::Options options{ u8"interface_generator_app" };
	auto result = (options | option_help | option_input | option_output).parse(argc, argv);

	if (result.count(option_help.name) != 0U)
	{
		auto str = options.help();
		std::fwrite(str.data(), sizeof(decltype(str)::value_type), str.size(), stdout);
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

	glasssix::ymer::interface_generator handle;

	if (!handle.load_field_map(map_file))
	{
		fprintf(stderr, "File not found. [%s]\n", map_file);
		std::exit(EXIT_SUCCESS);
	}
	if (!handle.load_predefined(predefined_file))
	{
		fprintf(stderr, "File not found. [%s]\n", predefined_file);
		std::exit(EXIT_SUCCESS);
	}
	if (!handle.load_template(template_file, impl_template_file, func_template_file))
	{
		fprintf(stderr, "File not found. [%s] or [%s] or [%s]\n", template_file, impl_template_file, func_template_file);
		std::exit(EXIT_SUCCESS);
	}

	handle.set_include_path({ "./" });
	handle.set_output_path(output_path);

	if (handle.run(input_file))
	{
		fprintf(stdout, "Code generated successfully.\n");
	}

	return 0;
}
catch (const std::exception& ex)
{
	fprintf(stderr, "%s\n", ex.what());
}
