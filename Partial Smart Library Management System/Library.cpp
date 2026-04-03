#include "Book.h"
#include "User.h"
#include "Librarian.h"
#include "Administrator.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace util {
    using Clock = std::chrono::system_clock;
    using TimePoint = std::chrono::system_clock::time_point;

    TimePoint now();
    TimePoint addDays(const TimePoint& timePoint, int days);
    long long daysBetween(const TimePoint& from, const TimePoint& to);
    std::string formatDate(const TimePoint& timePoint);
    std::string formatMoney(double amount);
    std::string toLower(std::string value);
    bool containsIgnoreCase(const std::string& text, const std::string& target);
    std::string trim(std::string value);
    void clearInput();
    std::string readLine(const std::string& prompt);
    std::string readNonEmptyLine(const std::string& prompt);
    int readInt(const std::string& prompt, int minValue, int maxValue);
    double readDouble(const std::string& prompt, double minValue);
}

class Library {
private:
    struct NotificationEvent {
        std::string recipientID;
        std::string message;
        util::TimePoint createdAt;
    };

    static constexpr int hardBorrowingCap = 5;

    int borrowingLimit = hardBorrowingCap;
    const int reservationExpiryDays = 3;
    const int borrowingDays = 14;
    double latePenaltyPerDay = 1.00;

    std::vector<Book> books;
    std::vector<User> users;
    std::vector<Librarian> librarians;
    std::vector<Administrator> administrators;
    std::vector<NotificationEvent> notificationLog;

    Book* findBookByID(const std::string& bookID);
    const Book* findBookByID(const std::string& bookID) const;
    User* findUserByID(const std::string& userID);
    const User* findUserByID(const std::string& userID) const;
    Librarian* findLibrarianByID(const std::string& librarianID);
    const Librarian* findLibrarianByID(const std::string& librarianID) const;
    Administrator* findAdministratorByID(const std::string& adminID);
    const Administrator* findAdministratorByID(const std::string& adminID) const;

    bool accountIDExists(const std::string& accountID) const;
    void addNotification(const std::string& message, const std::string& recipientID = "");
    void showAccountIntro(const Account& account) const;
    long long getOverdueDays(const Book& book) const;
    std::string statusLabel(const Book& book) const;
    void printBookDetails(const Book& book) const;
    void showSearchResults(const std::vector<const Book*>& results) const;
    void removeBookFromAllUserLists(const std::string& bookID);
    std::vector<std::string> buildLiveNotifications(const User& user) const;
    void expireReservations();

public:
    void loadSampleData();

    bool addBookRecord(const std::string& bookID, const std::string& title, const std::string& author);
    bool updateBookRecord(const std::string& bookID, const std::string& title, const std::string& author);
    bool removeBookRecord(const std::string& bookID);

    std::vector<const Book*> searchBooksByTitle(const std::string& title) const;
    std::vector<const Book*> searchBooksByAuthor(const std::string& author) const;
    void checkAvailability(const std::string& bookID) const;

    bool processBorrowRequest(const std::string& userID, const std::string& bookID);
    bool returnBook(const std::string& userID, const std::string& bookID);
    bool reserveBook(const std::string& userID, const std::string& bookID);

    bool addUserAccount(const std::string& userID, const std::string& name, const std::string& password);
    bool updateUserAccount(const std::string& userID, const std::string& name, const std::string& password);
    bool removeUserAccount(const std::string& userID);

    bool addLibrarianAccount(const std::string& librarianID, const std::string& name, const std::string& password);
    bool updateLibrarianAccount(const std::string& librarianID, const std::string& name, const std::string& password);
    bool removeLibrarianAccount(const std::string& librarianID);

    void setBorrowingLimit(int limit);
    void setLatePenalty(double penalty);

    User* loginUser(const std::string& userID, const std::string& password);
    Librarian* loginLibrarian(const std::string& librarianID, const std::string& password);
    Administrator* loginAdministrator(const std::string& adminID, const std::string& password);

    void showAllBooks() const;
    void showUserBorrowingInfo(const std::string& userID);
    void showMemberNotifications(const User& user);
    void showSystemNotifications();
    void generateLibraryReport() const;
    void generateOverdueReport() const;
    void generateMemberActivityReport() const;
    void showRules() const;
    void listAccounts() const;

    void userMenu(User& currentUser);
    void librarianMenu(Librarian& currentLibrarian);
    void administratorMenu(Administrator& currentAdministrator);
    void mainMenu();
};

namespace util {
    TimePoint now() {
        return Clock::now();
    }

    TimePoint addDays(const TimePoint& timePoint, int days) {
        return timePoint + std::chrono::hours(24LL * days);
    }

    long long daysBetween(const TimePoint& from, const TimePoint& to) {
        auto hours = std::chrono::duration_cast<std::chrono::hours>(to - from).count();
        return hours / 24;
    }

    std::string formatDate(const TimePoint& timePoint) {
        std::time_t raw = Clock::to_time_t(timePoint);
        std::tm result{};
    #ifdef _WIN32
        localtime_s(&result, &raw);
    #else
        localtime_r(&raw, &result);
    #endif

        std::ostringstream out;
        out << std::put_time(&result, "%Y-%m-%d");
        return out.str();
    }

