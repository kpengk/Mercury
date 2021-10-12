#pragma once
#include <string>
#include <vector>

namespace glasssix::ymer
{
	/* param_decl.attr
	 *   in:0, out: 1 inout: 2
	 *   set:0, get: 1 setget:2
	 */
	struct param_decl
	{
		std::int8_t attr;
		std::string type;
		std::string name;
	};

	struct function_decl
	{
		std::string func_name;
		std::string return_type;
		std::vector<param_decl> params;
		bool is_const_func;
		bool return_void;
	};


	struct interface_decl
	{
		std::string package_name;
		std::string class_name;
		std::vector<std::string> depend_class;
		std::vector<function_decl> functions;
		std::vector<param_decl> fields;
	};
}
