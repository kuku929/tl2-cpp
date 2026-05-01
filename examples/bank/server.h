/*
Emulates clients by randomly creating
transactions and spawning threads
to complete those transactions.
*/
#pragma once
#include "transaction.h"
#include "account.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>

class Server {
public:
  void spin();
  class Logger {
  public:
    void log_add(std::string name, int amt) { m_accounts[name] -= amt; }
    void log_withdraw(std::string name, int amt) { m_accounts[name] += amt; }
    bool check() {
      bool ok = true;
      std::cout << "Expected balance : " << std::format("{0}", m_accounts) << std::endl;
      for(const auto &[name, balance] : m_accounts) {
        if(accounts.get(name).balance != balance) {
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
  int random_int(int max, int min = 0);
  std::vector<std::thread> running;
  std::vector<std::string> names = {
      "Aarav",   "Vivaan", "Aditya",  "Vihaan", "Arjun",  "Reyansh",
      "Krishna", "Ishaan", "Shaurya", "Atharv", "Ananya", "Diya",
      "Saanvi",  "Aadhya", "Kavya",   "Riya",   "Ira",    "Myra",
      "Anika",   "Sara",   "Rohan",   "Karan",  "Rahul",  "Neha",
      "Pooja",   "Sneha",  "Amit",    "Nikhil", "Priya",  "Meera"};

};