    std::string formatMoney(double amount) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << amount;
        return out.str();
    }

    std::string toLower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }

    bool containsIgnoreCase(const std::string& text, const std::string& target) {
        return toLower(text).find(toLower(target)) != std::string::npos;
    }

    std::string trim(std::string value) {
        auto isSpace = [](unsigned char ch) {
            return std::isspace(ch) != 0;
        };

        auto begin = std::find_if_not(value.begin(), value.end(), isSpace);
        auto end = std::find_if_not(value.rbegin(), value.rend(), isSpace).base();
        if (begin >= end) {
            return "";
        }

        return std::string(begin, end);
    }

    void clearInput() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::string readLine(const std::string& prompt) {
        std::cout << prompt;
        std::cout.flush();
        std::string value;
        std::getline(std::cin, value);
        return trim(value);
    }

    std::string readNonEmptyLine(const std::string& prompt) {
        while (true) {
            std::string value = readLine(prompt);
            if (!value.empty()) {
                return value;
            }

            std::cout << "Input cannot be empty.\n";
        }
    }

    int readInt(const std::string& prompt, int minValue, int maxValue) {
        while (true) {
            std::cout << prompt;
            std::cout.flush();
            int value = 0;
            if (std::cin >> value && value >= minValue && value <= maxValue) {
                clearInput();
                return value;
            }

            std::cout << "Invalid input. Enter a number between "
                      << minValue << " and " << maxValue << ".\n";
            clearInput();
        }
    }

    double readDouble(const std::string& prompt, double minValue) {
        while (true) {
            std::cout << prompt;
            std::cout.flush();
            double value = 0.0;
            if (std::cin >> value && value >= minValue) {
                clearInput();
                return value;
            }

            std::cout << "Invalid input. Enter a number greater than or equal to "
                      << minValue << ".\n";
            clearInput();
        }
    }
}

Book* Library::findBookByID(const std::string& bookID) {
    for (auto& book : books) {
        if (book.getBookID() == bookID) {
            return &book;
        }
    }
    return nullptr;
}

const Book* Library::findBookByID(const std::string& bookID) const {
    for (const auto& book : books) {
        if (book.getBookID() == bookID) {
            return &book;
        }
    }
    return nullptr;
}

User* Library::findUserByID(const std::string& userID) {
    for (auto& user : users) {
        if (user.getID() == userID) {
            return &user;
        }
    }
    return nullptr;
}

const User* Library::findUserByID(const std::string& userID) const {
    for (const auto& user : users) {
        if (user.getID() == userID) {
            return &user;
        }
    }
    return nullptr;
}

Librarian* Library::findLibrarianByID(const std::string& librarianID) {
    for (auto& librarian : librarians) {
        if (librarian.getID() == librarianID) {
            return &librarian;
        }
    }
    return nullptr;
}

const Librarian* Library::findLibrarianByID(const std::string& librarianID) const {
    for (const auto& librarian : librarians) {
        if (librarian.getID() == librarianID) {
            return &librarian;
        }
    }
    return nullptr;
}

Administrator* Library::findAdministratorByID(const std::string& adminID) {
    for (auto& administrator : administrators) {
        if (administrator.getID() == adminID) {
            return &administrator;
        }
    }
    return nullptr;
}

const Administrator* Library::findAdministratorByID(const std::string& adminID) const {
    for (const auto& administrator : administrators) {
        if (administrator.getID() == adminID) {
            return &administrator;
        }
    }
    return nullptr;
}

bool Library::accountIDExists(const std::string& accountID) const {
    return findUserByID(accountID) != nullptr ||
           findLibrarianByID(accountID) != nullptr ||
           findAdministratorByID(accountID) != nullptr;
}

void Library::addNotification(const std::string& message, const std::string& recipientID) {
    notificationLog.push_back({recipientID, message, util::now()});
}

void Library::showAccountIntro(const Account& account) const {
    std::cout << "\nLogged in as " << account.roleName() << ": " << account.getName() << '\n';
    account.showCapabilities();
}

long long Library::getOverdueDays(const Book& book) const {
    if (!book.getDueDate().has_value()) {
        return 0;
    }

    const util::TimePoint currentTime = util::now();
    if (currentTime <= *book.getDueDate()) {
        return 0;
    }

    long long overdueDays = util::daysBetween(*book.getDueDate(), currentTime);
    return std::max(1LL, overdueDays);
}

std::string Library::statusLabel(const Book& book) const {
    if (book.isBorrowed() && book.hasReservation()) {
        return "Borrowed (Reserved)";
    }

    if (book.isBorrowed()) {
        return "Borrowed";
    }

    if (book.hasReservation()) {
        return "Reserved";
    }

    return "Available";
}

void Library::printBookDetails(const Book& book) const {
    std::cout << "Book ID: " << book.getBookID() << '\n';
    std::cout << "Title: " << book.getTitle() << '\n';
    std::cout << "Author: " << book.getAuthor() << '\n';
    std::cout << "Status: " << statusLabel(book) << '\n';

    if (book.isBorrowed()) {
        std::cout << "Borrowed by member: " << book.getBorrowedByUserID() << '\n';
    }

    if (book.getDueDate()) {
        std::cout << "Due date: " << util::formatDate(*book.getDueDate()) << '\n';
    }

    if (book.hasReservation()) {
        std::cout << "Reserved by member: " << book.getReservedByUserID() << '\n';
        if (book.getStatus() == BookStatus::Reserved && book.getReservationDate()) {
            util::TimePoint deadline = util::addDays(*book.getReservationDate(), reservationExpiryDays);
            std::cout << "Collection deadline: " << util::formatDate(deadline) << '\n';
        } else if (book.isBorrowed()) {
            std::cout << "Reservation status: Waiting for the current borrower to return the book.\n";
        }
    }

    std::cout << '\n';
}

void Library::showSearchResults(const std::vector<const Book*>& results) const {
    if (results.empty()) {
        std::cout << "No matching books found.\n";
        return;
    }

    for (const Book* book : results) {
        printBookDetails(*book);
    }
}

