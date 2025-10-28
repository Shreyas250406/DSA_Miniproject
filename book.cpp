#include <bits/stdc++.h>
#include "FileHandler.h"
using namespace std;

// ------------------ Helper: URL Decode ------------------
static string urlDecodeLocal(const string &src) {
    string ret; char ch; int i, ii;
    for (i = 0; i < (int)src.length(); i++) {
        if (src[i] == '%') {
            if (i + 2 < (int)src.length()) {
                sscanf(src.substr(i + 1, 2).c_str(), "%x", &ii);
                ch = static_cast<char>(ii);
                ret += ch;
                i += 2;
            }
        } else if (src[i] == '+') ret += ' ';
        else ret += src[i];
    }
    return ret;
}

// ------------------ Helper: Parse POST Data ------------------
static vector<pair<string, string>> parsePostLocal(const string &post) {
    vector<pair<string, string>> out;
    stringstream ss(post);
    string token;
    while (getline(ss, token, '&')) {
        size_t eq = token.find('=');
        if (eq != string::npos)
            out.emplace_back(token.substr(0, eq), urlDecodeLocal(token.substr(eq + 1)));
    }
    return out;
}

// ------------------ MAIN ------------------
int main() {
    cout << "Content-type:text/html\r\n\r\n";
    cout << "<html><head><title>Booking Confirmation</title>"
         << "<style>body{font-family:Arial;background:#0d1117;color:#e6edf3;padding:30px;}"
         << "a{color:#58a6ff;text-decoration:none;}a:hover{text-decoration:underline;}</style>"
         << "</head><body>";

    cout << "<h2>Vehicle Service Booking</h2>";

    // Read POST data from HTML form
    int contentLength = 0;
    char *cl = getenv("CONTENT_LENGTH");
    if (cl) contentLength = atoi(cl);

    string post;
    if (contentLength > 0) {
        post.resize(contentLength);
        cin.read(&post[0], contentLength);
    }

    auto pairs = parsePostLocal(post);
    string vehicleNo = "", date = "", name = "", serviceType = "General Service";

    for (auto &p : pairs) {
        if (p.first == "name") name = p.second;
        else if (p.first == "vehicleNo") vehicleNo = p.second;
        else if (p.first == "date") date = p.second;
        else if (p.first == "serviceType") serviceType = p.second;
    }

    // Basic validation
    if (vehicleNo.empty() || date.empty()) {
        cout << "<h3 style='color:red'>Error: Missing required fields.</h3>"
             << "<p><a href='/vehicle_service/booking.html'>Go Back</a></p></body></html>";
        return 0;
    }

    // Load all current data into memory
    FileHandler::loadAll();

    // Initialize a circular queue for the date if not present
    if (FileHandler::dateQueues.find(date) == FileHandler::dateQueues.end())
        FileHandler::dateQueues.emplace(date, CircularQueue(FileHandler::MAX_SLOTS_PER_DATE));

    CircularQueue &queue = FileHandler::dateQueues[date];

    // Check slot availability
    if (queue.isFull()) {
        cout << "<h3 style='color:orange'>All slots are full for " << date << ".</h3>"
             << "<p><a href='/vehicle_service/booking.html'>Try another date</a></p></body></html>";
        return 0;
    }

    // Create a new booking entry
    Booking b;
    b.id = FileHandler::genBookingID();
    b.name = name.empty() ? "Customer" : name;
    b.vehicleNo = vehicleNo;
    b.date = date;
    b.serviceType = serviceType;

    // Save booking to file
    if (FileHandler::appendBooking(b)) {
        FileHandler::vehicleMap[vehicleNo].push_back(b);
        FileHandler::dateQueues[date].enqueue(b);

        // Add customer to file
        Customer c;
        c.name = b.name;
        c.contact = "N/A";
        c.vehicleNo = b.vehicleNo;
        FileHandler::appendCustomer(c);

        // Output confirmation page
        cout << "<h3 style='color:#3fb950'>Booking Confirmed!</h3>"
             << "<p><b>Booking ID:</b> " << b.id << "</p>"
             << "<p><b>Name:</b> " << b.name << "</p>"
             << "<p><b>Vehicle:</b> " << b.vehicleNo << "</p>"
             << "<p><b>Date:</b> " << b.date << "</p>"
             << "<p><b>Service Type:</b> " << b.serviceType << "</p>";
    } else {
        cout << "<h3 style='color:red'>Error saving booking data.</h3>";
    }

    cout << "<p><a href='/vehicle_service/index.html'>Back to Home</a></p>";
    cout << "</body></html>";
    return 0;
}
