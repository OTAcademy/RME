//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "map/otml.h"
#include <format>

OTMLException::OTMLException(const OTMLNodePtr& node, const std::string& error) {
	if (node && !node->source().empty()) {
		m_what = std::format("OTML error in '{}': {}", node->source(), error);
	} else {
		m_what = std::format("OTML error: {}", error);
	}
}

OTMLException::OTMLException(const OTMLDocumentPtr& doc, const std::string& error, int line) {
	if (doc && !doc->source().empty()) {
		if (line >= 0) {
			m_what = std::format("OTML error in '{}' at line {}: {}", doc->source(), line, error);
		} else {
			m_what = std::format("OTML error in '{}': {}", doc->source(), error);
		}
	} else {
		m_what = std::format("OTML error: {}", error);
	}
}

OTMLNodePtr OTMLNode::create(std::string tag, bool unique) {
	OTMLNodePtr node = std::make_shared<OTMLNode>();
	node->setTag(tag);
	node->setUnique(unique);
	return node;
}

OTMLNodePtr OTMLNode::create(std::string tag, std::string value) {
	OTMLNodePtr node = std::make_shared<OTMLNode>();
	node->setTag(tag);
	node->setValue(value);
	node->setUnique(true);
	return node;
}

bool OTMLNode::hasChildren() const {
	for (const auto& child : m_children) {
		if (!child->isNull()) {
			return true;
		}
	}
	return false;
}

OTMLNodePtr OTMLNode::get(const std::string& childTag) const {
	for (const auto& child : m_children) {
		if (child->tag() == childTag && !child->isNull()) {
			return child;
		}
	}
	return OTMLNodePtr();
}

OTMLNodePtr OTMLNode::getIndex(int childIndex) const {
	if (childIndex < size() && childIndex >= 0) {
		return m_children[childIndex];
	}
	return OTMLNodePtr();
}

OTMLNodePtr OTMLNode::at(const std::string& childTag) {
	for (const auto& child : m_children) {
		if (child->tag() == childTag && !child->isNull()) {
			return child;
		}
	}

	throw OTMLException(shared_from_this(), std::format("child node with tag '{}' not found", childTag));
}

OTMLNodePtr OTMLNode::atIndex(int childIndex) {
	if (childIndex >= size() || childIndex < 0) {
		throw OTMLException(shared_from_this(), std::format("child node with index '{}' not found", childIndex));
	}
	return m_children[childIndex];
}

void OTMLNode::addChild(const OTMLNodePtr& newChild) {
	if (newChild->hasTag()) {
		for (const auto& node : m_children) {
			if (node->tag() == newChild->tag() && (node->isUnique() || newChild->isUnique())) {
				newChild->setUnique(true);

				if (node->hasChildren() && newChild->hasChildren()) {
					OTMLNodePtr tmpNode = node->clone();
					tmpNode->merge(newChild);
					newChild->copy(tmpNode);
				}

				replaceChild(node, newChild);
				for (auto it = m_children.begin(); it != m_children.end();) {
					OTMLNodePtr child = (*it);
					if (child != newChild && child->tag() == newChild->tag()) {
						child->setParent(OTMLNodePtr());
						it = m_children.erase(it);
					} else {
						++it;
					}
				}
				return;
			}
		}
	}
	m_children.push_back(newChild);
	newChild->setParent(shared_from_this());
}

bool OTMLNode::removeChild(const OTMLNodePtr& oldChild) {
	OTMLNodeList::iterator it = std::find(m_children.begin(), m_children.end(), oldChild);
	if (it != m_children.end()) {
		m_children.erase(it);
		oldChild->setParent(OTMLNodePtr());
		return true;
	}
	return false;
}

bool OTMLNode::replaceChild(const OTMLNodePtr& oldChild, const OTMLNodePtr& newChild) {
	OTMLNodeList::iterator it = std::find(m_children.begin(), m_children.end(), oldChild);
	if (it != m_children.end()) {
		oldChild->setParent(OTMLNodePtr());
		newChild->setParent(shared_from_this());
		it = m_children.erase(it);
		m_children.insert(it, newChild);
		return true;
	}
	return false;
}

