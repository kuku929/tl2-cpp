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
    Withdraw(std::string name, int amt) : m_name(name), m_amt(amt) {;}
    void execute() const override;
private:
    std::string m_name;
    int m_amt;
};