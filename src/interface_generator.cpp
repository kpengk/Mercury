#include "interface_generator.h"

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <regex>
#include <set>

#include <clang/Tooling/Tooling.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

#include "analysis_action.h"
#include "uuid.hpp"


namespace glasssix::ymer
{
	namespace
	{
		static std::string replace_regex(std::string_view str, std::string_view old_value, std::string_view new_value)
		{
			const std::regex reg_str(old_value.data());
			return std::regex_replace(str.data(), reg_str, new_value.data());
		}

		static void replace_text(std::string& str, std::string_view old_value, std::string_view new_value)
		{
			for (std::string::size_type pos{ 0 }; pos != std::string::npos; pos += new_value.length()) {
				if ((pos = str.find(old_value, pos)) != std::string::npos)
					str.replace(pos, old_value.length(), new_value);
				else
					break;
			}
		}

		static bool is_base_type(std::string_view type)
		{
			static std::set<std::string> base_type{
				u8"int8", u8"int16", u8"int32", u8"int64", u8"int",
				u8"uint8", u8"uint16", u8"uint32", u8"uint64",
				u8"float", u8"double", u8"_Bool", u8"bool", u8"char"
			};
			return base_type.find(type.data()) != base_type.cend();
		}

		static std::string read_file(std::string_view file, bool* ok = nullptr)
		{
			std::ifstream ifs(file.data());
			if (!ifs.is_open())
			{
				fprintf(stderr, u8"\033[31mOpen file fail. [%s].\n\033[0m", file.data());
				if (ok)
					*ok = false;

				return std::string();
			}

			ifs.seekg(0, std::ios::end);
			const size_t len = ifs.tellg();
			ifs.seekg(0, std::ios::beg);

			std::string buffer(len, ' ');
			ifs.read(buffer.data(), len);
			ifs.close();

			if (ok)
				*ok = true;

			return buffer;
		}

		static bool generate_code_file(std::string_view code_file_name, std::string_view template_code, const nlohmann::json& obj)
		{
			std::ofstream ofs(code_file_name.data());
			if (!ofs.is_open())
			{
				fprintf(stderr, u8"open output file fail. [%s].\n", code_file_name.data());
				return false;
			}
			try
			{
				inja::render_to(ofs, template_code, obj);
			}
			catch (const std::exception& e)
			{
				fprintf(stderr, u8"\033[31mGenerate code exception: %s.\n\033[0m", e.what());
				ofs.close();
				return false;
			}
			ofs.close();

			return true;
		}

		static std::vector<std::string> depend_header(const std::string& code)
		{
			static const std::regex pattern(R"(#\s*include\s*[<"]\s*([\w/\\]+)[\.]?[hpxc]*\s*[>"])");

			std::vector<std::string> result;

			std::smatch match_result;
			auto iter_start = code.cbegin();
			auto iter_end = code.cend();
			while (regex_search(iter_start, iter_end, match_result, pattern))
			{
				assert(match_result.size() == 2);
				result.push_back(match_result.str(1));

				iter_start = match_result[0].second;
			}


			return result;
		}
	}

	class interface_generator::impl
	{
	public:
		bool load_field_map(std::string_view file_name)
		{
			field_mapping_.clear();

			std::ifstream ifs(file_name.data());
			if (!ifs.is_open())
			{
				return false;
			}
			nlohmann::json field_json;
			ifs >> field_json;
			ifs.close();

			if (field_json.empty())
			{
				return false;
			}

			if (!field_json.contains(u8"field_map"))
			{
				return false;
			}

			field_json.at(u8"field_map").get_to(field_mapping_);

			return true;
		}

		bool load_predefined(std::string_view file_name)
		{
			predefined_file_ = file_name;
			bool ok{};
			read_file(file_name, &ok);
			return ok;
		}

		bool load_template(std::string_view interface_file, std::string_view impl_file, std::string_view func_file)
		{
			template_code_ = read_file(interface_file);
			impl_template_code_ = read_file(impl_file);
			func_template_code_ = read_file(func_file);

			return !(template_code_.empty() || impl_template_code_.empty() || func_template_code_.empty());
		}

