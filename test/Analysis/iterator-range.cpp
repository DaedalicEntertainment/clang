// RUN: %clang_analyze_cc1 -std=c++11 -analyzer-checker=core,cplusplus,alpha.cplusplus.IteratorRange -analyzer-config aggressive-binary-operation-simplification=true -analyzer-config c++-container-inlining=false %s -verify
// RUN: %clang_analyze_cc1 -std=c++11 -analyzer-checker=core,cplusplus,alpha.cplusplus.IteratorRange -analyzer-config aggressive-binary-operation-simplification=true -analyzer-config c++-container-inlining=true -DINLINE=1 %s -verify

#include "Inputs/system-header-simulator-cxx.h"

void clang_analyzer_warnIfReached();

void simple_good_end(const std::vector<int> &v) {
  auto i = v.end();
  if (i != v.end()) {
    clang_analyzer_warnIfReached();
    *i; // no-warning
  }
}

void simple_good_end_negated(const std::vector<int> &v) {
  auto i = v.end();
  if (!(i == v.end())) {
    clang_analyzer_warnIfReached();
    *i; // no-warning
  }
}

void simple_bad_end(const std::vector<int> &v) {
  auto i = v.end();
  *i; // expected-warning{{Iterator accessed outside of its range}}
}

void simple_good_begin(const std::vector<int> &v) {
  auto i = v.begin();
  if (i != v.begin()) {
    clang_analyzer_warnIfReached();
    *--i; // no-warning
  }
}

void simple_good_begin_negated(const std::vector<int> &v) {
  auto i = v.begin();
  if (!(i == v.begin())) {
    clang_analyzer_warnIfReached();
    *--i; // no-warning
  }
}

void simple_bad_begin(const std::vector<int> &v) {
  auto i = v.begin();
  *--i; // expected-warning{{Iterator accessed outside of its range}}
}

void copy(const std::vector<int> &v) {
  auto i1 = v.end();
  auto i2 = i1;
  *i2; // expected-warning{{Iterator accessed outside of its range}}
}

void decrease(const std::vector<int> &v) {
  auto i = v.end();
  --i;
  *i; // no-warning
}

void copy_and_decrease1(const std::vector<int> &v) {
  auto i1 = v.end();
  auto i2 = i1;
  --i1;
  *i1; // no-warning
}

void copy_and_decrease2(const std::vector<int> &v) {
  auto i1 = v.end();
  auto i2 = i1;
  --i1;
  *i2; // expected-warning{{Iterator accessed outside of its range}}
}

void copy_and_increase1(const std::vector<int> &v) {
  auto i1 = v.begin();
  auto i2 = i1;
  ++i1;
  if (i1 == v.end())
    *i2; // no-warning
}

void copy_and_increase2(const std::vector<int> &v) {
  auto i1 = v.begin();
  auto i2 = i1;
  ++i1;
  if (i2 == v.end())
    *i2; // expected-warning{{Iterator accessed outside of its range}}
}

void copy_and_increase3(const std::vector<int> &v) {
  auto i1 = v.begin();
  auto i2 = i1;
  ++i1;
  if (v.end() == i2)
    *i2; // expected-warning{{Iterator accessed outside of its range}}
}

template <class InputIterator, class T>
InputIterator nonStdFind(InputIterator first, InputIterator last,
                         const T &val) {
  for (auto i = first; i != last; ++i) {
    if (*i == val) {
      return i;
    }
  }
  return last;
}

void good_non_std_find(std::vector<int> &V, int e) {
  auto first = nonStdFind(V.begin(), V.end(), e);
  if (V.end() != first)
    *first; // no-warning
}

void bad_non_std_find(std::vector<int> &V, int e) {
  auto first = nonStdFind(V.begin(), V.end(), e);
  *first; // expected-warning{{Iterator accessed outside of its range}}
}

void tricky(std::vector<int> &V, int e) {
  const auto first = V.begin();
  const auto comp1 = (first != V.end()), comp2 = (first == V.end());
  if (comp1)
    *first;
}

void loop(std::vector<int> &V, int e) {
  auto start = V.begin();
  while (true) {
    auto item = std::find(start, V.end(), e);
    if (item == V.end())
      break;
    *item;          // no-warning
    start = ++item; // no-warning
  }
}

void good_push_back(std::list<int> &L, int n) {
  auto i0 = --L.cend();
  L.push_back(n);
  *++i0; // no-warning
}

void bad_push_back(std::list<int> &L, int n) {
  auto i0 = --L.cend();
  L.push_back(n);
  ++i0;
  *++i0; // expected-warning{{Iterator accessed outside of its range}}
}

void good_pop_back(std::list<int> &L, int n) {
  auto i0 = --L.cend(); --i0;
  L.pop_back();
  *i0; // no-warning
}

void bad_pop_back(std::list<int> &L, int n) {
  auto i0 = --L.cend(); --i0;
  L.pop_back();
  *++i0; // expected-warning{{Iterator accessed outside of its range}}
}

void good_push_front(std::list<int> &L, int n) {
  auto i0 = L.cbegin();
  L.push_front(n);
  *--i0; // no-warning
}

void bad_push_front(std::list<int> &L, int n) {
  auto i0 = L.cbegin();
  L.push_front(n);
  --i0;
  *--i0; // expected-warning{{Iterator accessed outside of its range}}
}

void good_pop_front(std::list<int> &L, int n) {
  auto i0 = ++L.cbegin();
  L.pop_front();
  *i0; // no-warning
}

void bad_pop_front(std::list<int> &L, int n) {
  auto i0 = ++L.cbegin();
  L.pop_front();
  *--i0; // expected-warning{{Iterator accessed outside of its range}}
}

void bad_move(std::list<int> &L1, std::list<int> &L2) {
  auto i0 = --L2.cend();
  L1 = std::move(L2);
  *++i0; // expected-warning{{Iterator accessed outside of its range}}
}

void bad_move_push_back(std::list<int> &L1, std::list<int> &L2, int n) {
  auto i0 = --L2.cend();
  L2.push_back(n);
  L1 = std::move(L2);
  ++i0;
  *++i0; // expected-warning{{Iterator accessed outside of its range}}
}