void Library::removeBookFromAllUserLists(const std::string& bookID) {
    for (auto& user : users) {
        user.removeBorrowedBook(bookID);
        user.removeReservedBook(bookID);
    }
}

std::vector<std::string> Library::buildLiveNotifications(const User& user) const {
    std::vector<std::string> notifications;

    for (const std::string& bookID : user.getBorrowedBooks()) {
        const Book* book = findBookByID(bookID);
        if (book == nullptr || !book->getDueDate().has_value()) {
            continue;
        }

        long long overdueDays = getOverdueDays(*book);
        if (overdueDays > 0) {
            notifications.push_back(
                "Overdue book: \"" + book->getTitle() + "\" was due on " +
                util::formatDate(*book->getDueDate()) + " (" +
                std::to_string(overdueDays) + " day(s) overdue)."
            );
            continue;
        }

        long long daysLeft = util::daysBetween(util::now(), *book->getDueDate());
        if (daysLeft >= 0 && daysLeft <= 2) {
            notifications.push_back(
                "Due soon: \"" + book->getTitle() + "\" is due on " +
                util::formatDate(*book->getDueDate()) + "."
            );
        }
    }

    return notifications;
}

void Library::expireReservations() {
    const util::TimePoint currentTime = util::now();

    for (auto& book : books) {
        if (!book.hasReservation() || book.isBorrowed() || !book.getReservationDate().has_value()) {
            continue;
        }

        long long reservedForDays = util::daysBetween(*book.getReservationDate(), currentTime);
        if (reservedForDays < reservationExpiryDays) {
            continue;
        }

        const std::string reservedUserID = book.getReservedByUserID();
        const std::string bookID = book.getBookID();
        const std::string title = book.getTitle();

        if (User* user = findUserByID(reservedUserID)) {
            user->removeReservedBook(bookID);
        }

        book.clearReservation();

        addNotification(
            "Reservation expired for \"" + title + "\" because it was not collected within " +
            std::to_string(reservationExpiryDays) + " days.",
            reservedUserID
        );
        addNotification("Reservation expired: " + bookID + " is available again.");
    }
}

void Library::loadSampleData() {
    administrators.emplace_back("A001", "System Admin", "admin123");
    librarians.emplace_back("L001", "Main Librarian", "lib123");
    users.emplace_back("U001", "Josh Gardener", "user123");
    users.emplace_back("U002", "John Bob", "user123");

    books.emplace_back("B001", "Diary of Wimpy Kid", "Jeff Kinney");
    books.emplace_back("B002", "Atomic Habits", "James Clear");
    books.emplace_back("B003", "Guinness Book of Records 2026", "Norris and Ross McWhirter");
    books.emplace_back("B004", "Let Them", "Mel Robbins");
    books.emplace_back("B005", "Love Letters to a Serial Killer", "Tasha Coryell");
}

bool Library::addBookRecord(const std::string& bookID, const std::string& title, const std::string& author) {
    if (bookID.empty() || title.empty() || author.empty() || findBookByID(bookID) != nullptr) {
        return false;
    }

    books.emplace_back(bookID, title, author);
    addNotification("Book added to the catalogue: " + bookID + " - " + title + ".");
    return true;
}

bool Library::updateBookRecord(const std::string& bookID, const std::string& title, const std::string& author) {
    Book* book = findBookByID(bookID);
    if (book == nullptr || title.empty() || author.empty()) {
        return false;
    }

    book->setTitle(title);
    book->setAuthor(author);
    addNotification("Book updated: " + bookID + " now stores the latest title and author details.");
    return true;
}

bool Library::removeBookRecord(const std::string& bookID) {
    Book* book = findBookByID(bookID);
    if (book == nullptr || book->isBorrowed() || book->hasReservation()) {
        return false;
    }

    removeBookFromAllUserLists(bookID);

    auto it = std::remove_if(books.begin(), books.end(), [&](const Book& currentBook) {
        return currentBook.getBookID() == bookID;
    });
    books.erase(it, books.end());

    addNotification("Book removed from the catalogue: " + bookID + ".");
    return true;
}

std::vector<const Book*> Library::searchBooksByTitle(const std::string& title) const {
    std::vector<const Book*> results;
    for (const auto& book : books) {
        if (util::containsIgnoreCase(book.getTitle(), title)) {
            results.push_back(&book);
        }
    }
    return results;
}

std::vector<const Book*> Library::searchBooksByAuthor(const std::string& author) const {
    std::vector<const Book*> results;
    for (const auto& book : books) {
        if (util::containsIgnoreCase(book.getAuthor(), author)) {
            results.push_back(&book);
        }
    }
    return results;
}

void Library::checkAvailability(const std::string& bookID) const {
    const Book* book = findBookByID(bookID);
    if (book == nullptr) {
        std::cout << "Book not found.\n";
        return;
    }

    printBookDetails(*book);
}

