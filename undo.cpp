#include <bits/stdc++.h>
#include "FileHandler.h"
using namespace std;

int main() {
    cout << "Content-type:text/html\r\n\r\n";
    cout << "<html><body style='font-family:Arial'>";
    cout << "<h2>Undo Cancellation</h2>";

    FileHandler::loadAll();
    Booking b;
    bool ok = FileHandler::popLastCancellation(b);
    if (!ok) {
        cout << "<h3>No cancellations to undo.</h3>";
    } else {
        b.id = FileHandler::genBookingID();
        if (FileHandler::appendBooking(b))
            cout << "<p>Undo successful. Restored booking for <b>"
                 << b.vehicleNo << "</b> on " << b.date << "</p>";
        else
            cout << "<h3>Error restoring booking.</h3>";
    }

    cout << "<p><a href='/vehicle_service/index.html'>Home</a></p></body></html>";
    return 0;
}
