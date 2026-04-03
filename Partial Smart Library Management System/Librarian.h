#ifndef LIBRARIAN_H
#define LIBRARIAN_H

#include "User.h"
#include <iostream>
#include <string>

class Librarian : public Account {
public:
    Librarian() = default;

    Librarian(const std::string& librarianID, const std::string& librarianName, const std::string& librarianPassword)
        : Account(librarianID, librarianName, librarianPassword) {}

    std::string roleName() const override {
        return "Librarian";
    }

    void showCapabilities() const override {
        std::cout << "You can add books, update books, remove books, handle borrow requests, and generate reports.\n";
    }
};

#endif