bool Library::processBorrowRequest(const std::string& userID, const std::string& bookID) {
    expireReservations();

    User* user = findUserByID(userID);
    Book* book = findBookByID(bookID);

    if (user == nullptr) {
        std::cout << "Member account not found.\n";
        return false;
    }

    if (book == nullptr) {
        std::cout << "Book not found.\n";
        return false;
    }

    if (user->hasBorrowedBook(bookID)) {
        std::cout << "You already borrowed this book.\n";
        return false;
    }

    if (user->borrowedCount() >= borrowingLimit) {
        std::cout << "Borrowing limit reached. Members cannot borrow more than "
                  << borrowingLimit << " books at a time.\n";
        return false;
    }

    if (book->isBorrowed()) {
        std::cout << "Book is currently borrowed.\n";
        return false;
    }

    if (book->hasReservation() && !book->isReservedFor(userID)) {
        std::cout << "Book is reserved for another member and cannot be borrowed right now.\n";
        return false;
    }

    util::TimePoint dueDate = util::addDays(util::now(), borrowingDays);
    const std::string title = book->getTitle();
    book->markBorrowed(userID, dueDate);
    user->addBorrowedBook(bookID);
    user->removeReservedBook(bookID);

    addNotification(
        "Borrow confirmed: \"" + title + "\" is due on " + util::formatDate(dueDate) + ".",
        userID
    );
    addNotification("Borrow recorded: " + userID + " borrowed " + bookID + ".");

    std::cout << "Book borrowed successfully.\n";
    std::cout << "Due date: " << util::formatDate(dueDate) << '\n';
    return true;
}

bool Library::returnBook(const std::string& userID, const std::string& bookID) {
    expireReservations();

    User* user = findUserByID(userID);
    Book* book = findBookByID(bookID);

    if (user == nullptr || book == nullptr) {
        std::cout << "User or book not found.\n";
        return false;
    }

    if (book->getBorrowedByUserID() != userID) {
        std::cout << "This book is not currently borrowed by this member.\n";
        return false;
    }

    double penalty = 0.0;
    long long overdueDays = getOverdueDays(*book);
    if (overdueDays > 0) {
        penalty = overdueDays * latePenaltyPerDay;
        user->addPenalty(penalty);
    }

    const util::TimePoint currentTime = util::now();
    const bool hasReservation = book->hasReservation();
    const std::string reservedUserID = book->getReservedByUserID();
    const std::string title = book->getTitle();

    user->removeBorrowedBook(bookID);
    book->clearBorrow();

    addNotification("Return recorded for \"" + title + "\".", userID);
    addNotification("Return recorded: " + userID + " returned " + bookID + ".");

    std::cout << "Book returned successfully.\n";
    if (penalty > 0.0) {
        std::cout << "Late return penalty added: $" << util::formatMoney(penalty) << '\n';
    }

    if (hasReservation && !reservedUserID.empty()) {
        book->setReservationReadyDate(currentTime);

        if (User* reservedUser = findUserByID(reservedUserID)) {
            reservedUser->addReservedBook(bookID);
        }

        const std::string deadline = util::formatDate(util::addDays(currentTime, reservationExpiryDays));
        addNotification(
            "Reserved book available: \"" + title + "\" is ready for collection until " + deadline + ".",
            reservedUserID
        );
        addNotification(
            "Reserved book available: " + bookID + " is now waiting for member " + reservedUserID + "."
        );

        std::cout << "This book is now reserved for member " << reservedUserID
                  << " until " << deadline << ".\n";
    }

    return true;
}

bool Library::reserveBook(const std::string& userID, const std::string& bookID) {
    expireReservations();

    User* user = findUserByID(userID);
    Book* book = findBookByID(bookID);

    if (user == nullptr || book == nullptr) {
        std::cout << "User or book not found.\n";
        return false;
    }

    if (book->getBorrowedByUserID() == userID) {
        std::cout << "You already borrowed this book.\n";
        return false;
    }

    if (!book->isBorrowed()) {
        if (book->hasReservation() && book->isReservedFor(userID)) {
            std::cout << "This book is already reserved for you and ready for collection.\n";
        } else if (book->hasReservation()) {
            std::cout << "Book is already reserved by another member.\n";
        } else {
            std::cout << "Book is available now. Borrow it instead of reserving it.\n";
        }
        return false;
    }

    if (book->hasReservation()) {
        if (book->isReservedFor(userID)) {
            std::cout << "You have already reserved this book.\n";
        } else {
            std::cout << "Book is already reserved by another member.\n";
        }
        return false;
    }

    const std::string title = book->getTitle();
    book->markReserved(userID, util::now());
    user->addReservedBook(bookID);

    addNotification(
        "Reservation created for \"" + title + "\". You will be notified when it becomes available.",
        userID
    );
    addNotification("Reservation created: " + userID + " reserved " + bookID + ".");

    std::cout << "Book reserved successfully.\n";
    std::cout << "The reservation will expire if it is not collected within "
              << reservationExpiryDays << " days after the book becomes available.\n";
    return true;
}

bool Library::addUserAccount(const std::string& userID, const std::string& name, const std::string& password) {
    if (userID.empty() || name.empty() || password.empty() || accountIDExists(userID)) {
        return false;
    }

    users.emplace_back(userID, name, password);
    addNotification("Member account created: " + userID + " - " + name + ".");
    return true;
}

bool Library::updateUserAccount(const std::string& userID, const std::string& name, const std::string& password) {
    User* user = findUserByID(userID);
    if (user == nullptr || name.empty() || password.empty()) {
        return false;
    }

    user->setName(name);
    user->setPassword(password);
    addNotification("Member account updated: " + userID + ".");
    return true;
}

bool Library::removeUserAccount(const std::string& userID) {
    User* user = findUserByID(userID);
    if (user == nullptr) {
        return false;
    }

    if (!user->getBorrowedBooks().empty() || !user->getReservedBooks().empty()) {
        return false;
    }

    auto it = std::remove_if(users.begin(), users.end(), [&](const User& currentUser) {
        return currentUser.getID() == userID;
    });
    users.erase(it, users.end());

    addNotification("Member account removed: " + userID + ".");
    return true;
}

