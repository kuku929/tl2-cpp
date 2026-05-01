/*
Emulates clients by randomly creating
transactions and spawning threads
to complete those transactions.
*/
#pragma once
#include "account.h"
#include "transaction.h"
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class Server {
public:
  void spin();
  class Logger {
  public:
    void log_add(std::string name, int amt) { m_accounts[name] += amt; }
    void log_withdraw(std::string name, int amt) { m_accounts[name] -= amt; }
    void log_transfer(std::string src, std::string dest, int amt) {
      m_accounts[src]  -= amt;
      m_accounts[dest] += amt;
    }
    bool check() {
      bool ok = true;
      for (const auto &[name, balance] : m_accounts) {
        if (auto a = accounts.get(name);
            a.has_value() && a->balance != balance) {
          // std::cout << "~" << std::endl;
          // std::cout << "Expected balance : "
          //           << std::format("{0}", std::pair(name, balance))
          //           << std::endl;
          // std::cout << "Actual   balance : "
          //           << std::format("{0}", std::pair(name, actual_bal))
                    // << std::endl;
          ok = false;
        }
      }
      return ok;
    }

  private:
    std::unordered_map<std::string, int> m_accounts;
  } logger;

private:
  std::string random_name();
  Withdraw construct_withdraw();
  Add construct_add();
  Transfer construct_transfer();

  int random_int(int max, int min = 0);
  std::vector<std::thread> running;
  std::vector<std::string> names = {
      "Aarav",   "Vivaan", "Aditya",  "Vihaan", "Arjun",  "Reyansh",
      "Krishna", "Ishaan", "Shaurya", "Atharv", "Ananya", "Diya",
      "Saanvi",  "Aadhya", "Kavya",   "Riya",   "Ira",    "Myra",
      "Anika",   "Sara",   "Rohan",   "Karan",  "Rahul",  "Neha",
      "Pooja",   "Sneha",  "Amit",    "Nikhil", "Priya",  "Meera"};
};