#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace json {

	class DictItemContext;
	class KeyItemContext;
	class ArrayItemContext;

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

		template<typename Container>
		Builder& StartContainer(Container container);
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

	template<typename Container>
	Builder& Builder::StartContainer(Container container) {
		if (nodes_stack_.empty() && std::holds_alternative<nullptr_t>(root_.GetValue())) {
			root_ = container;
			nodes_stack_.push_back(&root_);
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) {
			nodes_stack_.back()->GetNoConstValue() = container;
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			std::get<Array>(nodes_stack_.back()->GetNoConstValue()).push_back(container);
			nodes_stack_.emplace_back(&(std::get<Array>(nodes_stack_.back()->GetNoConstValue()).back()));
		}
		else {
			throw std::logic_error("Failed StartArray()");
		}

		return *this;
	}
}
