#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

using namespace std;

// Read expected.txt
vector<string> readLines(const string& filename) {
    vector<string> lines;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "Cannot open " << filename << endl;
        exit(1);
    }

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }

    return lines;
}

// Run interpreter and compare output with expected.txt
vector<string> runInterpreter() {
    vector<string> output;

    FILE* pipe = _popen(
        "SExpression.exe < input.txt", "r"
    );

    if (pipe == nullptr) {
        cerr << "Failed to run interpreter." << endl;
        exit(1);
    }

    char buffer[4096];
    string currentLine;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        currentLine += buffer;

        size_t pos;
        while ((pos = currentLine.find('\n')) != string::npos) {
            string line = currentLine.substr(0, pos);

            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            output.push_back(line);
            currentLine.erase(0, pos + 1);
        }
    }

    if (!currentLine.empty()) {
        if (currentLine.back() == '\r') {
            currentLine.pop_back();
        }
        output.push_back(currentLine);
    }

    int status = _pclose(pipe);

    if (status != 0) {
        cerr << "Interpreter exited with an error." << endl;
        exit(1);
    }

    return output;
}

int main() {
    vector<string> expected = readLines("expected.txt");
    vector<string> actual = runInterpreter();

    size_t count = max(expected.size(), actual.size());

    for (size_t i = 0; i < count; i++) {
        bool passed =
            i < expected.size() &&
            i < actual.size() &&
            expected[i] == actual[i];

        cout << "Test " << (i + 1) << ": " << (passed ? "PASS" : "FAIL") << endl;
    }

    return 0;
}
