//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef OTML_H
#define OTML_H

#include <format>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <exception>
#include <memory>
#include <algorithm>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <wx/string.h>
#include <wx/filename.h>

class OTMLNode;
class OTMLDocument;
class OTMLParser;
class OTMLEmitter;

using OTMLNodePtr = std::shared_ptr<OTMLNode>;
using OTMLNodeEnableSharedFromThis = std::enable_shared_from_this<OTMLNode>;
using OTMLDocumentPtr = std::shared_ptr<OTMLDocument>;
using OTMLNodeWeakPtr = std::weak_ptr<OTMLNode>;

using OTMLNodeList = std::vector<OTMLNodePtr>;

namespace otml_util {
	template <typename T, typename R>
	bool cast(const T& in, R& out) {
		std::stringstream ss;
		ss << in;
		ss >> out;
		return !!ss && ss.eof();
	}

	template <typename T>
	bool cast(const T& in, std::string& out) {
		std::stringstream ss;
		ss << in;
		out = ss.str();
		return true;
	}

	template <>
	inline bool cast(const std::string& in, std::string& out) {
		out = in;
		return true;
	}

	template <>
	inline bool cast(const std::string& in, bool& b) {
		if (in == "true") {
			b = true;
		} else if (in == "false") {
			b = false;
		} else {
			return false;
		}
		return true;
	}

	template <>
	inline bool cast(const std::string& in, char& c) {
		if (in.length() != 1) {
			return false;
		}
		c = in[0];
		return true;
	}

	template <>
	inline bool cast(const std::string& in, long& l) {
		if (in.find_first_not_of("-0123456789") != std::string::npos) {
			return false;
		}
		std::size_t t = in.find_last_of('-');
		if (t != std::string::npos && t != 0) {
			return false;
		}
		l = atol(in.c_str());
		return true;
	}

	template <>
	inline bool cast(const std::string& in, int& i) {
		long l;
		if (cast(in, l)) {
			i = l;
			return true;
		}
		return false;
	}

	template <>
	inline bool cast(const std::string& in, double& d) {
		if (in.find_first_not_of("-0123456789.") != std::string::npos) {
			return false;
		}
		std::size_t t = in.find_last_of('-');
		if (t != std::string::npos && t != 0) {
			return false;
		}
		t = in.find_first_of('.');
		if (t != std::string::npos && (t == 0 || t == in.length() - 1 || in.find_first_of('.', t + 1) != std::string::npos)) {
			return false;
		}
		d = atof(in.c_str());
		return true;
	}

	template <>
	inline bool cast(const bool& in, std::string& out) {
		out = (in ? "true" : "false");
		return true;
	}

	class BadCast : public std::bad_cast {
	public:
		virtual ~BadCast() noexcept = default;
		virtual const char* what() const noexcept override {
			return "failed to cast value";
		}
	};

	template <typename R, typename T>
	R safeCast(const T& t) {
		R r;
		if (!cast(t, r)) {
			throw BadCast();
		}
		return r;
	}
};

class OTMLException : public std::exception {
public:
	OTMLException(const std::string& error) :
		m_what(error) { }
	OTMLException(const OTMLNodePtr& node, const std::string& error);
	OTMLException(const OTMLDocumentPtr& doc, const std::string& error, int line = -1);
	virtual ~OTMLException() noexcept = default;

	virtual const char* what() const noexcept override {
		return m_what.c_str();
	}

protected:
	std::string m_what;
};

class OTMLNode : public OTMLNodeEnableSharedFromThis {
public:
	OTMLNode() :
		m_unique(false), m_null(false) { }
	virtual ~OTMLNode() = default;

	static OTMLNodePtr create(std::string tag = "", bool unique = false);
	static OTMLNodePtr create(std::string tag, std::string value);

	std::string tag() const {
		return m_tag;
	}
	int size() const {
		return m_children.size();
	}
	OTMLNodePtr parent() const {
		return m_parent.lock();
	}
	std::string source() const {
		return m_source;
	}
	std::string rawValue() const {
		return m_value;
	}

	bool isUnique() const {
		return m_unique;
	}
	bool isNull() const {
		return m_null;
	}

	bool hasTag() const {
		return !m_tag.empty();
	}
	bool hasValue() const {
		return !m_value.empty();
	}
	bool hasChildren() const;
	bool hasChildAt(const std::string& childTag) {
		return !!get(childTag);
	}
	bool hasChildAtIndex(int childIndex) {
		return !!getIndex(childIndex);
	}

