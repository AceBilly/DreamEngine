module;

#include <clang/Frontend/FrontendActions.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

export module Reflection;

using namespace clang;
using namespace clang::ast_matchers;

export namespace Dream {
	export namespace Reflection {

		export class GenerateTypeInfo : public MatchFinder::MatchCallback {

		};
	} // namespace Reflection
}  // namespace Dream