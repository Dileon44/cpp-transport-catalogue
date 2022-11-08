#include "json_builder.h"

#include <string>
#include <vector>

using namespace std;

namespace json {

	//====================  KeyItemContext  ==================================

	KeyItemContext::KeyItemContext(Builder& builder)
		: builder_(builder) {
	}

	DictItemContext KeyItemContext::Value(Node::Value value) {
		return DictItemContext{ builder_.Value(std::move(value)) };
	}

	DictItemContext KeyItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext KeyItemContext::StartArray() {
		return builder_.StartArray();
	}

	//====================  DictItemContext  =================================

	DictItemContext::DictItemContext(Builder& builder) 
		: builder_(builder) {
	}

	KeyItemContext DictItemContext::Key(std::string key) {
		return builder_.Key(std::move(key));
	}

	Builder& DictItemContext::EndDict() {
		return builder_.EndDict();
	}

	//====================  ArrayItemContext  ====================================

	ArrayItemContext::ArrayItemContext(Builder& builder) 
		: builder_(builder) {
	}

	ArrayItemContext ArrayItemContext::Value(Node::Value value) {
		return ArrayItemContext{ builder_.Value(std::move(value)) };
	}

	DictItemContext ArrayItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext ArrayItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}

	//====================  Builder  =========================================

	KeyItemContext Builder::Key(std::string key) {
		if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
			throw std::logic_error("Failed Key(): the last element of the vector is not a dictionary"s);
		}
		nodes_stack_.emplace_back(&std::get<Dict>(nodes_stack_.back()->GetNoConstValue())[key]);
		return KeyItemContext{ *this };
	}

	Builder& Builder::Value(Node::Value value) {
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

		return *this;
	}

	Node Builder::Build() const {
		if (nodes_stack_.empty() && !root_.IsNull()) {
			return root_;
		}
		throw std::logic_error("Failed Build()"s);
	}

	DictItemContext Builder::StartDict() {
		return DictItemContext{ this->StartContainer(Dict{}) };
	}

	Builder& Builder::EndDict() {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			nodes_stack_.erase(nodes_stack_.end() - 1);
		}
		else {
			throw std::logic_error("Failed EndDict()"s);
		}

		return *this;
	}

	ArrayItemContext Builder::StartArray() {
		return ArrayItemContext{ this->StartContainer(Array{}) };
	}

	Builder& Builder::EndArray() {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			nodes_stack_.erase(nodes_stack_.end() - 1);
		}
		else {
			throw std::logic_error("Failed EndArray()"s);
		}

		return *this;
	}
}
