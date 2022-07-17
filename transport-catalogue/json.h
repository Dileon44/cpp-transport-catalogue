#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
    using namespace std::literals;

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        PrintContext(std::ostream& out);

        PrintContext(std::ostream& out, int indent_step, int indent = 0);

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, const PrintContext& ctx);

    // Перегрузка функции PrintValue для вывода значений bool
    void PrintValue(bool value, const PrintContext& ctx);

    // Перегрузка функции PrintValue для вывода значений Array
    void PrintValue(const Array& value, const PrintContext& ctx);

    // Перегрузка функции PrintValue для вывода значений Dict
    void PrintValue(const Dict& value, const PrintContext& ctx);

    void PrintNode(const Node& node, std::ostream& out);

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        Node() = default;
        Node(Value value);
        template <typename Variable>
        Node(Variable variable);


        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        const Value& GetValue() const;

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

    private:
        Value value_ = nullptr;
    };

    std::ostream& operator<<(std::ostream& out, const Node& node);

    class Document {
    public:
        explicit Document();
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    template <typename Variable>
    Node::Node(Variable variable) {
        value_ = variable;
    }

}  // namespace json