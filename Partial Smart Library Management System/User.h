#ifndef USER_H
#define USER_H

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

class Account {
protected:
    std::string id;
    std::string name;
    std::string password;

public:
    Account() = default;

    Account(const std::string& accountID, const std::string& accountName, const std::string& accountPassword)
        : id(accountID), name(accountName), password(accountPassword) {}

    virtual ~Account() = default;

    const std::string& getID() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getPassword() const { return password; }

    void setName(const std::string& newName) { name = newName; }
    void setPassword(const std::string& newPassword) { password = newPassword; }

    bool login(const std::string& enteredID, const std::string& enteredPassword) const {
        return id == enteredID && password == enteredPassword;
    }

    virtual std::string roleName() const = 0;
    virtual void showCapabilities() const = 0;
};

class User : public Account {
private:
    std::vector<std::string> borrowedBooks;
    std::vector<std::string> reservedBooks;
    double outstandingPenalty = 0.0;

    static void addUniqueBook(std::vector<std::string>& bookList, const std::string& bookID) {
        if (std::find(bookList.begin(), bookList.end(), bookID) == bookList.end()) {
            bookList.push_back(bookID);
        }
    }

public:
    User() = default;

    User(const std::string& userID, const std::string& userName, const std::string& userPassword)
        : Account(userID, userName, userPassword) {}

    std::string roleName() const override {
        return "Member";
    }

    void showCapabilities() const override {
        std::cout << "You can search books, check availability, borrow books, return books, reserve books, and view borrowing information.\n";
    }

    void addBorrowedBook(const std::string& bookID) {
        addUniqueBook(borrowedBooks, bookID);
    }

    void removeBorrowedBook(const std::string& bookID) {
        borrowedBooks.erase(
            std::remove(borrowedBooks.begin(), borrowedBooks.end(), bookID),
            borrowedBooks.end()
        );
    }

    void addReservedBook(const std::string& bookID) {
        addUniqueBook(reservedBooks, bookID);
    }

    void removeReservedBook(const std::string& bookID) {
        reservedBooks.erase(
            std::remove(reservedBooks.begin(), reservedBooks.end(), bookID),
            reservedBooks.end()
        );
    }

    int borrowedCount() const {
        return static_cast<int>(borrowedBooks.size());
    }

    bool hasBorrowedBook(const std::string& bookID) const {
        return std::find(borrowedBooks.begin(), borrowedBooks.end(), bookID) != borrowedBooks.end();
    }

    bool hasReservedBook(const std::string& bookID) const {
        return std::find(reservedBooks.begin(), reservedBooks.end(), bookID) != reservedBooks.end();
    }

    const std::vector<std::string>& getBorrowedBooks() const {
        return borrowedBooks;
    }

    const std::vector<std::string>& getReservedBooks() const {
        return reservedBooks;
    }

    void addPenalty(double amount) {
        if (amount > 0.0) {
            outstandingPenalty += amount;
        }
    }

    double getOutstandingPenalty() const {
        return outstandingPenalty;
    }

    void viewBorrowingInfo() const {
        std::cout << "User ID: " << id << '\n';
        std::cout << "Name: " << name << '\n';
        std::cout << "Role: " << roleName() << '\n';
        std::cout << "Borrowed books: " << borrowedBooks.size() << '\n';
        std::cout << "Reserved books: " << reservedBooks.size() << '\n';
        std::cout << "Outstanding penalties: $" << outstandingPenalty << '\n';
    }
};

#endif