		void set_include_path(std::vector<std::string> dirs)
		{
			run_args_.clear();
			for (std::string_view path : dirs)
			{
				run_args_.push_back(std::string(u8"-I").append(path));
			}
		}

		std::string_view set_output_path(std::string_view path)
		{
			if (path.empty())
				path = "./";

			auto syspath = std::filesystem::path(path);
			if (!std::filesystem::exists(syspath))
			{
				std::filesystem::create_directories(syspath);
			}

			std::string str_path = std::filesystem::absolute(syspath).string();
			if (const char last = *(--str_path.end()); last != '\\' && last != '/')
			{
#ifdef _WIN32
				str_path.push_back('\\');
#else
				str_path.push_back('/');
#endif // WIN32_
			}
			output_path_ = str_path;
			return output_path_;
		}

		bool run_file(std::string_view file_name)
		{
			// read file
			bool ok;
			std::string code = read_file(file_name, &ok);
			if (!ok)
			{
				return false;
			}

			return run_code(code);
		}

		bool run_code(std::string_view code)
		{
			interface_json_.clear();
			std::vector<ymer::interface_decl> all_class_info;
			if (code.empty() || !analysis(code, all_class_info) || all_class_info.empty())
			{
				return false;
			}

			auto decl = --all_class_info.end();
			if (decl->functions.empty() && decl->fields.empty())
			{
				return false;
			}

			// Interface fields to function
			for (const param_decl& field : decl->fields)
			{
				if (field.attr == 0)//set
				{
					function_decl fun_decl;
					fun_decl.func_name = u8"set_" + field.name;
					fun_decl.is_const_func = false;
					fun_decl.return_type = u8"void";
					fun_decl.return_void = true;
					fun_decl.params = { field };
					decl->functions.push_back(fun_decl);
				}
				else if (field.attr == 1)//get
				{
					function_decl fun_decl;
					fun_decl.func_name = u8"get_" + field.name;
					fun_decl.is_const_func = true;
					fun_decl.return_type = field.type;
					fun_decl.return_void = false;
					decl->functions.push_back(fun_decl);
				}
				else if (field.attr == 2)//set-get
				{
					function_decl set_fun_decl;
					set_fun_decl.func_name = u8"set_" + field.name;
					set_fun_decl.is_const_func = false;
					set_fun_decl.return_type = u8"void";
					set_fun_decl.return_void = true;
					param_decl param = field;
					param.attr = 0;
					set_fun_decl.params = { param };
					decl->functions.push_back(set_fun_decl);

					function_decl get_fun_decl;
					get_fun_decl.func_name = u8"get_" + field.name;
					get_fun_decl.is_const_func = true;
					get_fun_decl.return_type = field.type;
					get_fun_decl.return_void = false;
					decl->functions.push_back(get_fun_decl);
				}
			}

			return generator(*decl);
		}

		std::vector<std::string> function_signature()
		{
			if (interface_json_.empty())
			{
				return std::vector<std::string>();
			}

			std::string func_interface;
			try
			{
				func_interface = inja::render(func_template_code_, interface_json_);
			}
			catch (const std::exception& e)
			{
				fprintf(stderr, u8"\033[31mGenerate code exception: %s.\n\033[0m", e.what());
				return std::vector<std::string>();
			}

			if (func_interface.empty())
				return std::vector<std::string>();

			std::vector<std::string> funcs;
			for (std::string::size_type pos{ 0 }; pos != std::string::npos;) {
				auto idx = func_interface.find('\n', pos);
				if (idx != std::string::npos)
					funcs.push_back(func_interface.substr(pos, idx - pos));
				else
					break;

				pos = idx + 1;
			}

			return funcs;
		}

		bool analysis(std::string_view code, std::vector<ymer::interface_decl>& result)
		{
			const auto& header = depend_header(code.data());

			// pretreatment
			std::string whole_code = "#include \"" + predefined_file_ + "\"\n" + code.data();

			static const std::array<std::string, 5> attributes{ u8"in", u8"out", u8"inout", u8"set", u8"get" };
			for (std::string_view attr : attributes)
			{
				char old_str[64]{};
				char new_str[64]{};
				sprintf_s(old_str, sizeof(old_str), u8"\\[\\s*\\[\\s*%s\\s*\\]\\s*\\]", attr.data());
				sprintf_s(new_str, sizeof(new_str), u8"__attribute__((annotate(\"%s\")))", attr.data());

				whole_code = replace_regex(whole_code, old_str, new_str);
			}

			// run
			const bool ret = clang::tooling::runToolOnCodeWithArgs(std::make_unique<ymer::analysis_action>(result), whole_code.c_str(), run_args_);
			if (ret)
			{
				(--result.end())->depend_class = header;
			}

			return ret;
		}

