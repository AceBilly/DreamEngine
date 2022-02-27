module;
#include "helper.h"
#include <iostream>
#include <filesystem>
#include <DirectXMath.h>
#pragma comment(lib, "dxgi")
export module utility;
template<typename T>concept drawadble = requires {
    T::draw();
};
namespace fs = std::filesystem;
// 
// 储存顶点数据 
//
export struct Vertex0 {
    DirectX::XMFLOAT3 pos;  // 顶点位置信息
    DirectX::XMFLOAT4 col;  // 顶点颜色信息
};
// 格式化输出source_location
export void formatOutSourceLocation(const std::string_view& message, const std::source_location& location, std::ostream& = std::cout);
//  如果返回值不正确那 抛出std::runtime(***) 的异常；
//TODO: 异常处理尚未完善
export void throwIfFailed(HRESULT result,const std::string_view& message, const std::source_location location = std::source_location::current());
// 获得硬件适配器，若没有则使用 base rendering warp adapters
export void getHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1* ppAdapter);
// 获取资产文件全路径
// @param relativePath 资产文件的相对路径；
// @returns 资产的绝对路径；
export fs::path getAssetFullPath(const fs::path& relativePath);
module: private;

void getHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1* pAdapter) {
    for (UINT adaptersIndex = 0;;++adaptersIndex) {
        if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adaptersIndex, &pAdapter)) {
            break;
        }
        if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {
            return;
        }
    }
}

void formatOutSourceLocation(const std::string_view& message, const std::source_location& location, std::ostream& os) {
	os  <<  "file: "
        << location.file_name() << "("
        << location.line() << ":"
        << location.column() << ") `"
        << location.function_name() << "`: "
        << message << '\n';
}
void throwIfFailed(HRESULT result, const std::string_view& message, const std::source_location location) {
    if (result != S_OK) {
        //formatOutSourceLocation(message, location);
        throw std::runtime_error(message.data());
    }
}
// 在工作目录中搜索
bool isExist(const fs::path& directory, const fs::path& filename) {
    if (fs::is_directory(directory)) {
        return fs::exists(directory / filename);
    }
    return false;
}
fs::path getAssetFullPath(const fs::path& _path) {
    // TODO: 并发搜索？
    // 向下搜索直到 1层 TODO（无深度）
    // TODO:若向下搜索没有将向上搜索两层
    fs::path rootPath = fs::current_path(); // 程序运行目录 这这里开始查询
    if (isExist(rootPath, _path)) {
        return rootPath / _path;
    }
    for (auto& directory_entry : fs::directory_iterator(rootPath)) {
        if (directory_entry.is_directory()) {
            if (isExist(directory_entry, _path)) {
                return directory_entry / _path;
            }
        }
    }
    
    rootPath = "NoN";
    return rootPath;
}