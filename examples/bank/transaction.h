/*
Types of transactions supported by the bank.
*/
#pragma once
#include <string>

class AbstractTransaction {
public:
  virtual void execute() const = 0;
};

class Withdraw : public AbstractTransaction {
public:
  Withdraw(std::string name, int amt) : m_name(name), m_amt(amt) { ; }
  void execute() const override;
  std::string name() { return m_name; }
  int amt() { return m_amt; }

private:
  std::string m_name;
  int m_amt;
};

class Add : public AbstractTransaction {
public:
  Add(std::string name, int amt) : m_name(name), m_amt(amt) { ; }
  void execute() const override;
  std::string name() { return m_name; }
  int amt() { return m_amt; }

private:
  std::string m_name;
  int m_amt;
};

class Transfer : public AbstractTransaction {
public:
  Transfer(std::string src, std::string dest, int amt) : m_src(src), m_dest(dest), m_amt(amt) { ; }
  void execute() const override;
  std::string src() { return m_src; }
  std::string dest() { return m_dest; }
  int amt() { return m_amt; }

private:
  std::string m_src;
  std::string m_dest;
  int m_amt;
};