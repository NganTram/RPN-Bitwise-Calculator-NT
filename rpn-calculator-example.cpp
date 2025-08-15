#include <stdint.h>

#include <bitset>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

/*
 * *** STUDENTS WILL NEED TO CHANGE INPUT_CSV_FILE PATH BELOW TO POINT TO THE rpn-input.csv FILE ***
 * This version assumes rpn-input.csv is in the same folder as this .cpp file.
 */
#define INPUT_CSV_FILE "rpn-input.csv"

// test controls
#define MIN_VALUE 0
#define MAX_VALUE 100
uint8_t const table_width[] = {14, 18, 14, 18, 14, 18};

// test harness structs and params
#define VALUE_NULLPTR -999

enum command : uint16_t
{
    cmd_enter = 0,
    cmd_clear,
    cmd_pop,
    cmd_top,
    cmd_left_shift,
    cmd_right_shift,
    cmd_or,
    cmd_and,
    cmd_add,
};
vector<string> command_name = {"cmd_enter", "cmd_clear", "cmd_pop", "cmd_top", "cmd_left_shift",
                               "cmd_right_shift", "cmd_or", "cmd_and", "cmd_add"};
uint8_t const width = 16U;

/*
 * Helper function: perform 16-bit bitwise-only addition and detect overflow
 */
static uint16_t bitwise_add16(uint16_t x, uint16_t y, bool &overflow)
{
    overflow = false;
    while (y != 0)
    {
        uint16_t pair = static_cast<uint16_t>(x & y);
        // Detect carry out of bit 15 before shifting
        if (pair & 0x8000u)
        {
            overflow = true;
            return 0;
        }
        uint16_t carry = static_cast<uint16_t>(pair << 1);
        x = static_cast<uint16_t>(x ^ y);
        y = carry;
    }
    return x;
}

/*
 * Main RPN calculator function
 */
shared_ptr<uint16_t> rpn_calc(command const cmd, uint16_t const value = 0)
{
    // Persistent stack across multiple calls
    static vector<uint16_t> stk;

    auto top_ptr = [&]() -> shared_ptr<uint16_t>
    {
        if (stk.empty())
            return nullptr;
        return make_shared<uint16_t>(stk.back());
    };

    switch (cmd)
    {
    case cmd_enter:
    {
        stk.push_back(static_cast<uint16_t>(value & 0xFFFFu));
        return top_ptr();
    }
    case cmd_clear:
    {
        stk.clear();
        return nullptr;
    }
    case cmd_pop:
    {
        if (stk.empty())
            return nullptr;
        stk.pop_back();
        return top_ptr();
    }
    case cmd_top:
    {
        return top_ptr();
    }
    case cmd_left_shift:
    case cmd_right_shift:
    case cmd_or:
    case cmd_and:
    case cmd_add:
    {
        if (stk.size() < 2)
            return nullptr;

        uint16_t a = stk.back();
        uint16_t b = stk[stk.size() - 2];

        if (cmd == cmd_add)
        {
            bool ovf = false;
            uint16_t sum = bitwise_add16(b, a, ovf);
            if (ovf)
                return nullptr;
            stk.pop_back();
            stk.pop_back();
            stk.push_back(sum);
            return top_ptr();
        }

        uint16_t r = 0;
        if (cmd == cmd_left_shift)
        {
            uint16_t s = static_cast<uint16_t>(a & 0x0Fu);
            r = static_cast<uint16_t>((b << s) & 0xFFFFu);
        }
        else if (cmd == cmd_right_shift)
        {
            uint16_t s = static_cast<uint16_t>(a & 0x0Fu);
            r = static_cast<uint16_t>(b >> s);
        }
        else if (cmd == cmd_or)
        {
            r = static_cast<uint16_t>((b | a) & 0xFFFFu);
        }
        else if (cmd == cmd_and)
        {
            r = static_cast<uint16_t>((b & a) & 0xFFFFu);
        }

        stk.pop_back();
        stk.pop_back();
        stk.push_back(r);
        return top_ptr();
    }
    default:
        return nullptr;
    }
}

/*
 * *** STUDENTS SHOULD NOT NEED TO CHANGE THE CODE BELOW. IT IS A CUSTOM TEST HARNESS. ***
 */

void header()
{
    cout << left << setw(table_width[0]) << setfill(' ') << "pass/fail";
    cout << left << setw(table_width[1]) << setfill(' ') << "command";
    cout << left << setw(table_width[2]) << setfill(' ') << "value";
    cout << left << setw(table_width[3]) << setfill(' ') << "value bits";
    cout << left << setw(table_width[4]) << setfill(' ') << "result";
    cout << left << setw(table_width[5]) << setfill(' ') << "result bits" << endl;

    cout << left << setw(table_width[0]) << setfill(' ') << "--------";
    cout << left << setw(table_width[1]) << setfill(' ') << "--------";
    cout << left << setw(table_width[2]) << setfill(' ') << "--------";
    cout << left << setw(table_width[3]) << setfill(' ') << "--------";
    cout << left << setw(table_width[4]) << setfill(' ') << "--------";
    cout << left << setw(table_width[5]) << setfill(' ') << "--------" << endl;
}

