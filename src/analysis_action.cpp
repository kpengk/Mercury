#include "analysis_action.h"

#include <clang/Frontend/CompilerInstance.h>

#include "analysis_consumer.h"



namespace glasssix::ymer
{
	analysis_action::analysis_action(std::vector<interface_decl>& result)
		: result_(result)
	{
	}

	analysis_action::~analysis_action()
	{
	}

	std::unique_ptr<clang::ASTConsumer> analysis_action::CreateASTConsumer(clang::CompilerInstance& compiler, llvm::StringRef in_file)
	{
		return std::make_unique<analysis_consumer>(&compiler.getASTContext(), result_);
	}
}
