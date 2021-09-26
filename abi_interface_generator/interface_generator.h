#pragma once
#include <string_view>
#include <vector>
#include <memory>

#ifdef _WIN32
#ifdef INTERFACE_GENERATOR_EXPORT
#define INTERFACE_GENERATOR_API __declspec(dllexport)
#else
#define INTERFACE_GENERATOR_API __declspec(dllimport)
#endif
#else
#ifdef INTERFACE_GENERATOR_EXPORT
#define INTERFACE_GENERATOR_API __attribute__((visibility("default")))
#endif
#endif

namespace glasssix::ymer
{
	class INTERFACE_GENERATOR_API interface_generator
	{
	public:
		interface_generator();
		~interface_generator();

		bool load_field_map(std::string_view file_name);
		bool load_predefined(std::string_view file_name);
		bool load_template(std::string_view interface_file, std::string_view impl_file, std::string_view func_file);
		void set_include_path(std::vector<std::string> dirs);
		void set_output_path(std::string_view path);

		bool run(std::string_view file_name);

		std::vector<std::string> function_signature();

	private:
		class impl;
		std::unique_ptr<impl> impl_;
	};
}