void print_row(bool const test_success, command const cmd, int16_t const value, shared_ptr<uint16_t> top_of_stack)
{
    string const pass_fail = test_success ? "PASS" : "FAIL";
    cout << left << setw(table_width[0]) << setfill(' ') << pass_fail;
    cout << left << setw(table_width[1]) << setfill(' ') << command_name[cmd];
    if (value == VALUE_NULLPTR)
    {
        cout << left << setw(table_width[2]) << setfill(' ') << " ";
        cout << left << setw(table_width[3]) << setfill(' ') << " ";
    }
    else
    {
        cout << left << setw(table_width[2]) << setfill(' ') << value;
        cout << left << setw(table_width[3]) << setfill(' ') << bitset<width>(value);
    }

    if (top_of_stack)
    {
        cout << left << setw(table_width[4]) << setfill(' ') << *top_of_stack;
        cout << left << setw(table_width[5]) << setfill(' ') << bitset<width>(*top_of_stack) << endl;
    }
    else
    {
        cout << left << setw(table_width[4]) << setfill(' ') << " ";
        cout << left << setw(table_width[5]) << setfill(' ') << " " << endl;
    }
}

vector<string> split(string const &s, string const &delimiter)
{
    vector<string> tokens;
    size_t pos = 0;
    size_t start = 0;
    string token;
    while (pos != string::npos)
    {
        pos = s.find(",", start);
        token = s.substr(start, pos - start);
        tokens.push_back(token);
        start = pos + delimiter.length();
    }
    return tokens;
}

void init_command_map(unordered_map<string, command> &command_map)
{
    for (size_t i = 0; i < command_name.size(); i++)
    {
        string const cmd = command_name[i];
        command_map[cmd] = static_cast<command>(i);
    }
}

bool parse_csv_line(string const line, unordered_map<string, command> command_map, command &input_cmd,
                    uint16_t &input_value, int32_t &answer_value)
{
    try
    {
        vector<string> tokens = split(line, ",");
        string cmd = tokens[0];
        if (command_map.count(cmd) == 1)
        {
            input_cmd = command_map[cmd];
        }
        else
        {
            cout << "ERROR: Invalid command cmd=" << cmd << endl;
            return false;
        }
        input_value = stoi(tokens[1].c_str());
        answer_value = stoi(tokens[2].c_str());
    }
    catch (exception const &e)
    {
        cout << "ERROR: Unable to parse input csv file, line=" << line << endl;
        cout << "ERROR: exception e=" << e.what() << endl;
        return false;
    }
    return true;
}

bool test()
{
    ifstream input_file(INPUT_CSV_FILE);
    if (!input_file.is_open())
    {
        cout << "ERROR: Unable to find and open the file " << INPUT_CSV_FILE << endl;
        cout << "       Make sure the path to the file is correct in your code" << endl;
        return false;
    }
    unordered_map<string, command> command_map;
    init_command_map(command_map);
    header();
    bool success = true;
    bool all_test_pass = true;
    uint16_t pass = 0;
    string line;
    size_t row = 0;
    while (getline(input_file, line))
    {
        if (row > 0)
        {
            command input_cmd;
            uint16_t input_value;
            int32_t input_answer;
            bool parse_success = parse_csv_line(line, command_map, input_cmd, input_value, input_answer);
            if (!parse_success)
            {
                return false;
            }
            shared_ptr<uint16_t> answer;
            if (input_answer == VALUE_NULLPTR)
            {
                answer = nullptr;
            }
            else
            {
                answer = make_shared<uint16_t>(input_answer);
            }
            shared_ptr<uint16_t> result = rpn_calc(input_cmd, input_value);
            bool test_success = false;
            bool both_null = answer == nullptr && result == nullptr;
            bool both_same_value = answer && result && (*answer == *result);
            if (both_null || both_same_value)
            {
                pass += 1;
                test_success = true;
            }
            all_test_pass = all_test_pass && test_success;
            print_row(test_success, input_cmd, input_value, result);
        }
        row += 1;
    }
    input_file.close();
    cout << "-------------------------------------------" << endl;
    if (all_test_pass)
    {
        cout << "SUCCESS ";
    }
    else
    {
        cout << "FAILURE ";
    }
    const size_t num_tests = row - 1;
    cout << pass << "/" << num_tests << " passed" << endl;
    cout << "-------------------------------------------" << endl;
    return success;
}

int main()
{
    if (!test())
    {
        return -1;
    }
    return 0;
}
