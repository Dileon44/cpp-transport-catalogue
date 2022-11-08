#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace json {

	class DictItemContext;
	class KeyItemContext;
	class ArrayItemContext;
	//class Builder;

	class Builder {
	public:
		Builder() = default;

		Builder& EndDict();
		Builder& Value(Node::Value value);
		Builder& EndArray();

		DictItemContext StartDict();
		ArrayItemContext StartArray();
		KeyItemContext Key(std::string key);

		Node Build() const;
	private:
		Node root_;
		std::vector<Node*> nodes_stack_;
	};

	// 1
	class KeyItemContext {
	public:
		KeyItemContext(Builder& builder);
		DictItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
	private:
		Builder& builder_;
	};

	// 2
	class DictItemContext {
	public:
		DictItemContext(Builder& builder);
		KeyItemContext Key(std::string key);
		Builder& EndDict();
	private:
		Builder& builder_;
	};

	// 3
	class ArrayItemContext {
	public:
		ArrayItemContext(Builder& builder);
		ArrayItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
	private:
		Builder& builder_;
	};
}
