#pragma once

#include <clang/Frontend/FrontendAction.h>
#include "analysis_result.hpp"

namespace glasssix::ymer
{
	class analysis_action : public clang::ASTFrontendAction
	{
	public:
		analysis_action(std::vector<interface_decl>& result);

		~analysis_action();

		std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& compiler, llvm::StringRef in_file) override;

	private:
		std::vector<interface_decl>& result_;
	};
}
