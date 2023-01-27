#include "RemoteFileTree.h"

#include <iostream>
#include <functional>

namespace cgip {

	RemoteFileTree::RemoteFileTree(std::string value, std::string name, std::string path, bool is_file, RemoteFileTree* parent) {
		m_value = value;
		m_path = path;
		m_name = name;
		m_is_file = is_file;

		m_parent = parent;
	}

	RemoteFileTree::~RemoteFileTree() {
		//TODO:: fix here
		// Deallocate children
		for (int i = 0; i < m_children.size(); i++) {
			delete m_children[i];
		}
	}

	RemoteFileTree* RemoteFileTree::getChild(int idx) {
		if (idx >= getCntChildren()) {
			return nullptr;
		}

		return m_children[idx];
	}

	int RemoteFileTree::getChildIndex() {
		if (m_parent) {
			for (int i = 0; i < m_parent->getCntChildren(); i++) {
				if (m_parent->m_children[i] == this) return i;
			}
		}

		return 0;
	}

	bool RemoteFileTree::removeChild(int idx) {
		if (idx >= getCntChildren()) {
			return false;
		}

		// Remove the child data
		RemoteFileTree* child = m_children[idx];
		delete child;
		child = nullptr;

		m_children.erase(m_children.begin() + idx);

		return true;
	}



	void RemoteFileTree::addChild(RemoteFileTree* child) {
		child->m_parent = this;

		m_children.push_back(child);
	}

	void RemoteFileTree::print(int level, bool recursive) {

		// Print current node
		for (int i = 0; i < level; i++)
			std::cout << "\t";
		std::cout << m_value << std::endl;

		if (recursive == false) return;

		// Print children recursively 
		for (int i = 0; i < m_children.size(); i++) {
			m_children[i]->print(level + 1, recursive);
		}
	}
}