void OTMLNode::copy(const OTMLNodePtr& node) {
	setTag(node->tag());
	setValue(node->rawValue());
	setUnique(node->isUnique());
	setNull(node->isNull());
	setSource(node->source());
	clear();
	for (const auto& child : node->m_children) {
		addChild(child->clone());
	}
}

void OTMLNode::merge(const OTMLNodePtr& node) {
	for (const auto& child : node->m_children) {
		addChild(child->clone());
	}
	setTag(node->tag());
	setSource(node->source());
}

void OTMLNode::clear() {
	for (const auto& child : m_children) {
		child->setParent(OTMLNodePtr());
	}
	m_children.clear();
}

OTMLNodeList OTMLNode::children() const {
	OTMLNodeList children;
	for (const auto& child : m_children) {
		if (!child->isNull()) {
			children.push_back(child);
		}
	}
	return children;
}

OTMLNodePtr OTMLNode::clone() const {
	OTMLNodePtr myClone = std::make_shared<OTMLNode>();
	myClone->setTag(m_tag);
	myClone->setValue(m_value);
	myClone->setUnique(m_unique);
	myClone->setNull(m_null);
	myClone->setSource(m_source);
	for (const auto& child : m_children) {
		myClone->addChild(child->clone());
	}
	return myClone;
}

std::string OTMLNode::emit() {
	return OTMLEmitter::emitNode(shared_from_this(), 0);
}

OTMLDocumentPtr OTMLDocument::create() {
	OTMLDocumentPtr doc = std::make_shared<OTMLDocument>();
	doc->setTag("doc");
	return doc;
}

OTMLDocumentPtr OTMLDocument::parse(const std::string& fileName) {
	std::ifstream fin(fileName.c_str());
	if (!fin.good()) {
		throw OTMLException(std::format("failed to open file {}", fileName));
	}
	return parse(fin, fileName);
}

OTMLDocumentPtr OTMLDocument::parse(std::istream& in, const std::string& source) {
	OTMLDocumentPtr doc = std::make_shared<OTMLDocument>();
	doc->setSource(source);
	OTMLParser parser(doc, in);
	parser.parse();
	return doc;
}

std::string OTMLDocument::emit() {
	return OTMLEmitter::emitNode(shared_from_this()) + "\n";
}

bool OTMLDocument::save(const std::string& fileName) {
	m_source = fileName;
	std::ofstream fout(fileName.c_str());
	if (fout.good()) {
		fout << emit();
		fout.close();
		return true;
	}
	return false;
}

std::string OTMLEmitter::emitNode(const OTMLNodePtr& node, int currentDepth) {
	std::stringstream ss;
	if (currentDepth >= 0) {
		for (int i = 0; i < currentDepth; ++i) {
			ss << "  ";
		}
		if (node->hasTag()) {
			ss << node->tag();
			if (node->hasValue() || node->isUnique() || node->isNull()) {
				ss << ":";
			}
		} else {
			ss << "-";
		}
		if (node->isNull()) {
			ss << " ~";
		} else if (node->hasValue()) {
			ss << " ";
			std::string value = node->rawValue();
			if (value.find("\n") != std::string::npos) {
				if (value[value.length() - 1] == '\n' && value[value.length() - 2] == '\n') {
					ss << "|+";
				} else if (value[value.length() - 1] == '\n') {
					ss << "|";
				} else {
					ss << "|-";
				}
				for (std::size_t pos = 0; pos < value.length(); ++pos) {
					ss << "\n";
					for (int i = 0; i < currentDepth + 1; ++i) {
						ss << "  ";
					}
					while (pos < value.length()) {
						if (value[pos] == '\n') {
							break;
						}
						ss << value[pos++];
					}
				}
			} else {
				ss << value;
			}
		}
	}
	int i = 0;
	for (const auto& child : node->children()) {
		if (currentDepth >= 0 || i != 0) {
			ss << "\n";
		}
		ss << emitNode(child, currentDepth + 1);
		++i;
	}
	return ss.str();
}

