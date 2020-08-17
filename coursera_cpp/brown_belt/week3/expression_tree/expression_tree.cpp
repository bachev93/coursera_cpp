#include "Common.h"
#include "test_runner.h"

#include <sstream>

using namespace std;

class ValueExpression : public Expression {
 public:
  ValueExpression(int value) : value_(value) {}

  int Evaluate() const override { return value_; }

  string ToString() const override { return to_string(value_); }

 private:
  int value_;
};

class BinaryOpExpression : public Expression {
 public:
  BinaryOpExpression(ExpressionPtr leftOp, ExpressionPtr rightOp)
      : leftOperand_(move(leftOp)), rightOPerand_(move(rightOp)) {}

 protected:
  ExpressionPtr leftOperand_;
  ExpressionPtr rightOPerand_;
};

class SumExpression : public BinaryOpExpression {
 public:
  //    SumExpression(ExpressionPtr leftOp, ExpressionPtr rightOp)
  //        : BinaryOpExpression(move(leftOp), move(rightOp)) {}
  // Вместо переопределения конструктора используем конструктор базового класса
  using BinaryOpExpression::BinaryOpExpression;

  int Evaluate() const override {
    return leftOperand_->Evaluate() + rightOPerand_->Evaluate();
  }

  string ToString() const override {
    string result;
    result += '(' + leftOperand_->ToString() + ')' + '+' + '(' +
              rightOPerand_->ToString() + ')';
    return result;
  }
};

class ProductExpression : public BinaryOpExpression {
 public:
  using BinaryOpExpression::BinaryOpExpression;

  int Evaluate() const override {
    return leftOperand_->Evaluate() * rightOPerand_->Evaluate();
  }

  string ToString() const override {
    string result;
    result += '(' + leftOperand_->ToString() + ')' + '*' + '(' +
              rightOPerand_->ToString() + ')';
    return result;
  }
};

ExpressionPtr Value(int value) {
  return make_unique<ValueExpression>(value);
}

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right) {
  return make_unique<SumExpression>(move(left), move(right));
}

ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right) {
  return make_unique<ProductExpression>(move(left), move(right));
}

string Print(const Expression* e) {
  if (!e) {
    return "Null expression provided";
  }
  stringstream output;
  output << e->ToString() << " = " << e->Evaluate();
  return output.str();
}

void Test() {
  ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
  ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

  ExpressionPtr e2 = Sum(move(e1), Value(5));
  ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

  ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, Test);
  return 0;
}
