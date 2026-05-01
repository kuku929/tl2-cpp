/*
Emulates clients by randomly creating
transactions and spawning threads
to complete those transactions.
*/
#pragma once
#include "transaction.h"
#include <thread>
#include <vector>

class Server {
public:
  void spin();

private:
  std::string random_name();
  Withdraw construct_withdraw();
  int random_int(int max, int min = 0);
  std::vector<std::thread> running;
  std::vector<std::string> names = {
      "Aarav",   "Vivaan", "Aditya",  "Vihaan", "Arjun",  "Reyansh",
      "Krishna", "Ishaan", "Shaurya", "Atharv", "Ananya", "Diya",
      "Saanvi",  "Aadhya", "Kavya",   "Riya",   "Ira",    "Myra",
      "Anika",   "Sara",   "Rohan",   "Karan",  "Rahul",  "Neha",
      "Pooja",   "Sneha",  "Amit",    "Nikhil", "Priya",  "Meera"};
};