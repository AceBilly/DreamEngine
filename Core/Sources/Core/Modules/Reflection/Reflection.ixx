module;

#include "clang/Frontend/FrontendActions.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/AttrKinds.h"

export module Reflection;


export namespace Dream {
	export namespace Reflection {
		export int getClassTypeDumpInfo(const int argc, const char* const argv[]);

		clang::ast_matchers::DeclarationMatcher reflectionAttrMatcher = 
			clang::ast_matchers::decl(clang::ast_matchers::hasAttr(clang::attr::Annotate)).bind("reflection_class_ID");
		class GenerateTypeInfo 
			: public clang::ast_matchers::MatchFinder::MatchCallback {
			virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result) final;
		};
	} // namespace Reflection
}  // namespace Dream

module : private;

int Dream::Reflection::getClassTypeDumpInfo(const int argc, const char* const argv[]) {

}

void Dream::Reflection::GenerateTypeInfo::run(const clang::ast_matchers::MatchFinder::MatchResult& result) {
	if (const clang::CXXRecordDecl *reflectionStmt = result.Nodes.getNodeAs<clang::CXXRecordDecl>(llvm::StringRef("reflection_class_ID"))) {
		reflectionStmt->dumpColor();
	}
}