	void setTag(std::string tag) {
		m_tag = tag;
	}
	void setValue(const std::string& value) {
		m_value = value;
	}
	void setNull(bool null) {
		m_null = null;
	}
	void setUnique(bool unique) {
		m_unique = unique;
	}
	void setParent(const OTMLNodePtr& parent) {
		m_parent = parent;
	}
	void setSource(const std::string& source) {
		m_source = source;
	}

	OTMLNodePtr get(const std::string& childTag) const;
	OTMLNodePtr getIndex(int childIndex) const;

	OTMLNodePtr at(const std::string& childTag);
	OTMLNodePtr atIndex(int childIndex);

	void addChild(const OTMLNodePtr& newChild);
	bool removeChild(const OTMLNodePtr& oldChild);
	bool replaceChild(const OTMLNodePtr& oldChild, const OTMLNodePtr& newChild);
	void merge(const OTMLNodePtr& node);
	void copy(const OTMLNodePtr& node);
	void clear();

	OTMLNodeList children() const;
	OTMLNodePtr clone() const;

	template <typename T>
	T value();
	template <typename T>
	T valueAt(const std::string& childTag);
	template <typename T>
	T valueAtIndex(int childIndex);
	template <typename T>
	T valueAt(const std::string& childTag, const T& def);
	template <typename T>
	T valueAtIndex(int childIndex, const T& def);

	template <typename T>
	void write(const T& v);
	template <typename T>
	void writeAt(const std::string& childTag, const T& v);
	template <typename T>
	void writeIn(const T& v);

	virtual std::string emit();

protected:
	OTMLNodeList m_children;
	OTMLNodeWeakPtr m_parent;
	std::string m_tag;
	std::string m_value;
	std::string m_source;
	bool m_unique;
	bool m_null;
};

class OTMLDocument : public OTMLNode {
public:
	OTMLDocument() { }
	~OTMLDocument() override = default;
	static OTMLDocumentPtr create();
	static OTMLDocumentPtr parse(const wxString& fileName);
	static OTMLDocumentPtr parse(std::istream& in, const wxString& source);
	std::string emit() override;
	bool save(const wxString& fileName);
};

class OTMLParser {
public:
	OTMLParser(OTMLDocumentPtr doc, std::istream& in) :
		currentDepth(0), currentLine(0),
		doc(doc), currentParent(doc),
		in(in) { }
	void parse();

private:
	std::string getNextLine();
	int getLineDepth(const std::string& line, bool multilining = false);
	void parseLine(std::string line);
	void parseNode(const std::string& data);

	int currentDepth;
	int currentLine;
	OTMLDocumentPtr doc;
	OTMLNodePtr currentParent;
	OTMLNodePtr previousNode;
	std::istream& in;
};

class OTMLEmitter {
public:
	static std::string emitNode(const OTMLNodePtr& node, int currentDepth = -1);
};

template <>
inline std::string OTMLNode::value() {
	std::string value = m_value;
	if (boost::starts_with(value, "\"") && boost::ends_with(value, "\"")) {
		value = value.substr(1, value.length() - 2);
		boost::replace_all(value, "\\\\", "\\");
		boost::replace_all(value, "\\\"", "\"");
		boost::replace_all(value, "\\t", "\t");
		boost::replace_all(value, "\\n", "\n");
		boost::replace_all(value, "\\'", "\'");
	}
	return value;
}

template <typename T>
T OTMLNode::value() {
	T ret;
	if (!otml_util::cast(m_value, ret)) {
		throw OTMLException(shared_from_this(), "failed to cast node value");
	}
	return ret;
}

template <typename T>
T OTMLNode::valueAt(const std::string& childTag) {
	OTMLNodePtr node = at(childTag);
	return node->value<T>();
}

template <typename T>
T OTMLNode::valueAtIndex(int childIndex) {
	OTMLNodePtr node = atIndex(childIndex);
	return node->value<T>();
}

template <typename T>
T OTMLNode::valueAt(const std::string& childTag, const T& def) {
	if (OTMLNodePtr node = get(childTag)) {
		if (!node->isNull()) {
			return node->value<T>();
		}
	}
	return def;
}

template <typename T>
T OTMLNode::valueAtIndex(int childIndex, const T& def) {
	if (OTMLNodePtr node = getIndex(childIndex)) {
		return node->value<T>();
	}
	return def;
}

template <typename T>
void OTMLNode::write(const T& v) {
	m_value = otml_util::safeCast<std::string>(v);
}

template <typename T>
void OTMLNode::writeAt(const std::string& childTag, const T& v) {
	OTMLNodePtr child = OTMLNode::create(childTag);
	child->setUnique(true);
	child->write<T>(v);
	addChild(child);
}

template <typename T>
void OTMLNode::writeIn(const T& v) {
	OTMLNodePtr child = OTMLNode::create();
	child->write<T>(v);
	addChild(child);
}

#endif
