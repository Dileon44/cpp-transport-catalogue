#pragma once

#include "json.h"

#include <string>
#include <vector>


namespace json {

	class DictItemContext;
	class KeyItemContext;
	class ValueAfterKeyContext;
	class ArrayContext;
	class ValueArrayContext;
	class ValueContext;
	class Builder;
	class Node;

	class BuilderContext {
	public:
		BuilderContext() = default;
		BuilderContext(Builder& builder);
		BuilderContext(ValueAfterKeyContext value);
		BuilderContext(ArrayContext array_context);
		BuilderContext(ValueArrayContext value);

		DictItemContext StartDict();
		BuilderContext EndDict();
		ArrayContext StartArray();
		BuilderContext EndArray();
		KeyItemContext Key(std::string key);
		ValueContext Value(Node::Value value);
		Node Build() const;
	private:
		Builder& builder_;
	};

	// Класс-родитель, от него далее наследуются ValueAfterKeyContext и ValueArrayContext
	class ValueContext : public BuilderContext {
	public:
		ValueContext(BuilderContext builder_context)
			: BuilderContext(std::move(builder_context)) {
		}
	};

	// 1
	class KeyItemContext : public BuilderContext {
	public:
		KeyItemContext(BuilderContext builder_context);
		ValueAfterKeyContext Value(Node::Value value);
		BuilderContext Key() = delete;
		BuilderContext EndDict() = delete;
		BuilderContext EndArray() = delete;
		Node Build() const = delete;
	};

	// 2
	class ValueAfterKeyContext : public BuilderContext {
	public:
		ValueAfterKeyContext(BuilderContext builder_context);
		BuilderContext StartDict() = delete;
		BuilderContext StartArray() = delete;
		BuilderContext EndArray() = delete;
		BuilderContext Value(Node::Value value) = delete;
		Node Build() const = delete;
	};

	// 3
	class DictItemContext : public BuilderContext {
	public:
		DictItemContext(BuilderContext builder_context);
		BuilderContext StartDict() = delete;
		BuilderContext StartArray() = delete;
		BuilderContext EndArray() = delete;
		BuilderContext Value(Node::Value value) = delete;
		Node Build() const = delete;
	};

	// 4
	class ArrayContext : public BuilderContext {
	public:
		ArrayContext(BuilderContext builder_context);
		ValueArrayContext Value(Node::Value value);
		BuilderContext Key() = delete;
		BuilderContext EndDict() = delete;
		Node Build() const = delete;
	};

	// 5
	class ValueArrayContext : public BuilderContext {
	public:
		ValueArrayContext(BuilderContext builder_context);
		ValueArrayContext Value(Node::Value value);
		BuilderContext Key() = delete;
		BuilderContext EndDict() = delete;
		Node Build() const = delete;
	};

	class Builder {
	public:
		Builder() = default;
		DictItemContext StartDict();
		Builder EndDict();
		KeyItemContext Key(std::string key);
		ValueContext Value(Node::Value value);
		ArrayContext StartArray();
		Builder EndArray();
		Node Build() const;
	private:
		Node root_;
		std::vector<Node*> nodes_stack_;
	};
}