void OTMLParser::parse() {
	if (!in.good()) {
		throw OTMLException(doc, "cannot read from input stream");
	}
	while (!in.eof()) {
		parseLine(getNextLine());
	}
}

std::string OTMLParser::getNextLine() {
	currentLine++;
	std::string line;
	std::getline(in, line);
	return line;
}

int OTMLParser::getLineDepth(const std::string& line, bool multilining) {
	std::size_t spaces = 0;
	while (spaces < line.size() && line[spaces] == ' ') {
		spaces++;
	}

	int depth = spaces / 2;
	if (!multilining || depth <= currentDepth) {
		if (line[spaces] == '\t') {
			throw OTMLException(doc, "indentation with tabs are not allowed", currentLine);
		}
		if (spaces % 2 != 0) {
			throw OTMLException(doc, "must indent every 2 spaces", currentLine);
		}
	}
	return depth;
}

void OTMLParser::parseLine(std::string line) {
	int depth = getLineDepth(line);
	if (depth == -1) {
		return;
	}
	boost::trim(line);
	if (line.empty()) {
		return;
	}
	if (line.substr(0, 2) == "//") {
		return;
	}
	if (depth == currentDepth + 1) {
		currentParent = previousNode;
	} else if (depth < currentDepth) {
		for (int i = 0; i < currentDepth - depth; ++i) {
			currentParent = currentParent->parent();
		}
	} else if (depth != currentDepth) {
		throw OTMLException(doc, "invalid indentation depth, are you indenting correctly?", currentLine);
	}
	currentDepth = depth;
	parseNode(line);
}

void OTMLParser::parseNode(const std::string& data) {
	std::string tag;
	std::string value;
	std::size_t dotsPos = data.find_first_of(':');
	int nodeLine = currentLine;
	if (!data.empty() && data[0] == '-') {
		value = data.substr(1);
		boost::trim(value);
	} else if (dotsPos != std::string::npos) {
		tag = data.substr(0, dotsPos);
		if (data.size() > dotsPos + 1) {
			value = data.substr(dotsPos + 1);
		}
	} else {
		tag = data;
	}
	boost::trim(tag);
	boost::trim(value);
	if (value == "|" || value == "|-" || value == "|+") {
		std::string multiLineData;
		do {
			size_t lastPos = in.tellg();
			std::string line = getNextLine();
			int depth = getLineDepth(line, true);
			if (depth > currentDepth) {
				multiLineData += line.substr((currentDepth + 1) * 2);
			} else {
				boost::trim(line);
				if (!line.empty()) {
					in.seekg(lastPos, std::ios::beg);
					currentLine--;
					break;
				}
			}
			multiLineData += "\n";
		} while (!in.eof());
		if (value == "|" || value == "|-") {
			int lastPos = multiLineData.length();
			while (multiLineData[--lastPos] == '\n') {
				multiLineData.erase(lastPos, 1);
			}

			if (value == "|") {
				multiLineData.append("\n");
			}
		}
		value = multiLineData;
	}
	OTMLNodePtr node = OTMLNode::create(tag);
	node->setUnique(dotsPos != std::string::npos);
	node->setTag(tag);
	node->setSource(doc->source() + ":" + otml_util::safeCast<std::string>(nodeLine));
	if (value == "~") {
		node->setNull(true);
	} else {
		if (boost::starts_with(value, "[") && boost::ends_with(value, "]")) {
			using Tokenizer = boost::tokenizer<boost::escaped_list_separator<char>>;
			std::string tmp = value.substr(1, value.length() - 2);
			Tokenizer tok(tmp);
			for (const auto& v_tok : tok) {
				std::string v = v_tok;
				boost::trim(v);
				node->writeIn(v);
			}
		} else {
			node->setValue(value);
		}
	}

	currentParent->addChild(node);
	previousNode = node;
}
