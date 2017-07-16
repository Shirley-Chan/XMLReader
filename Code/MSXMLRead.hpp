#ifndef __MSXMLREAD_HPP__
#define __MSXMLREAD_HPP__
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <msxml.h>

class Node {
private:
public:
	Node() = default;
	std::basic_string<TCHAR> NodeFullPath;
	IXMLDOMNodeList* NodeList;
	long Length;
	std::basic_string<TCHAR> operator [] (const long Count) const; // ‚¿‚å‚Á‚Æ’·‚¢‚©‚çcpp‘¤‚É’è‹`
};

class MSXMLRead {
private:
	IXMLDOMDocument* lpXmlDoc;
	IXMLDOMNodeList* XmlSetNodeList(const TCHAR* NodePath, long &Length);
	std::vector<Node> Data;
	void LoadFromFile(const TCHAR* NodeFullPath);
public:
	MSXMLRead(const TCHAR* FileName);
	long CheckLength(const TCHAR* NodeFullPath);
	void Check(const TCHAR* NodeFullPath) { this->CheckLength(NodeFullPath); }
	template<typename ...Args> void Check(const TCHAR* NodeFullPath, Args ...arg) {
		this->CheckLength(NodeFullPath);
		this->Check(std::forward<Args>(arg)...);
	}
	void Load(const TCHAR* NodeFullPath) { this->LoadFromFile(NodeFullPath); }
	template<typename ...Args> void Load(const TCHAR* NodeFullPath, Args ...arg) {
		this->LoadFromFile(NodeFullPath);
		this->Load(std::forward<Args>(arg)...);
	}
	Node operator [] (const size_t Count) const { return this->Data[Count]; }
	Node operator | (const std::basic_string<TCHAR> NodeFullPath) const; // WStringToString‚ðŽg‚¤ŠÖŒW‚Åcpp‘¤‚É’è‹`
};
#endif
