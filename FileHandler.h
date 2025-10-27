#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
using namespace std;
// ------------------ Core Structs ------------------
struct Booking {
    string id;
    string name;
    string vehicleNo;
    string date;
    string serviceType;
};

struct Customer {
    string name;
    string contact;
    string vehicleNo;
    Customer* next;
};

// ------------------ Circular Queue ------------------
class CircularQueue {
    vector<Booking> arr;
    int front, rear, count, capacity;
public:
    CircularQueue(int size = 5) {
        capacity = size;
        arr.resize(size);
        front = rear = count = 0;
    }
    bool isFull() const { return count == capacity; }
    bool isEmpty() const { return count == 0; }
    bool enqueue(const Booking &b) {
        if (isFull()) return false;
        arr[rear] = b;
        rear = (rear + 1) % capacity;
        count++;
        return true;
    }
    bool dequeue() {
        if (isEmpty()) return false;
        front = (front + 1) % capacity;
        count--;
        return true;
    }
    Booking frontItem() const { return arr[front]; }
    int size() const { return count; }
};

// ------------------ FileHandler Namespace ------------------
namespace FileHandler {
    // Constants
    const int MAX_SLOTS_PER_DATE = 5;

    // Global Maps
    extern unordered_map<string, CircularQueue> dateQueues; // date → bookings
    extern unordered_map<string, vector<Booking>> vehicleMap; // vehicle → bookings

    // --- Helpers ---
    string urlDecode(const string &src);
    vector<pair<string, string>> parsePost(const string &post);
    string genBookingID();
    string genCustomerID();

    // --- Booking operations ---
    bool appendBooking(const Booking &b);
    vector<Booking> loadBookings();
    bool removeBookingByID(const string &bid, Booking &removed);

    // --- Cancellations (Stack) ---
    bool appendCancellation(const Booking &b);
    bool popLastCancellation(Booking &b);

    // --- Customer Linked List ---
    bool appendCustomer(const Customer &c);
    vector<Customer> loadCustomers();
    bool isVehicleRegistered(const string &vehicleNo);

    // --- New helper functions ---
    void loadAll();
    vector<Booking> sortBookingsByDate(vector<Booking> &v);
    Booking* searchByVehicleNo(const string &veh);
}

#endif
