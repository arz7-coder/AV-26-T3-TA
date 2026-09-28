// Part A: This is an extension task that requires you to decode sensor data from CAN log files.
// CAN (Controller Area Network) is a communication standard used in automotive applications (including Redback cars)
// to allow communication between sensors and controllers.
//
// Your Task: Using the signal definitions in SteeringBench.dbc, read each CAN capture in data/
// and turn it into a CSV with one row per decoded frame:
// t,u_commanded,y_measured
// eg:
// 0,15.0,0.0
// 0.005,15.0,0.0
// ...
// where t is the frame timestamp minus the first kept frame's timestamp (s), u_commanded is
// the decoded CmdAngularRate (deg/s), and y_measured is the decoded MeasuredAngle (deg).
// The above values are not real numbers; they are only there to show the expected data output format.
// Do this for all three captures:
// data/step_test.log       ->  data/step_test.csv
// data/reversal_test.log   ->  data/reversal_test.csv
// data/deadband_test.log   ->  data/deadband_test.csv
//
// The Row type, writeCsv(), and main() below are provided -- they loop the three logs, call your
// decodeLog(), and write the CSV in exactly the format above. You just need to implement decodeLog().
//
// You do not need to use any external libraries. Use the resources below to understand how to
// extract sensor data.
// Hint: Think about manual bit masking and shifting, data types required,
// what formats are used to represent values, etc.
// Resources:
// https://www.csselectronics.com/pages/can-bus-simple-intro-tutorial
// https://www.csselectronics.com/pages/can-dbc-file-database-intro
//
// Sanity check: plot your CSVs (python3 plot_data.py) and compare against the pre-plotted
// data/*.png files -- they should match.
//
// Build & run (from the TA/ folder):
//     c++ -std=c++17 Question-A.cc -o decode
//     ./decode

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

// One output row.
struct Row {
    double t;            // seconds since the first kept frame
    double u_commanded;  // deg/s
    double y_measured;   // deg
};

// Read the candump log at `path` and return one Row per STEER_ActuatorLog frame, in order.
// Push one Row{t, u_commanded, y_measured} per kept frame.
std::vector<Row> decodeLog(const std::string& path) {
    std::vector<Row> rows;

    // TODO: your code here
    // to be removed: (void)path;  // remove once you open the file

    /*
    Pseudocode:

    extract header of 200 (which is the 512 CAN ID);

    if matches:
    check fields for:
    SG_ MeasuredAngle {0->16 bits} "deg"
        capture bytes using bit mask 0xFFFF000000000000
        shift down to 0x000000000000XXXX
        capture last 2 bytes and shift up
        capture first 2 X bytes and shift down
        remerge into one data
        convert to binary
        plug into output equation with offset and scales
        done?


    SG_ CmdAngularRate {16->32 bits} "deg/s"
    (see logic above)
   

    SG_ SupplyMilliVolts {32->48 bits}
    (see logic above - not needed though for this, cause milliVolts is not a requirement for the data)

    SG_ ActuatorTemp {48->64 bits}
    (see logic above - not needed though for this, cause degreess Celcius is not a requirement for the data)


    Then reorganize data.


    Disclaimer: As I do not know C++ code, I have translated my logic into C++ code via AI for testing and submission);



    */
    std::ifstream in(path);
    if (!in) {
        std::printf("Could not open %s\n", path.c_str());
        return rows;
    }

    bool haveFirst = false;
    double t0 = 0.0;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        // Pull the timestamp out of "(...)"
        size_t lp = line.find('(');
        size_t rp = line.find(')');
        if (lp == std::string::npos || rp == std::string::npos || rp < lp) continue;
        double timestamp = std::stod(line.substr(lp + 1, rp - lp - 1));

        // Find "ID#DATA"
        size_t hash = line.find('#', rp);
        if (hash == std::string::npos) continue;
        size_t idStart = line.rfind(' ', hash);
        if (idStart == std::string::npos) continue;
        idStart += 1;
        std::string idStr = line.substr(idStart, hash - idStart);
        unsigned long id = std::stoul(idStr, nullptr, 16);

        if (id != 512) continue;  // 0x200 -- only keep STEER_ActuatorLog frames

        // Grab the hex payload after '#', trim any trailing junk (CR, flags, etc.)
        std::string dataStr = line.substr(hash + 1);
        while (!dataStr.empty() && !std::isxdigit(static_cast<unsigned char>(dataStr.back())))
            dataStr.pop_back();
        if (dataStr.size() < 16) continue;  // need 8 bytes = 16 hex chars

        uint8_t bytes[8];
        for (int i = 0; i < 8; ++i)
            bytes[i] = static_cast<uint8_t>(std::stoul(dataStr.substr(i * 2, 2), nullptr, 16));

        // Little-endian 16-bit signed pairs, per the DBC
        int16_t rawMeasured = static_cast<int16_t>(bytes[0] | (bytes[1] << 8));
        int16_t rawCmdRate  = static_cast<int16_t>(bytes[2] | (bytes[3] << 8));

        double y_measured  = rawMeasured * 0.1;  // deg
        double u_commanded = rawCmdRate  * 0.1;  // deg/s

        if (!haveFirst) { t0 = timestamp; haveFirst = true; }

        rows.push_back(Row{timestamp - t0, u_commanded, y_measured});
    }

    return rows;
}

// Provided -- writes the rows to a CSV in the required format. Do not change.
void writeCsv(const std::string& path, const std::vector<Row>& rows) {
    std::ofstream f(path);
    f << "t,u_commanded,y_measured\n";
    for (const Row& r : rows)
        f << r.t << "," << r.u_commanded << "," << r.y_measured << "\n";
}

// Provided -- runs decodeLog() + writeCsv() for each of the three captures.
int main() {
    const char* names[] = {"step_test", "reversal_test", "deadband_test"};
    for (const char* n : names) {
        const std::string in  = std::string("data/") + n + ".log";
        const std::string out = std::string("data/") + n + ".csv";
        const std::vector<Row> rows = decodeLog(in);
        writeCsv(out, rows);
        std::printf("%-14s %6zu frames -> %s\n", n, rows.size(), out.c_str());
    }
    return 0;
}