bool Library::addLibrarianAccount(const std::string& librarianID, const std::string& name, const std::string& password) {
    if (librarianID.empty() || name.empty() || password.empty() || accountIDExists(librarianID)) {
        return false;
    }

    librarians.emplace_back(librarianID, name, password);
    addNotification("Librarian account created: " + librarianID + " - " + name + ".");
    return true;
}

bool Library::updateLibrarianAccount(const std::string& librarianID, const std::string& name, const std::string& password) {
    Librarian* librarian = findLibrarianByID(librarianID);
    if (librarian == nullptr || name.empty() || password.empty()) {
        return false;
    }

    librarian->setName(name);
    librarian->setPassword(password);
    addNotification("Librarian account updated: " + librarianID + ".");
    return true;
}

bool Library::removeLibrarianAccount(const std::string& librarianID) {
    auto it = std::remove_if(librarians.begin(), librarians.end(), [&](const Librarian& currentLibrarian) {
        return currentLibrarian.getID() == librarianID;
    });

    if (it == librarians.end()) {
        return false;
    }

    librarians.erase(it, librarians.end());
    addNotification("Librarian account removed: " + librarianID + ".");
    return true;
}

void Library::setBorrowingLimit(int limit) {
    borrowingLimit = std::clamp(limit, 1, hardBorrowingCap);
}

void Library::setLatePenalty(double penalty) {
    if (penalty >= 0.0) {
        latePenaltyPerDay = penalty;
    }
}

User* Library::loginUser(const std::string& userID, const std::string& password) {
    User* user = findUserByID(userID);
    return (user != nullptr && user->login(userID, password)) ? user : nullptr;
}

Librarian* Library::loginLibrarian(const std::string& librarianID, const std::string& password) {
    Librarian* librarian = findLibrarianByID(librarianID);
    return (librarian != nullptr && librarian->login(librarianID, password)) ? librarian : nullptr;
}

Administrator* Library::loginAdministrator(const std::string& adminID, const std::string& password) {
    Administrator* administrator = findAdministratorByID(adminID);
    return (administrator != nullptr && administrator->login(adminID, password)) ? administrator : nullptr;
}

void Library::showAllBooks() const {
    if (books.empty()) {
        std::cout << "No books in the system.\n";
        return;
    }

    for (const auto& book : books) {
        printBookDetails(book);
    }
}

void Library::showUserBorrowingInfo(const std::string& userID) {
    expireReservations();

    const User* user = findUserByID(userID);
    if (user == nullptr) {
        std::cout << "Member not found.\n";
        return;
    }

    std::cout << "User ID: " << user->getID() << '\n';
    std::cout << "Name: " << user->getName() << '\n';
    std::cout << "Role: " << user->roleName() << '\n';
    std::cout << "Borrowed books: " << user->getBorrowedBooks().size() << '\n';
    std::cout << "Reserved books: " << user->getReservedBooks().size() << '\n';
    std::cout << "Outstanding penalties: $" << util::formatMoney(user->getOutstandingPenalty()) << "\n\n";

    std::cout << "Currently borrowed books:\n";
    if (user->getBorrowedBooks().empty()) {
        std::cout << "None\n";
    } else {
        for (const std::string& bookID : user->getBorrowedBooks()) {
            const Book* book = findBookByID(bookID);
            if (book == nullptr) {
                continue;
            }

            std::cout << "- " << book->getTitle() << " (" << book->getBookID() << ")";
            if (book->getDueDate()) {
                std::cout << " | Due: " << util::formatDate(*book->getDueDate());
            }
            std::cout << '\n';
        }
    }

    std::cout << "\nReserved books:\n";
    if (user->getReservedBooks().empty()) {
        std::cout << "None\n";
    } else {
        for (const std::string& bookID : user->getReservedBooks()) {
            const Book* book = findBookByID(bookID);
            if (book == nullptr) {
                continue;
            }

            std::cout << "- " << book->getTitle() << " (" << book->getBookID() << ")";
            if (book->getStatus() == BookStatus::Reserved && book->getReservationDate()) {
                util::TimePoint deadline = util::addDays(*book->getReservationDate(), reservationExpiryDays);
                std::cout << " | Ready to collect until: " << util::formatDate(deadline);
            } else {
                std::cout << " | Waiting for return";
            }
            std::cout << '\n';
        }
    }

    std::cout << "\nOverdue books:\n";
    bool hasOverdueBooks = false;
    for (const std::string& bookID : user->getBorrowedBooks()) {
        const Book* book = findBookByID(bookID);
        if (book == nullptr || !book->getDueDate().has_value()) {
            continue;
        }

        long long overdueDays = getOverdueDays(*book);
        if (overdueDays <= 0) {
            continue;
        }

        hasOverdueBooks = true;
        std::cout << "- " << book->getTitle() << " (" << book->getBookID() << ")"
                  << " | Due: " << util::formatDate(*book->getDueDate())
                  << " | Overdue by: " << overdueDays << " day(s)\n";
    }

    if (!hasOverdueBooks) {
        std::cout << "None\n";
    }
}

void Library::showMemberNotifications(const User& user) {
    expireReservations();

    std::vector<std::string> liveNotifications = buildLiveNotifications(user);
    std::vector<std::string> shownNotifications;
    std::size_t counter = 1;

    for (const std::string& message : liveNotifications) {
        if (std::find(shownNotifications.begin(), shownNotifications.end(), message) == shownNotifications.end()) {
            shownNotifications.push_back(message);
            std::cout << counter++ << ". " << message << '\n';
        }
    }

    for (const auto& notification : notificationLog) {
        if (notification.recipientID == user.getID()) {
            std::string formatted = "[" + util::formatDate(notification.createdAt) + "] " + notification.message;
            if (std::find(shownNotifications.begin(), shownNotifications.end(), formatted) == shownNotifications.end() &&
                std::find(shownNotifications.begin(), shownNotifications.end(), notification.message) == shownNotifications.end()) {
                shownNotifications.push_back(formatted);
                std::cout << counter++ << ". " << formatted << '\n';
            }
        }
    }

    if (counter == 1) {
        std::cout << "No notifications available.\n";
    }
}

