#include "MSXMLRead.hpp"
#include <Shlwapi.h>
#include <comdef.h>
#include <type_traits>
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

namespace Replace {
	// code copy from make_array.h in repositry xml_text_cooking_quiz(Author:yumetodo)
	std::basic_string<TCHAR> LF(std::basic_string<TCHAR>&& Str) {
		if (_T("empty") == Str) return _T("");
		const std::basic_string<TCHAR> ReplaceTarget = _T("\\n");
		const std::basic_string<TCHAR> AfterReplace = _T("\n");
		for (size_t pos = Str.find(ReplaceTarget); std::string::npos != pos; pos = Str.find(ReplaceTarget, pos + AfterReplace.length())) {
			Str.replace(pos, ReplaceTarget.length(), AfterReplace);
		}
		return Str;
	}
}

std::basic_string<TCHAR> Node::operator [] (const long Count) const {
	IXMLDOMNode* lpItem;
	this->NodeList->get_item(Count, &lpItem);
	BSTR Strings;
	lpItem->get_text(&Strings);
	std::basic_string<TCHAR> Str = Replace::LF(
#if defined(UNICODE)
		Strings
#else
		Win32LetterConvert::WStringToString(Strings)
#endif
	);
	Str.resize(std::char_traits<TCHAR>::length(Str.c_str()));
	return Str;
}

MSXMLRead::MSXMLRead(const std::basic_string<TCHAR> FileName, const std::basic_string<TCHAR> CommonPath) {
	if (FALSE == PathFileExists(FileName.c_str())) throw std::runtime_error(
#if defined(UNICODE)
		Win32LetterConvert::WStringToString(FileName)
#else
		FileName
#endif
		+ " : file is not found.");
	CoInitialize(nullptr);
	IXMLDOMDocument* XmlDoc;
	const HRESULT ErrorCode = CoCreateInstance(CLSID_DOMDocument, nullptr, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&XmlDoc);
	if (XmlDoc == nullptr) throw std::runtime_error("xml open failed.\nCause : " + Win32Error::GetErrorMessage(ErrorCode));
	this->lpXmlDoc = XmlDoc;
	VARIANT_BOOL Result;
	this->lpXmlDoc->put_async(VARIANT_FALSE);
	this->lpXmlDoc->load(_variant_t(FileName.c_str()), &Result);
	if (0 == Result) {
		this->lpXmlDoc->Release();
		CoUninitialize();
		throw std::runtime_error("xml read failed");
	}
	this->CommonPath = CommonPath;
}

IXMLDOMNodeList* MSXMLRead::XmlSetNodeList(const std::basic_string<TCHAR> NodePath, long &Length) {
	BSTR Path = SysAllocString(( 
#if defined(UNICODE)
		this->CommonPath + NodePath
#else
		Win32LetterConvert::StringToWString(this->CommonPath + NodePath)
#endif
	).c_str());
	if (NodePath.empty() || 0 == SysStringLen(Path)) {
		SysFreeString(Path);
		return NULL;
	}
	IXMLDOMNodeList* lpNodeList;
	if (E_INVALIDARG == this->lpXmlDoc->selectNodes(Path, &lpNodeList)) {
		SysFreeString(Path);
		lpNodeList->Release();
		return NULL;
	}
	lpNodeList->get_length(&Length);
	SysFreeString(Path);
	return lpNodeList;
}

long MSXMLRead::CheckLength(const std::basic_string<TCHAR> NodePath) {
	long Length = 0;
	IXMLDOMNodeList* list = XmlSetNodeList(this->CommonPath + NodePath, Length);
	if (NULL == list) {
		list->Release();
		throw std::runtime_error("xml read failed");
	}
	list->Release();
	return Length;
}

void MSXMLRead::LoadFromFile(const std::basic_string<TCHAR> NodePath) {
	Node node;
	node.NodePath = this->CommonPath + NodePath;
	node.NodePath.resize(std::char_traits<TCHAR>::length(node.NodePath.c_str()));
	node.NodeList = this->XmlSetNodeList(NodePath, node.Length);
	this->Data.emplace_back(node);
}

void MSXMLRead::ChangeCommonPath(const std::basic_string<TCHAR> NewRoot) {
	this->clear();
	this->CommonPath = NewRoot;
}

Node MSXMLRead::operator [] (const std::basic_string<TCHAR> NodePath) const {
	for (const auto& i : this->Data) if (i.NodePath == this->CommonPath + NodePath) return i;
	throw std::runtime_error(
#if defined(UNICODE)
			Win32LetterConvert::WStringToString(this->CommonPath + NodePath)
#else
			this->CommonPath + NodePath
#endif
			+ " : not found such node.");
}

