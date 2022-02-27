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
// ���涥������ 
//
export struct Vertex0 {
    DirectX::XMFLOAT3 pos;  // ����λ����Ϣ
    DirectX::XMFLOAT4 col;  // ������ɫ��Ϣ
};
// ��ʽ�����source_location
export void formatOutSourceLocation(const std::string_view& message, const std::source_location& location, std::ostream& = std::cout);
//  �������ֵ����ȷ�� �׳�std::runtime(***) ���쳣��
//TODO: �쳣������δ����
export void throwIfFailed(HRESULT result,const std::string_view& message, const std::source_location location = std::source_location::current());
// ���Ӳ������������û����ʹ�� base rendering warp adapters
export void getHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1* ppAdapter);
// ��ȡ�ʲ��ļ�ȫ·��
// @param relativePath �ʲ��ļ������·����
// @returns �ʲ��ľ���·����
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
// �ڹ���Ŀ¼������
bool isExist(const fs::path& directory, const fs::path& filename) {
    if (fs::is_directory(directory)) {
        return fs::exists(directory / filename);
    }
    return false;
}
fs::path getAssetFullPath(const fs::path& _path) {
    // TODO: ����������
    // ��������ֱ�� 1�� TODO������ȣ�
    // TODO:����������û�н�������������
    fs::path rootPath = fs::current_path(); // ��������Ŀ¼ �����￪ʼ��ѯ
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