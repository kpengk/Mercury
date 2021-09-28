#pragma once

#include <clang/AST/RecursiveASTVisitor.h>
#include "analysis_result.hpp"

namespace glasssix::ymer
{
	class clang::ASTContext;
	class interface_decl;

	class analysis_visitor : public clang::RecursiveASTVisitor<analysis_visitor>
	{
	public:
		explicit analysis_visitor(clang::ASTContext* context, std::vector<interface_decl>& result);

		~analysis_visitor();

		bool VisitCXXRecordDecl(clang::CXXRecordDecl* declaration);

	private:
		std::string line_info(clang::SourceLocation location);

	private:
		clang::ASTContext* context_;
		std::vector<interface_decl>& interface_list_;
	};

}
