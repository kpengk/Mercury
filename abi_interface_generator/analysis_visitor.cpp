#include "analysis_visitor.h"
#include <clang/AST/Type.h>
#include <iostream>

#pragma comment(lib, "Version")


namespace glasssix::ymer
{
	analysis_visitor::analysis_visitor(clang::ASTContext* context, std::vector<interface_decl>& result)
		: context_(context), interface_list_(result)
	{
		interface_list_.clear();
	}

	analysis_visitor::~analysis_visitor()
	{
	}

	std::string analysis_visitor::line_info(clang::SourceLocation location)
	{
		clang::FullSourceLoc full_location = context_->getFullLoc(location);
		if (full_location.isValid())
		{
			return full_location.getLineNumber() + ":" + full_location.getColumnNumber();
		}
		return std::string();
	}

	bool analysis_visitor::VisitCXXRecordDecl(clang::CXXRecordDecl* record_decl)
	{
		// Verify the interface is valid
		if (!record_decl->hasDefinition())
		{
			return true;
		}

		/*if (record_decl->hasDirectFields())
		{
			std::cout << "Line: " << line_info(record_decl->getBeginLoc());
			printf("\033[31mError: has direct fields.\033[0m\n");
			return true;
		}*/

		if (record_decl->isDynamicClass())
		{
			std::cout << "Line: " << line_info(record_decl->getBeginLoc());
			printf("\033[31mError: is dynamic class.\033[0m\n");
			return true;
		}

		interface_decl class_decl;
		class_decl.class_name = record_decl->getNameAsString();
		const std::string& full_name = record_decl->getQualifiedNameAsString();
		class_decl.package_name = full_name.substr(0, full_name.length() - class_decl.class_name.length() - 2);

		// Parsing interface methods
		for (clang::CXXMethodDecl* method_decl : record_decl->methods())
		{
			if (method_decl->isDefaulted() || method_decl->isLateTemplateParsed())
			{
				continue;
			}

			if (method_decl->isStatic())
			{
				std::cout << "Line: " << line_info(method_decl->getBeginLoc());
				printf("\033[31mError: The function is static.\033[0m\n");
				continue;
			}

			if (method_decl->isDefined())
			{
				std::cout << "Line: " << line_info(method_decl->getBeginLoc());
				printf("\033[33mWarning: The function is defined.\033[0m\n");
			}

			function_decl func_decl;

			clang::QualType ret_type = method_decl->getReturnType();

			func_decl.func_name = method_decl->getNameAsString();
			func_decl.return_type = ret_type.getAsString();
			func_decl.is_const_func = method_decl->isConst();
			func_decl.return_void = ret_type->isVoidType();

			for (clang::ParmVarDecl* parm_decl : method_decl->parameters())
			{
				param_decl param{};
				for (clang::Attr* attr : parm_decl->attrs())
				{
					if (attr->getAttrName()->getName().str() != "annotate")
						continue;

					clang::AnnotateAttr* annotate = (clang::AnnotateAttr*)(attr);
					if (!annotate)
						continue;

					const std::string& param_attr = annotate->getAnnotation().str();
					if (param_attr == "out")
					{
						param.attr = 1;
						break;
					}
					else if (param_attr == "inout")
					{
						param.attr = 2;
						break;
					}
				}

				param.type = parm_decl->getType().getAsString();
				param.name = parm_decl->getQualifiedNameAsString();

				func_decl.params.push_back(param);
			}

			class_decl.functions.push_back(func_decl);
		}

		// Parsing interface fields
		for (const clang::FieldDecl* field_decl : record_decl->fields())
		{
			if (field_decl->getType()->isVoidType())
			{
				printf("\033[33mWarning: Field %s is void.\033[0m\n", field_decl->getNameAsString().c_str());
				continue;
			}

			param_decl param{ 2 };
			for (clang::Attr* attr : field_decl->attrs())
			{
				if (attr->getAttrName()->getName().str() != "annotate")
					continue;

				clang::AnnotateAttr* annotate = (clang::AnnotateAttr*)(attr);
				if (!annotate)
					continue;

				const std::string& param_attr = annotate->getAnnotation().str();
				if (param_attr == "set")
				{
					param.attr = 0;
					break;
				}
				else if (param_attr == "get")
				{
					param.attr = 1;
					break;
				}
			}

			param.type = field_decl->getType().getAsString();
			param.name = field_decl->getNameAsString();

			class_decl.fields.push_back(param);
		}

		interface_list_.push_back(class_decl);

		return true;
	}
}