void Library::showSystemNotifications() {
    expireReservations();

    if (notificationLog.empty()) {
        std::cout << "No notifications available.\n";
        return;
    }

    for (std::size_t i = 0; i < notificationLog.size(); ++i) {
        const auto& notification = notificationLog[i];
        std::cout << i + 1 << ". "
                  << "[" << util::formatDate(notification.createdAt) << "] ";

        if (!notification.recipientID.empty()) {
            std::cout << "To " << notification.recipientID << ": ";
        }

        std::cout << notification.message << '\n';
    }
}

void Library::generateLibraryReport() const {
    int availableBooks = 0;
    int borrowedBooks = 0;
    int reservedBooks = 0;
    int activeReservationRequests = 0;
    int overdueBooks = 0;
    double totalPenaltyBalance = 0.0;

    for (const auto& book : books) {
        if (book.getStatus() == BookStatus::Available) {
            ++availableBooks;
        } else if (book.getStatus() == BookStatus::Borrowed) {
            ++borrowedBooks;
        } else if (book.getStatus() == BookStatus::Reserved) {
            ++reservedBooks;
        }

        if (book.hasReservation()) {
            ++activeReservationRequests;
        }

        if (getOverdueDays(book) > 0) {
            ++overdueBooks;
        }
    }

    for (const auto& user : users) {
        totalPenaltyBalance += user.getOutstandingPenalty();
    }

    std::cout << "Library Summary Report\n";
    std::cout << "Total books: " << books.size() << '\n';
    std::cout << "Available books: " << availableBooks << '\n';
    std::cout << "Borrowed books: " << borrowedBooks << '\n';
    std::cout << "Reserved books ready for collection: " << reservedBooks << '\n';
    std::cout << "Active reservation requests: " << activeReservationRequests << '\n';
    std::cout << "Overdue books: " << overdueBooks << '\n';
    std::cout << "Members: " << users.size() << '\n';
    std::cout << "Librarians: " << librarians.size() << '\n';
    std::cout << "Administrators: " << administrators.size() << '\n';
    std::cout << "Maximum borrowing limit: " << borrowingLimit << '\n';
    std::cout << "Borrowing period: " << borrowingDays << " days\n";
    std::cout << "Reservation collection period: " << reservationExpiryDays << " days\n";
    std::cout << "Late penalty per day: $" << util::formatMoney(latePenaltyPerDay) << '\n';
    std::cout << "Outstanding penalty balance: $" << util::formatMoney(totalPenaltyBalance) << '\n';
}

void Library::generateOverdueReport() const {
    bool foundOverdueBook = false;

    std::cout << "Overdue Books Report\n";
    for (const auto& book : books) {
        long long overdueDays = getOverdueDays(book);
        if (overdueDays <= 0) {
            continue;
        }

        foundOverdueBook = true;
        std::cout << "Book ID: " << book.getBookID() << '\n';
        std::cout << "Title: " << book.getTitle() << '\n';
        std::cout << "Borrower: " << book.getBorrowedByUserID() << '\n';
        std::cout << "Due date: " << util::formatDate(*book.getDueDate()) << '\n';
        std::cout << "Days overdue: " << overdueDays << "\n\n";
    }

    if (!foundOverdueBook) {
        std::cout << "No overdue books found.\n";
    }
}

void Library::generateMemberActivityReport() const {
    std::cout << "Member Activity Report\n";
    if (users.empty()) {
        std::cout << "No members found.\n";
        return;
    }

    for (const auto& user : users) {
        int overdueCount = 0;
        for (const std::string& bookID : user.getBorrowedBooks()) {
            const Book* book = findBookByID(bookID);
            if (book != nullptr && getOverdueDays(*book) > 0) {
                ++overdueCount;
            }
        }

        std::cout << user.getID() << " - " << user.getName()
                  << " | Borrowed: " << user.getBorrowedBooks().size()
                  << " | Reserved: " << user.getReservedBooks().size()
                  << " | Overdue: " << overdueCount
                  << " | Penalties: $" << util::formatMoney(user.getOutstandingPenalty())
                  << '\n';
    }
}

void Library::showRules() const {
    std::cout << "Current Library Rules\n";
    std::cout << "Maximum borrowing limit: " << borrowingLimit << " books\n";
    std::cout << "Hard maximum allowed by the system: " << hardBorrowingCap << " books\n";
    std::cout << "Borrowing period: " << borrowingDays << " days\n";
    std::cout << "Reservation collection period: " << reservationExpiryDays << " days\n";
    std::cout << "Late return penalty per day: $" << util::formatMoney(latePenaltyPerDay) << '\n';
}

void Library::listAccounts() const {
    std::cout << "Members:\n";
    if (users.empty()) {
        std::cout << "None\n";
    } else {
        for (const auto& user : users) {
            std::cout << user.getID() << " - " << user.getName()
                      << " - " << user.roleName() << '\n';
        }
    }

    std::cout << "\nLibrarians:\n";
    if (librarians.empty()) {
        std::cout << "None\n";
    } else {
        for (const auto& librarian : librarians) {
            std::cout << librarian.getID() << " - " << librarian.getName()
                      << " - " << librarian.roleName() << '\n';
        }
    }

    std::cout << "\nAdministrators:\n";
    if (administrators.empty()) {
        std::cout << "None\n";
    } else {
        for (const auto& administrator : administrators) {
            std::cout << administrator.getID() << " - " << administrator.getName()
                      << " - " << administrator.roleName() << '\n';
        }
    }
}

