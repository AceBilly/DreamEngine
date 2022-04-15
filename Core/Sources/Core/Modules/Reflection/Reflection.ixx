module;

#include <iostream>
#include <string>

#include "clang/Frontend/FrontendActions.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/AttrKinds.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Tooling/Tooling.h"

export module Core.Reflection;


export namespace Dream {
	export namespace Reflection {
		export int getClassTypeDumpInfo(int &argc, const char** argv);
		clang::ast_matchers::DeclarationMatcher reflectionAttrMatcher = 
			clang::ast_matchers::cxxRecordDecl(clang::ast_matchers::hasAttr(clang::attr::Annotate)).bind("reflection_class_ID");
		llvm::cl::OptionCategory MyToolCategory("global-detect options");

		class GenerateTypeInfo 
			: public clang::ast_matchers::MatchFinder::MatchCallback {
			virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result) final;
		};
	} // namespace Reflection
}  // namespace Dream

module : private;

int Dream::Reflection::getClassTypeDumpInfo(int &argc, const char** argv) {
	auto optionsParser = clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory);
	if (auto err = optionsParser.takeError()) {
		// TODO: error handing
		std::cerr << "Reflection: generate type info failed, " << llvm::toString(std::move(err));
		return -1;
	}
	clang::tooling::ClangTool tool((*optionsParser).getCompilations(),
								   (*optionsParser).getSourcePathList());

	clang::ast_matchers::MatchFinder matchFinder;
	GenerateTypeInfo matchCallback;
	matchFinder.addMatcher(reflectionAttrMatcher, &matchCallback);
	return 0;
}

void Dream::Reflection::GenerateTypeInfo::run(const clang::ast_matchers::MatchFinder::MatchResult& result) {
	if (const clang::CXXRecordDecl *reflectionStmt 
		= result.Nodes.getNodeAs<clang::CXXRecordDecl>(llvm::StringRef("reflection_class_ID"))) {
		reflectionStmt->dumpColor();
	}
}