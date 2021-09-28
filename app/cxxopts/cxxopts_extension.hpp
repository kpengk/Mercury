#pragma once
#include <memory>
#include <string>

namespace glasssix
{
	struct command_option
	{
		std::string abbreviation;
		std::string name;
		std::string help_string;
		std::shared_ptr<const cxxopts::Value> value = cxxopts::value<bool>();
	};

	cxxopts::Options& operator|(cxxopts::Options& options, const command_option& command_option)
	{

		options.add_option({}, cxxopts::Option
			{
				command_option.abbreviation + ',' + command_option.name,
				command_option.help_string,
				command_option.value ? command_option.value : cxxopts::value<bool>()
			});

		return options;
	}
}