void Library::userMenu(User& currentUser) {
    showAccountIntro(currentUser);

    while (true) {
        expireReservations();

        std::cout << "\n1. View borrowing information\n";
        std::cout << "2. Search book by title\n";
        std::cout << "3. Search book by author\n";
        std::cout << "4. Check book availability\n";
        std::cout << "5. Borrow book\n";
        std::cout << "6. Return book\n";
        std::cout << "7. Reserve book\n";
        std::cout << "8. View notifications\n";
        std::cout << "9. View all books\n";
        std::cout << "10. Logout\n";

        int choice = util::readInt("Choose an option: ", 1, 10);

        if (choice == 1) {
            showUserBorrowingInfo(currentUser.getID());
        } else if (choice == 2) {
            showSearchResults(searchBooksByTitle(util::readNonEmptyLine("Enter title keyword: ")));
        } else if (choice == 3) {
            showSearchResults(searchBooksByAuthor(util::readNonEmptyLine("Enter author keyword: ")));
        } else if (choice == 4) {
            checkAvailability(util::readNonEmptyLine("Enter book ID: "));
        } else if (choice == 5) {
            processBorrowRequest(currentUser.getID(), util::readNonEmptyLine("Enter book ID to borrow: "));
        } else if (choice == 6) {
            returnBook(currentUser.getID(), util::readNonEmptyLine("Enter book ID to return: "));
        } else if (choice == 7) {
            reserveBook(currentUser.getID(), util::readNonEmptyLine("Enter book ID to reserve: "));
        } else if (choice == 8) {
            showMemberNotifications(currentUser);
        } else if (choice == 9) {
            showAllBooks();
        } else {
            return;
        }
    }
}

void Library::librarianMenu(Librarian& currentLibrarian) {
    showAccountIntro(currentLibrarian);

    while (true) {
        expireReservations();

        std::cout << "\n1. Add new book\n";
        std::cout << "2. Update book\n";
        std::cout << "3. Remove book\n";
        std::cout << "4. View all books\n";
        std::cout << "5. Handle borrow request\n";
        std::cout << "6. Generate library report\n";
        std::cout << "7. Generate overdue report\n";
        std::cout << "8. Generate member activity report\n";
        std::cout << "9. View notifications\n";
        std::cout << "10. Logout\n";

        int choice = util::readInt("Choose an option: ", 1, 10);

        if (choice == 1) {
            std::string id = util::readNonEmptyLine("Enter new book ID: ");
            std::string title = util::readNonEmptyLine("Enter title: ");
            std::string author = util::readNonEmptyLine("Enter author: ");

            if (addBookRecord(id, title, author)) {
                std::cout << "Book added successfully.\n";
            } else {
                std::cout << "A book with that ID already exists, or the details were invalid.\n";
            }
        } else if (choice == 2) {
            std::string id = util::readNonEmptyLine("Enter book ID to update: ");
            std::string title = util::readNonEmptyLine("Enter new title: ");
            std::string author = util::readNonEmptyLine("Enter new author: ");

            if (updateBookRecord(id, title, author)) {
                std::cout << "Book updated successfully.\n";
            } else {
                std::cout << "Book not found or the new details were invalid.\n";
            }
        } else if (choice == 3) {
            std::string id = util::readNonEmptyLine("Enter book ID to remove: ");
            if (removeBookRecord(id)) {
                std::cout << "Book removed successfully.\n";
            } else {
                std::cout << "Book could not be removed. It may not exist, may be borrowed, or may be reserved.\n";
            }
        } else if (choice == 4) {
            showAllBooks();
        } else if (choice == 5) {
            std::string userID = util::readNonEmptyLine("Enter member ID: ");
            std::string bookID = util::readNonEmptyLine("Enter book ID: ");
            processBorrowRequest(userID, bookID);
        } else if (choice == 6) {
            generateLibraryReport();
        } else if (choice == 7) {
            generateOverdueReport();
        } else if (choice == 8) {
            generateMemberActivityReport();
        } else if (choice == 9) {
            showSystemNotifications();
        } else {
            return;
        }
    }
}

