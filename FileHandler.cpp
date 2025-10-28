#include "FileHandler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <ctime>

using namespace std;
// --- Define the global variables declared in FileHandler.h ---
unordered_map<string, CircularQueue> FileHandler::dateQueues;
unordered_map<string, vector<Booking>> FileHandler::vehicleMap;

static const string BOOKINGS_FILE = "C:/xampp/data/bookings.csv";
static const string CANC_FILE = "C:/xampp/data/cancellations.csv";
static const string CUST_FILE = "C:/xampp/data/customers.csv";

// ------------------ URL Decode ------------------
string FileHandler::urlDecode(const string &src) {
    string ret;
    char ch;
    int i, ii;
    for (i = 0; i < (int)src.length(); i++) {
        if (src[i] == '%') {
            if (i + 2 < (int)src.length()) {
                sscanf(src.substr(i + 1, 2).c_str(), "%x", &ii);
                ch = static_cast<char>(ii);
                ret += ch;
                i += 2;
            }
        } else if (src[i] == '+') {
            ret += ' ';
        } else {
            ret += src[i];
        }
    }
    return ret;
}

// ------------------ Parse POST ------------------
vector<pair<string, string>> FileHandler::parsePost(const string &post) {
    vector<pair<string, string>> out;
    stringstream ss(post);
    string token;
    while (getline(ss, token, '&')) {
        size_t eq = token.find('=');
        if (eq != string::npos) {
            string k = token.substr(0, eq);
            string v = token.substr(eq + 1);
            out.emplace_back(k, urlDecode(v));
        }
    }
    return out;
}

// ------------------ ID Generators ------------------
string FileHandler::genBookingID() {
    time_t t = time(nullptr);
    stringstream ss;
    ss << "B" << t;
    return ss.str();
}

string FileHandler::genCustomerID() {
    time_t t = time(nullptr);
    stringstream ss;
    ss << "C" << t;
    return ss.str();
}

// ------------------ Bookings ------------------
bool FileHandler::appendBooking(const Booking &b) {
    ofstream out(BOOKINGS_FILE, ios::app);
    if (!out.is_open()) return false;
    out << b.id << "," << b.name << "," << b.vehicleNo << "," << b.date << "," << b.serviceType << "\n";
    out.close();
    return true;
}

vector<Booking> FileHandler::loadBookings() {
    vector<Booking> v;
    ifstream in(BOOKINGS_FILE);
    if (!in.is_open()) return v;
    string line;
    while (getline(in, line)) {
        if (line.size() < 3) continue;
        stringstream ss(line);
        string id, name, veh, date, stype;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, veh, ',');
        getline(ss, date, ',');
        getline(ss, stype, ',');
        Booking b{ id, name, veh, date, stype };
        v.push_back(b);
    }
    in.close();
    return v;
}

bool FileHandler::removeBookingByID(const string &bid, Booking &removed) {
    vector<Booking> v = loadBookings();
    bool found = false;
    vector<Booking> keep;
    for (auto &b : v) {
        if (!found && b.id == bid) {
            removed = b;
            found = true;
        } else {
            keep.push_back(b);
        }
    }
    ofstream out(BOOKINGS_FILE, ios::trunc);
    if (!out.is_open()) return false;
    for (auto &b : keep) {
        out << b.id << "," << b.name << "," << b.vehicleNo << "," << b.date << "," << b.serviceType << "\n";
    }
    out.close();
    return found;
}

// ------------------ Cancellations (Stack) ------------------
bool FileHandler::appendCancellation(const Booking &b) {
    ofstream out(CANC_FILE, ios::app);
    if (!out.is_open()) return false;
    out << b.id << "," << b.name << "," << b.vehicleNo << "," << b.date << "," << b.serviceType << "\n";
    out.close();
    return true;
}

// ------------------ Undo (pop last cancellation) ------------------
bool FileHandler::popLastCancellation(Booking &b) {
    ifstream in(CANC_FILE);
    if (!in.is_open()) return false;

    vector<Booking> all;
    string line;
    while (getline(in, line)) {
        if (line.size() < 3) continue;
        stringstream ss(line);
        string id, name, veh, date, stype;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, veh, ',');
        getline(ss, date, ',');
        getline(ss, stype, ',');
        all.push_back({id, name, veh, date, stype});
    }
    in.close();

    if (all.empty()) return false;
    b = all.back();
    all.pop_back();

    ofstream out(CANC_FILE, ios::trunc);
    if (!out.is_open()) return false;
    for (auto &bk : all)
        out << bk.id << "," << bk.name << "," << bk.vehicleNo << "," << bk.date << "," << bk.serviceType << "\n";
    out.close();
    return true;
}
// ------------------ Customers (Linked List) ------------------
bool FileHandler::appendCustomer(const Customer &c) {
    ofstream out("C:/xampp/data/customers.csv", ios::app);
    if (!out.is_open()) return false;
    out << c.name << "," << c.contact << "," << c.vehicleNo << "\n";
    out.close();
    return true;
}

vector<Customer> FileHandler::loadCustomers() {
    vector<Customer> v;
    ifstream in("C:/xampp/data/customers.csv");
    if (!in.is_open()) return v;
    string line;
    while (getline(in, line)) {
        if (line.size() < 3) continue;
        stringstream ss(line);
        string name, contact, veh;
        getline(ss, name, ',');
        getline(ss, contact, ',');
        getline(ss, veh, ',');
        v.push_back({name, contact, veh, nullptr});
    }
    in.close();
    return v;
}

bool FileHandler::isVehicleRegistered(const string &vehicleNo) {
    auto v = loadCustomers();
    for (auto &c : v) {
        if (c.vehicleNo == vehicleNo) return true;
    }
    return false;
}

// ------------------ Load all data into memory ------------------
void FileHandler::loadAll() {
    auto bookings = loadBookings();
    for (auto &b : bookings) {
        dateQueues[b.date].enqueue(b);
        vehicleMap[b.vehicleNo].push_back(b);
    }
}

// ------------------ Sort and Search helpers ------------------
vector<Booking> FileHandler::sortBookingsByDate(vector<Booking> &v) {
    sort(v.begin(), v.end(), [](const Booking &a, const Booking &b) {
        return a.date < b.date;
    });
    return v;
}

Booking* FileHandler::searchByVehicleNo(const string &veh) {
    if (vehicleMap.find(veh) == vehicleMap.end() || vehicleMap[veh].empty()) return nullptr;
    return new Booking(vehicleMap[veh].front());
}