		bool generator(const interface_decl& decl)
		{
			// Transfer interface parameters to JSON
			interface_json_ = to_json(decl);

			const std::string code_file_name = output_path_ + decl.class_name + ".hpp";
			bool ret = generate_code_file(code_file_name, template_code_, interface_json_);

			const std::string code_impl_file_name = output_path_ + decl.class_name + "_impl.hpp";
			ret &= generate_code_file(code_impl_file_name, impl_template_code_, interface_json_);

			return ret;
		}

		std::string to_std_type(const std::string& type) const
		{
			std::string result = type;
			replace_text(result, "struct ", "");
			const auto iter = field_mapping_.find(type);
			if (iter != field_mapping_.end())
			{
				result = iter->second;
			}
			else
			{
				for (const auto& field : field_mapping_)
				{
					replace_text(result, field.first, field.second);
				}
			}

			return result;
		}

		nlohmann::json to_json(const interface_decl& decl)
		{
			nlohmann::json interface_json;
			interface_json[u8"depend_class"] = decl.depend_class;
			interface_json[u8"package_name"] = decl.package_name;
			interface_json[u8"class_name"] = decl.class_name;
			interface_json[u8"guid"] = glasssix::uuid::generate();

			nlohmann::json all_func_json = nlohmann::json::array();;
			for (const auto& func_decl : decl.functions)
			{
				nlohmann::json func_json;
				func_json[u8"func_name"] = func_decl.func_name;
				func_json[u8"return_type"] = to_std_type(func_decl.return_type);
				func_json[u8"is_const_func"] = func_decl.is_const_func;
				func_json[u8"return_void"] = func_decl.return_void;

				nlohmann::json all_param_json = nlohmann::json::array();
				for (const auto& param : func_decl.params)
				{
					nlohmann::json param_json;
					param_json[u8"attr"] = param.attr;
					param_json[u8"type"] = to_std_type(param.type);
					param_json[u8"base_type"] = is_base_type(param.type);
					param_json[u8"name"] = param.name;

					all_param_json.push_back(param_json);
				}

				func_json[u8"params"] = all_param_json;

				all_func_json.push_back(func_json);
			}
			interface_json[u8"functions"] = all_func_json;

			return interface_json;
		}

	private:
		std::string predefined_file_;
		std::string template_code_;
		std::string impl_template_code_;
		std::string func_template_code_;
		std::string output_path_;
		std::vector<std::string> run_args_;
		std::unordered_map<std::string, std::string> field_mapping_;
		nlohmann::json interface_json_;
	};



	interface_generator::interface_generator()
		:impl_(std::make_unique<impl>())
	{
	}

	interface_generator::~interface_generator()
	{
	}

	bool interface_generator::load_field_map(std::string_view file_name)
	{
		return impl_->load_field_map(file_name);
	}

	bool interface_generator::load_predefined(std::string_view file_name)
	{
		return impl_->load_predefined(file_name);
	}

	bool interface_generator::load_template(std::string_view interface_file, std::string_view impl_file, std::string_view func_file)
	{
		return impl_->load_template(interface_file, impl_file, func_file);
	}

	void interface_generator::set_include_path(std::vector<std::string> dirs)
	{
		impl_->set_include_path(dirs);
	}

	std::string_view interface_generator::set_output_path(std::string_view path)
	{
		return impl_->set_output_path(path);
	}

	bool interface_generator::run_file(std::string_view file_name)
	{
		return impl_->run_file(file_name);
	}

	bool interface_generator::run_code(std::string_view code)
	{
		return impl_->run_code(code);
	}

	std::vector<std::string> interface_generator::function_signature()
	{
		return impl_->function_signature();
	}
}
