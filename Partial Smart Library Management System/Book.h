#ifndef BOOK_H
#define BOOK_H

#include <chrono>
#include <optional>
#include <string>

namespace util {
    using TimePoint = std::chrono::system_clock::time_point;
}

enum class BookStatus {
    Available,
    Borrowed,
    Reserved
};

inline std::string toString(BookStatus status) {
    switch (status) {
        case BookStatus::Available: return "Available";
        case BookStatus::Borrowed:  return "Borrowed";
        case BookStatus::Reserved:  return "Reserved";
        default:                    return "Unknown";
    }
}

class Book {
private:
    std::string bookID;
    std::string title;
    std::string author;
    BookStatus status = BookStatus::Available;
    std::optional<util::TimePoint> dueDate;
    std::optional<util::TimePoint> reservationDate;
    std::string borrowedByUserID;
    std::string reservedByUserID;

public:
    Book() = default;

    Book(const std::string& id, const std::string& bookTitle, const std::string& bookAuthor)
        : bookID(id), title(bookTitle), author(bookAuthor) {}

    const std::string& getBookID() const { return bookID; }
    const std::string& getTitle() const { return title; }
    const std::string& getAuthor() const { return author; }
    BookStatus getStatus() const { return status; }
    const std::string& getBorrowedByUserID() const { return borrowedByUserID; }
    const std::string& getReservedByUserID() const { return reservedByUserID; }
    std::optional<util::TimePoint> getDueDate() const { return dueDate; }
    std::optional<util::TimePoint> getReservationDate() const { return reservationDate; }

    void setTitle(const std::string& newTitle) { title = newTitle; }
    void setAuthor(const std::string& newAuthor) { author = newAuthor; }

    bool isAvailable() const { return status == BookStatus::Available; }
    bool isBorrowed() const { return !borrowedByUserID.empty(); }
    bool hasReservation() const { return !reservedByUserID.empty(); }
    bool isReservedFor(const std::string& userID) const { return reservedByUserID == userID; }

    void markBorrowed(const std::string& userID, const util::TimePoint& due) {
        borrowedByUserID = userID;
        dueDate = due;
        reservedByUserID.clear();
        reservationDate.reset();
        refreshStatus();
    }

    void markReserved(const std::string& userID, const util::TimePoint& reservedOn) {
        reservedByUserID = userID;
        reservationDate = reservedOn;
        refreshStatus();
    }

    void setReservationReadyDate(const util::TimePoint& readyOn) {
        reservationDate = readyOn;
        refreshStatus();
    }

    void clearBorrow() {
        borrowedByUserID.clear();
        dueDate.reset();
        refreshStatus();
    }

    void clearReservation() {
        reservedByUserID.clear();
        reservationDate.reset();
        refreshStatus();
    }

    void refreshStatus() {
        if (isBorrowed()) {
            status = BookStatus::Borrowed;
        } else if (hasReservation()) {
            status = BookStatus::Reserved;
        } else {
            status = BookStatus::Available;
        }
    }

    void reset() {
        borrowedByUserID.clear();
        reservedByUserID.clear();
        dueDate.reset();
        reservationDate.reset();
        status = BookStatus::Available;
    }
};

#endif
