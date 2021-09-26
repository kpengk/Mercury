#pragma once

## for header in depend_class
#include "{{ header }}.hpp"
## endfor

#include <abi/consumer.hpp>

namespace glasssix::{{package_name}}
{
	struct {{class_name}};
}

namespace glasssix::exposing::impl
{
	template<> struct abi<{{package_name}}::{{class_name}}>
	{
		using identity_type = type_identity_interface;

		static constexpr guid id{ "{{ guid }}" };

		struct type : abi_unknown_object
		{
## for func in functions
##   if func.return_void
			virtual std::int32_t G6_ABI_CALL {{ func.func_name }}({% for arg in func.params %}{% if arg.attr == 0 %}abi_in_t{% else %}abi_out_t{% endif %}<{{ arg.type }}> {{ arg.name }}{% if not loop.is_last %}, {% endif %}{% endfor %}) noexcept = 0;
##   else
			virtual std::int32_t G6_ABI_CALL {{ func.func_name }}({% for arg in func.params %}{% if arg.attr == 0 %}abi_in_t{% else %}abi_out_t{% endif %}<{{ arg.type }}> {{ arg.name }}, {% endfor %}abi_out_t<{{ func.return_type }}> result) noexcept = 0;
##   endif
## endfor
		};
	};


	template<typename Derived>
	struct interface_vtable<Derived, {{package_name}}::{{class_name}}> : interface_vtable_base<Derived, {{package_name}}::{{class_name}}>
	{
## for func in functions
##   if func.return_void
		virtual std::int32_t G6_ABI_CALL {{ func.func_name }}({% for arg in func.params %}{% if arg.attr == 0 %}abi_in_t{% else %}abi_out_t{% endif %}<{{ arg.type }}> {{ arg.name }}{% if not loop.is_last %}, {% endif %}{% endfor %}) noexcept override
		{
##     for arg in func.params
##       if arg.attr == 1 and not arg.base_type
			{{ arg.type }} inner_{{ arg.name }}{ nullptr };
##       else if arg.attr == 2 and not arg.base_type
			{{ arg.type }} inner_{{ arg.name }}{ take_over_abi_from_void_ptr{ *{{ arg.name }} } };
##       endif
##     endfor
			auto inner_ret = abi_safe_call([&] { this->self().{{ func.func_name }}(
##     for arg in func.params
##       if arg.base_type
##         if arg.attr == 0
				{{ arg.name }}{% if not loop.is_last %}, {% endif %}
##         else
				*{{ arg.name }}{% if not loop.is_last %}, {% endif %}
##         endif
##       else
##         if arg.attr == 0
				create_from_abi<{{ arg.type }}>({{ arg.name }}){% if not loop.is_last %}, {% endif %}
##         else
				inner_{{ arg.name }}{% if not loop.is_last %}, {% endif %}
##         endif
##       endif
##     endfor
			); });
##     for arg in func.params
##       if (arg.attr == 1 or arg.attr == 2) and not arg.base_type
			*{{ arg.name }} = detach_abi(inner_{{ arg.name }});
##       endif
##     endfor
			return inner_ret;
		}
##   else
		virtual std::int32_t G6_ABI_CALL {{ func.func_name }}({% for arg in func.params %}{% if arg.attr == 0 %}abi_in_t{% else %}abi_out_t{% endif %}<{{ arg.type }}> {{ arg.name }}, {% endfor %}abi_out_t<{{ func.return_type }}> result) noexcept override
		{
##     for arg in func.params
##       if arg.attr == 1 and not arg.base_type
			{{ arg.type }} inner_{{ arg.name }}{ nullptr };
##       else if arg.attr == 2 and not arg.base_type
			{{ arg.type }} inner_{{ arg.name }}{ take_over_abi_from_void_ptr{ {{ arg.name }} } };
##       endif
##     endfor
			auto inner_ret = abi_safe_call([&] { *result = detach_abi(this->self().{{ func.func_name }}(
##     for arg in func.params
##       if arg.base_type
##         if arg.attr == 0
				{{ arg.name }}{% if not loop.is_last %}, {% endif %}
##         else
				*{{ arg.name }}{% if not loop.is_last %}, {% endif %}
##         endif
##       else
##         if arg.attr == 0
				create_from_abi<{{ arg.type }}>({{ arg.name }}){% if not loop.is_last %}, {% endif %}
##         else
				inner_{{ arg.name }}{% if not loop.is_last %}, {% endif %}
##         endif
##       endif
##     endfor
			)); });
##     for arg in func.params
##       if arg.attr == 1 and not arg.base_type
			*{{ arg.name }} = detach_abi(inner_{{ arg.name }});
##       endif
##     endfor
			return inner_ret;
		}
##   endif

## endfor
	};


	template<> struct abi_adapter<{{package_name}}::{{class_name}}>
	{
		template<typename Derived>
		struct type : enable_self_abi_awareness<Derived, {{package_name}}::{{class_name}}>
		{
## for func in functions
			{{ func.return_type }} {{ func.func_name }}({% for arg in func.params %}{{ arg.type }}{% if arg.attr == 1 or arg.attr == 2 %}&{% endif %} {{ arg.name }}{% if not loop.is_last %}, {% endif %}{% endfor %}){% if func.is_const_func %} const{% endif %}
			{
##   if not func.return_void
				{{ func.return_type }} result{};
				return (check_abi_result(this->self_abi().{{ func.func_name }}(
##     for arg in func.params
					{% if arg.attr == 0 %}get_abi{% else if arg.attr == 1 %}put_abi{% else if arg.attr == 2 %}put_abi_dangerous{% endif %}({{ arg.name }}), 
##     endfor
					put_abi(result))), result);
##   else
				check_abi_result(this->self_abi().{{ func.func_name }}(
##     for arg in func.params
					{% if arg.attr == 0 %}get_abi{% else if arg.attr == 1 %}put_abi{% else if arg.attr == 2 %}put_abi_dangerous{% endif %}({{ arg.name }}){% if not loop.is_last %},{% endif %}
##     endfor
				));
##   endif
			}
			
## endfor
		};
	};
}


namespace glasssix::{{package_name}}
{
	struct {{class_name}} : exposing::inherits<{{class_name}}>
	{
		using inherits::inherits;
	};
}
