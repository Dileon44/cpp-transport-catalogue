#include "json_builder.h"

#include <string>
#include <vector>

using namespace std;

namespace json {

	//====================  BuilderContext  ==================================

	BuilderContext::BuilderContext(Builder& builder)
		: builder_(builder) {
	}
	BuilderContext::BuilderContext(ValueAfterKeyContext value)
		: builder_(value.builder_) {
	}
	BuilderContext::BuilderContext(ArrayContext array_context) 
		: builder_(array_context.builder_) {
	}
	BuilderContext::BuilderContext(ValueArrayContext value) 
		: builder_(value.builder_) {}
	KeyItemContext BuilderContext::Key(std::string key) {
		builder_.Key(std::move(key));
		return *this;
	}
	ValueContext BuilderContext::Value(Node::Value value) {
		builder_.Value(std::move(value));
		//return builder_.Value(std::move(value));
		return *this;
	}
	Node BuilderContext::Build() const {
		return builder_.Build();
	}

	DictItemContext BuilderContext::StartDict() {
		//builder_.StartDict();
		return DictItemContext{ builder_.StartDict() };
		//return *this;
	}
	BuilderContext BuilderContext::EndDict() {
		builder_.EndDict();
		return *this;
	}
	ArrayContext BuilderContext::StartArray() {
		//builder_.StartArray();
		return ArrayContext{ builder_.StartArray() };
		//return *this;
	}
	BuilderContext BuilderContext::EndArray() {
		builder_.EndArray();
		return *this;
	}


	//====================  KeyItemContext  ==================================
	
	KeyItemContext::KeyItemContext(BuilderContext builder_context)
		: BuilderContext(builder_context) {
	}

	ValueAfterKeyContext KeyItemContext::Value(Node::Value value) {
		return static_cast<ValueAfterKeyContext>(BuilderContext::Value(std::move(value)));
	}

	//====================  ValueAfterKeyContext  ============================

	ValueAfterKeyContext::ValueAfterKeyContext(BuilderContext builder_context)
		: BuilderContext(std::move(builder_context)) {
	}

	//====================  DictItemContext  =================================

	DictItemContext::DictItemContext(BuilderContext builder_context)
		: BuilderContext(std::move(builder_context)) {
	}
	
	//====================  ArrayContext  ====================================

	ArrayContext::ArrayContext(BuilderContext builder_context)
		: BuilderContext(builder_context) {
	}

	ValueArrayContext ArrayContext::Value(Node::Value value) {
		return static_cast<ValueArrayContext>(BuilderContext::Value(std::move(value)));
	}

	//====================  ValueArrayContext  ==================================

	ValueArrayContext::ValueArrayContext(BuilderContext builder_context)
		: BuilderContext(std::move(builder_context)) {
	}

	ValueArrayContext ValueArrayContext::Value(Node::Value value) {
		return static_cast<ValueArrayContext>(BuilderContext::Value(std::move(value)));
	}

	//====================  Builder  =========================================

	KeyItemContext Builder::Key(std::string key) {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			nodes_stack_.emplace_back(&std::get<Dict>(nodes_stack_.back()->GetNoConstValue())[key]);
		}
		else {
			throw std::logic_error("Failed Key(): the last element of the vector is not a dictionary"s);
		}

		return KeyItemContext{ *this };
	}

	ValueContext Builder::Value(Node::Value value) {
		if (nodes_stack_.empty() && std::holds_alternative<nullptr_t>(root_.GetValue())) {
			root_ = value;
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) {
			nodes_stack_.back()->GetNoConstValue() = value;
			nodes_stack_.erase(nodes_stack_.end() - 1);
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			std::get<Array>(nodes_stack_.back()->GetNoConstValue()).push_back(value);
		}
		else {
			throw std::logic_error("Failed Value()"s);
		}

		return ValueContext{ *this };
	}

	Node Builder::Build() const {
		if (nodes_stack_.empty() && !root_.IsNull()) {
			return root_;
		}
		throw std::logic_error("Failed Build()"s);
	}

	DictItemContext Builder::StartDict() {
		if (nodes_stack_.empty() && std::holds_alternative<nullptr_t>(root_.GetValue())) {
			root_ = Dict{};
			nodes_stack_.push_back(&root_);
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) {
			nodes_stack_.back()->GetNoConstValue() = Dict{};
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			std::get<Array>(nodes_stack_.back()->GetNoConstValue()).push_back(Dict{});
			nodes_stack_.emplace_back(&(std::get<Array>(nodes_stack_.back()->GetNoConstValue())).back());
		}
		else {
			throw std::logic_error("Failed StartDict()"s);
		}

		return DictItemContext{ *this };
	}

	Builder Builder::EndDict() {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			nodes_stack_.erase(nodes_stack_.end() - 1);
		}
		else {
			throw std::logic_error("Failed EndDict()"s);
		}

		return *this;
	}

	ArrayContext Builder::StartArray() {
		if (nodes_stack_.empty() && std::holds_alternative<nullptr_t>(root_.GetValue())) {
			root_ = Array{};
			nodes_stack_.push_back(&root_);
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) {
			nodes_stack_.back()->GetNoConstValue() = Array{};
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			std::get<Array>(nodes_stack_.back()->GetNoConstValue()).push_back(Array{});
			nodes_stack_.emplace_back(&std::get<Array>(nodes_stack_.back()->GetNoConstValue()).back());
		}
		else {
			throw std::logic_error("Failed StartArray()"s);
		}

		return ArrayContext{ *this };
	}

	Builder Builder::EndArray() {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			nodes_stack_.erase(nodes_stack_.end() - 1);
		}
		else {
			throw std::logic_error("Failed EndArray()"s);
		}

		return *this;
	}
}