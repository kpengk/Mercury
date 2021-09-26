#pragma once

#include <clang/AST/ASTConsumer.h>
#include "analysis_result.hpp"

namespace glasssix::ymer
{
	class analysis_visitor;

	class analysis_consumer : public clang::ASTConsumer
	{
	public:
		explicit analysis_consumer(clang::ASTContext* context, std::vector<interface_decl>& result);

		~analysis_consumer();

		void HandleTranslationUnit(clang::ASTContext& context) override;

	private:
		analysis_visitor* visitor_;
	};

}
