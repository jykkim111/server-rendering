#pragma once

#include <vector>
#include <string>

//#include "../CGIPIO/MaskInfo.h"

namespace cgip {
	class RemoteFileTree {

	public:
		RemoteFileTree(std::string value, std::string name, std::string path, bool is_file, RemoteFileTree* parent = nullptr);
		~RemoteFileTree();

		// Tree manipulation
		void addChild(RemoteFileTree* child);
		inline int getCntChildren() { return m_children.size(); }
		inline RemoteFileTree* getParent() { return m_parent; }
		RemoteFileTree* getChild(int idx);
		int getChildIndex();
		inline std::vector<RemoteFileTree*> getChildren() { return m_children; }
		bool removeChild(int idx);

		// Access node value
		inline std::string getValue() { return m_value; }
		inline std::string getPath() { return m_path; }
		inline std::string getName() { return m_name; }
		inline bool getIsFile() { return m_is_file; }

		void print(int level = 0, bool recursive = true);
	private:
		std::string m_value;
		std::string m_path;
		std::string m_name;
		bool m_is_file;

		// Tree info
		std::vector<RemoteFileTree*> m_children;
		RemoteFileTree* m_parent;
	};
}