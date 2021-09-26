## for func in functions
{{ func.return_type }} {{ func.func_name }}({% for arg in func.params %}{{ arg.type }}{% if arg.attr == 1 or arg.attr == 2 %}&{% endif %} {{ arg.name }}{% if not loop.is_last %}, {% endif %}{% endfor %}){% if func.is_const_func %} const{% endif %};
## endfor