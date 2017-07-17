#ifndef __MSXMLREAD_HPP__
#define __MSXMLREAD_HPP__
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <msxml.h>
#include <tchar.h>

class Node {
private:
public:
	Node() = default;
	std::basic_string<TCHAR> NodePath;
	IXMLDOMNodeList* NodeList;
	long Length;
	std::basic_string<TCHAR> operator [] (const long Count) const; // ‚¿‚å‚Á‚Æ’·‚¢‚©‚çcpp‘¤‚É’è‹`
};

class MSXMLRead {
private:
	IXMLDOMDocument* lpXmlDoc;
	IXMLDOMNodeList* XmlSetNodeList(const std::basic_string<TCHAR> NodePath, long &Length);
	std::vector<Node> Data;
	std::basic_string<TCHAR> CommonPath;
	void LoadFromFile(const std::basic_string<TCHAR> NodePath);
public:
	MSXMLRead(const std::basic_string<TCHAR> FileName, const std::basic_string<TCHAR> CommonPath = TEXT(""));
	~MSXMLRead() { 
		this->lpXmlDoc->Release();
		this->Data.clear();
	}
	long CheckLength(const std::basic_string<TCHAR> NodePath);
	void Check(const std::basic_string<TCHAR> NodePath) { this->CheckLength(NodePath); }
	template<class ...Args> void Check(const std::basic_string<TCHAR> NodePath, Args ...arg) {
		this->CheckLength(NodePath);
		this->Check(std::forward<Args>(arg)...);
	}
	void Load(const std::basic_string<TCHAR> NodePath) { this->LoadFromFile(NodePath); }
	template<class ...Args> void Load(const std::basic_string<TCHAR> NodePath, Args ...arg) {
		this->LoadFromFile(NodePath);
		this->Load(std::forward<Args>(arg)...);
	}
	size_t size() const { return this->Data.size(); }
	Node at(const size_t Count) const { return this->Data[Count]; }
	void clear() { this->Data.clear(); }
	void ChangeCommonPath(const std::basic_string<TCHAR> NewRoot);
	Node operator [] (const size_t Count) const { return this->at(Count); }
	Node operator [] (const std::basic_string<TCHAR> NodePath) const; // WStringToString‚ðŽg‚¤ŠÖŒW‚Åcpp‘¤‚É’è‹`
};
#endif
