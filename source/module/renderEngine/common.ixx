module;
#include <fstream>
#include <vector>
#include <filesystem>
export module common;
namespace fs = std::filesystem;
// ¶ÁÈ¡shaderÎÄ¼þ
export
std::vector<char> readShaderFile(const fs::path& shaderFile);


module : private;
std::vector<char> readShaderFile(const fs::path& shaderFile) {
	std::ifstream file(shaderFile, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("failed to open shader File: " + shaderFile.string());
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}