void Library::administratorMenu(Administrator& currentAdministrator) {
    showAccountIntro(currentAdministrator);

    while (true) {
        expireReservations();

        std::cout << "\n1. Create member account\n";
        std::cout << "2. Update member account\n";
        std::cout << "3. Remove member account\n";
        std::cout << "4. Create librarian account\n";
        std::cout << "5. Update librarian account\n";
        std::cout << "6. Remove librarian account\n";
        std::cout << "7. List all accounts\n";
        std::cout << "8. Set maximum borrowing limit\n";
        std::cout << "9. Set late return penalty\n";
        std::cout << "10. View library report\n";
        std::cout << "11. View overdue report\n";
        std::cout << "12. View member activity report\n";
        std::cout << "13. View library rules\n";
        std::cout << "14. View notifications\n";
        std::cout << "15. Logout\n";

        int choice = util::readInt("Choose an option: ", 1, 15);

        if (choice == 1) {
            std::string id = util::readNonEmptyLine("Enter member ID: ");
            std::string name = util::readNonEmptyLine("Enter member name: ");
            std::string password = util::readNonEmptyLine("Enter password: ");

            if (addUserAccount(id, name, password)) {
                std::cout << "Member account created successfully.\n";
            } else {
                std::cout << "That ID is already in use, or the details were invalid.\n";
            }
        } else if (choice == 2) {
            std::string id = util::readNonEmptyLine("Enter member ID to update: ");
            std::string name = util::readNonEmptyLine("Enter new member name: ");
            std::string password = util::readNonEmptyLine("Enter new password: ");

            if (updateUserAccount(id, name, password)) {
                std::cout << "Member account updated successfully.\n";
            } else {
                std::cout << "Member account not found or the new details were invalid.\n";
            }
        } else if (choice == 3) {
            std::string id = util::readNonEmptyLine("Enter member ID to remove: ");

            if (removeUserAccount(id)) {
                std::cout << "Member account removed successfully.\n";
            } else {
                std::cout << "Member account could not be removed. It may not exist or still have borrowed/reserved books.\n";
            }
        } else if (choice == 4) {
            std::string id = util::readNonEmptyLine("Enter librarian ID: ");
            std::string name = util::readNonEmptyLine("Enter librarian name: ");
            std::string password = util::readNonEmptyLine("Enter password: ");

            if (addLibrarianAccount(id, name, password)) {
                std::cout << "Librarian account created successfully.\n";
            } else {
                std::cout << "That ID is already in use, or the details were invalid.\n";
            }
        } else if (choice == 5) {
            std::string id = util::readNonEmptyLine("Enter librarian ID to update: ");
            std::string name = util::readNonEmptyLine("Enter new librarian name: ");
            std::string password = util::readNonEmptyLine("Enter new password: ");

            if (updateLibrarianAccount(id, name, password)) {
                std::cout << "Librarian account updated successfully.\n";
            } else {
                std::cout << "Librarian account not found or the new details were invalid.\n";
            }
        } else if (choice == 6) {
            std::string id = util::readNonEmptyLine("Enter librarian ID to remove: ");

            if (removeLibrarianAccount(id)) {
                std::cout << "Librarian account removed successfully.\n";
            } else {
                std::cout << "Librarian account not found.\n";
            }
        } else if (choice == 7) {
            listAccounts();
        } else if (choice == 8) {
            int limit = util::readInt("Enter new borrowing limit (1-5): ", 1, hardBorrowingCap);
            setBorrowingLimit(limit);
            std::cout << "Borrowing limit updated.\n";
        } else if (choice == 9) {
            setLatePenalty(util::readDouble("Enter late penalty per day: ", 0.0));
            std::cout << "Late penalty updated.\n";
        } else if (choice == 10) {
            generateLibraryReport();
        } else if (choice == 11) {
            generateOverdueReport();
        } else if (choice == 12) {
            generateMemberActivityReport();
        } else if (choice == 13) {
            showRules();
        } else if (choice == 14) {
            showSystemNotifications();
        } else {
            return;
        }
    }
}

void Library::mainMenu() {
    while (true) {
        expireReservations();

        std::cout << "\n1. Login as Member\n";
        std::cout << "2. Login as Librarian\n";
        std::cout << "3. Login as Administrator\n";
        std::cout << "4. Search book by title\n";
        std::cout << "5. Search book by author\n";
        std::cout << "6. Check book availability\n";
        std::cout << "7. View all books\n";
        std::cout << "8. Exit\n";

        int choice = util::readInt("Choose an option: ", 1, 8);

        if (choice == 1) {
            std::string id = util::readNonEmptyLine("Enter member ID: ");
            std::string password = util::readNonEmptyLine("Enter password: ");

            User* user = loginUser(id, password);
            if (user != nullptr) {
                userMenu(*user);
            } else {
                std::cout << "Invalid member ID or password.\n";
            }
        } else if (choice == 2) {
            std::string id = util::readNonEmptyLine("Enter librarian ID: ");
            std::string password = util::readNonEmptyLine("Enter password: ");

            Librarian* librarian = loginLibrarian(id, password);
            if (librarian != nullptr) {
                librarianMenu(*librarian);
            } else {
                std::cout << "Invalid librarian ID or password.\n";
            }
        } else if (choice == 3) {
            std::string id = util::readNonEmptyLine("Enter administrator ID: ");
            std::string password = util::readNonEmptyLine("Enter password: ");

            Administrator* administrator = loginAdministrator(id, password);
            if (administrator != nullptr) {
                administratorMenu(*administrator);
            } else {
                std::cout << "Invalid administrator ID or password.\n";
            }
        } else if (choice == 4) {
            showSearchResults(searchBooksByTitle(util::readNonEmptyLine("Enter title keyword: ")));
        } else if (choice == 5) {
            showSearchResults(searchBooksByAuthor(util::readNonEmptyLine("Enter author keyword: ")));
        } else if (choice == 6) {
            checkAvailability(util::readNonEmptyLine("Enter book ID: "));
        } else if (choice == 7) {
            showAllBooks();
        } else {
            std::cout << "Goodbye.\n";
            return;
        }
    }
}

int main() {
    Library library;
    library.loadSampleData();

    std::cout << "Smart Library System\n";
    std::cout << "System setup: 1 administrator, 1 librarian, and 2 members\n";
    std::cout << "Sample member logins: U001 / user123 and U002 / user123\n";
    std::cout << "Members: U001 - Josh Gardener, U002 - John Bob\n";
    std::cout << "Sample librarian login: L001 / lib123\n";
    std::cout << "Librarian: L001 - Main Librarian\n";
    std::cout << "Sample administrator login: A001 / admin123\n";
    std::cout << "Administrator: A001 - System Admin\n";
    std::cout << '\n';

    library.mainMenu();
    return 0;
}
