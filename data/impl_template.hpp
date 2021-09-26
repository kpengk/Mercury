#pragma once

#include "{{class_name}}.hpp"

#include <memory>

#include <abi/consumer.hpp>

namespace glasssix::{{package_name}}
{
	inline constexpr exposing::utf8_string_view {{class_name}}_qualified_name{ u8"g6.libnamexxx.image" };

	class {{class_name}}_impl : public exposing::implements<{{class_name}}_impl, {{class_name}}>, public exposing::make_external_qualified_name<{{class_name}}_qualified_name>
	{
	public:
		{{class_name}}_impl();
		~{{class_name}}_impl();

## for func in functions
		{{ func.return_type }} {{ func.func_name }}({% for arg in func.params %}{{ arg.type }}{% if arg.attr == 1 or arg.attr == 2 %}&{% endif %} {{ arg.name }}{% if not loop.is_last %}, {% endif %}{% endfor %}){% if func.is_const_func %} const{% endif %};
## endfor
	};
}
