#include "MSXMLRead.hpp"
#include <Shlwapi.h>
#include <comdef.h>
#pragma comment(lib, "Shlwapi.lib")

namespace Win32Error {
	// code copy from in FunctionType/Code/ExceptionManager.cpp in repositry "WindowsServiceCppLibrary"(Author:AinoMegumi)
	std::string GetErrorMessage(const unsigned long ErrorCode) {
		char* lpMessageBuffer = nullptr;
		const DWORD length = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, ErrorCode, LANG_USER_DEFAULT, (LPSTR)&lpMessageBuffer, 0, NULL);
		if (length == 0) throw std::runtime_error("エラーメッセージ取得時にエラーが発生しました。\nエラーコード : " + std::to_string(GetLastError()));
		DWORD i = length - 3;
		for (; '\r' != lpMessageBuffer[i] && '\n' != lpMessageBuffer[i] && '\0' != lpMessageBuffer[i]; i++);//改行文字削除
		lpMessageBuffer[i] = '\0';
		std::string s = "エラーコード : " + std::to_string(ErrorCode) + "　" + lpMessageBuffer;
		LocalFree(lpMessageBuffer);
		return s;
	}

	std::string GetErrorMessage() {
		const DWORD ErrorCode = GetLastError();
		return GetErrorMessage(ErrorCode);
	}
}

namespace Win32LetterConvert {
	// code copy from Storage.cpp in repositry "DefeatMonster.old"(Author:AinoMegumi)
	std::string WStringToString(const std::wstring oWString) {
		const int iBufferSize = WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, (char *)NULL, 0, NULL, NULL);
		if (0 == iBufferSize) throw std::runtime_error(Win32Error::GetErrorMessage());
		char* cpMultiByte = new char[iBufferSize];
		WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, cpMultiByte, iBufferSize, NULL, NULL);
		std::string oRet(cpMultiByte, cpMultiByte + iBufferSize - 1);
		delete[] cpMultiByte;
		return oRet;
	}

	std::wstring StringToWString(const std::string oString) {
		const int iBufferSize = MultiByteToWideChar(CP_OEMCP, 0, oString.c_str(), -1, (wchar_t*)NULL, 0);
		if (0 == iBufferSize) throw std::runtime_error(Win32Error::GetErrorMessage());
		wchar_t* cpWideChar = new wchar_t[iBufferSize];
		MultiByteToWideChar(CP_OEMCP, 0, oString.c_str(), -1, cpWideChar, iBufferSize);
		std::wstring oRet(cpWideChar, cpWideChar + iBufferSize - 1);
		delete[] cpWideChar;
		return oRet;
	}
}

std::basic_string<TCHAR> Node::operator [] (const long Count) const {
	IXMLDOMNode* lpItem;
	this->NodeList->get_item(Count, &lpItem);
	BSTR Strings;
	lpItem->get_text(&Strings);
	std::basic_string<TCHAR> Str =
#if defined(UNICODE)
	Strings;
#else
	Win32LetterConvert::WStringToString(Strings);
#endif
	Str.resize(std::char_traits<TCHAR>::length(Str.c_str()));
	delete[] Strings;
	return Str;
}

MSXMLRead::MSXMLRead(const TCHAR* FileName) {
	CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDocument, (PVOID*)&this->lpXmlDoc);
	VARIANT_BOOL Result;
	this->lpXmlDoc->put_async(VARIANT_FALSE);
	this->lpXmlDoc->load(_variant_t(FileName), &Result);
	if (0 == Result) {
		this->lpXmlDoc->Release();
		CoUninitialize();
		throw std::runtime_error("xml read failed");
	}
}

IXMLDOMNodeList* MSXMLRead::XmlSetNodeList(const TCHAR* NodeFullPath, long &Length) {
	BSTR NodePath = SysAllocString(
#if defined(UNICODE)
		NodeFullPath
#else
		Win32LetterConvert::StringToWString(NodeFullPath).c_str()
#endif
	);
	if (NULL == NodePath || 0 == SysStringLen(NodePath)) {
		SysFreeString(NodePath);
		return NULL;
	}
	IXMLDOMNodeList* lpNodeList;
	if (E_INVALIDARG == this->lpXmlDoc->selectNodes(NodePath, &lpNodeList)) {
		SysFreeString(NodePath);
		lpNodeList->Release();
		return NULL;
	}
	lpNodeList->get_length(&Length);
	SysFreeString(NodePath);
	return lpNodeList;
}

long MSXMLRead::CheckLength(const TCHAR* NodeFullPath) {
	long Length = 0;
	IXMLDOMNodeList* list = XmlSetNodeList(NodeFullPath, Length);
	if (NULL == list) {
		list->Release();
		throw std::runtime_error("xml read failed");
	}
	return Length;
}

void MSXMLRead::LoadFromFile(const TCHAR* NodeFullPath) {
	Node node;
	node.NodeFullPath = NodeFullPath;
	node.NodeFullPath.resize(std::char_traits<TCHAR>::length(node.NodeFullPath.c_str()));
	node.NodeList = this->XmlSetNodeList(NodeFullPath, node.Length);
	this->Data.emplace_back(node);
}

Node MSXMLRead::operator | (const std::basic_string<TCHAR> NodeFullPath) const {
	for (const auto& i : this->Data) if (i.NodeFullPath == NodeFullPath) return i;
	throw std::runtime_error(
#if defined(UNICODE)
			Win32LetterConvert::WStringToString(NodeFullPath)
#else
			NodeFullPath
#endif
			+ " : not found such node.");
}

