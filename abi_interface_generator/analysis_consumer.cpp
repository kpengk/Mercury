#include "analysis_consumer.h"
#include "analysis_visitor.h"


namespace glasssix::ymer
{
	analysis_consumer::analysis_consumer(clang::ASTContext* context, std::vector<interface_decl>& result)
		: visitor_(new analysis_visitor(context, result))
	{}

	analysis_consumer::~analysis_consumer()
	{
		delete visitor_;
	}

	void analysis_consumer::HandleTranslationUnit(clang::ASTContext & context)
	{
		visitor_->TraverseDecl(context.getTranslationUnitDecl());
	}
}