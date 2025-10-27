#include <bits/stdc++.h>
#include "FileHandler.h"
using namespace std;

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

static vector<pair<string, string>> parsePostLocal(const string &post) {
    vector<pair<string, string>> out;
    stringstream ss(post); string token;
    while (getline(ss, token, '&')) {
        size_t eq = token.find('=');
        if (eq != string::npos)
            out.emplace_back(token.substr(0, eq), urlDecodeLocal(token.substr(eq + 1)));
    }
    return out;
}

int main() {
    cout << "Content-type:text/html\r\n\r\n";
    cout << "<html><body style='font-family:Arial'>";
    cout << "<h2>Cancel Booking</h2>";

    int contentLength = 0;
    char *cl = getenv("CONTENT_LENGTH");
    if (cl) contentLength = atoi(cl);
    string post;
    if (contentLength > 0) { post.resize(contentLength); cin.read(&post[0], contentLength); }
    else getline(cin, post);

    auto pairs = parsePostLocal(post);
    string bookingID = "";
    for (auto &p : pairs) if (p.first == "bookingID") bookingID = p.second;

    if (bookingID.empty()) {
        cout << "<h3>Error: Enter Booking ID.</h3>";
        cout << "<a href='/vehicle_service/cancel.html'>Go Back</a></body></html>";
        return 0;
    }

    FileHandler::loadAll();

    Booking removed;
    bool ok = FileHandler::removeBookingByID(bookingID, removed);
    if (!ok) {
        cout << "<h3>No booking found with ID " << bookingID << ".</h3>";
    } else {
        FileHandler::appendCancellation(removed);
        cout << "<p>Booking Cancelled.</p>"
             << "<p>ID: " << removed.id << "</p>"
             << "<p>Vehicle: " << removed.vehicleNo << "</p>";
    }

    cout << "<p><a href='/vehicle_service/index.html'>Home</a></p></body></html>";
    return 0;
}
