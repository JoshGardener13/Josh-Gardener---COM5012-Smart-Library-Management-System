#ifndef ADMINISTRATOR_H
#define ADMINISTRATOR_H

#include "User.h"
#include <iostream>
#include <string>

class Administrator : public Account {
public:
    Administrator() = default;

    Administrator(const std::string& administratorID, const std::string& administratorName, const std::string& administratorPassword)
        : Account(administratorID, administratorName, administratorPassword) {}

    std::string roleName() const override {
        return "Administrator";
    }

    void showCapabilities() const override {
        std::cout << "You can manage member accounts, manage librarian accounts, define library rules, create late return penalties, and review reports.\n";
    }
};

#endif
