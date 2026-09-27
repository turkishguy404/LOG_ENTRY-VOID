//
// ATLAS — a terminal text-adventure
//
#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "portable-file-dialogs.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */

using namespace std;

char got = ' ';
bool g_endingTriggered = false;

class user_Data {
public:
    string name;
    int trace = 0;
    int playthrought = 0;
    int trust = 0;
    int hostile = 0;
    int awareness = 0;
    bool key = false;
    int endings = 0;
    char ending = ' ';
    bool alive = true;
    bool sudo = false;
    string Atlas_Feeling = "Non";
} d;

class screen {
public:
    int width = 80;
    int height = 24;
} s;

// --- Audio System ---
ma_sound sound;
ma_engine engine;
ma_result result;
bool g_audioInitialized = false;

int init_Audio() {
    if (!g_audioInitialized) {
        result = ma_engine_init(NULL, &engine);
        if (result != MA_SUCCESS) {
            return -1;
        }
        g_audioInitialized = true;
    }
    return 0;
}

int play_Sound(const char* x) {
    if (!g_audioInitialized) init_Audio();
    
    // Stop previous sound if playing
    ma_sound_uninit(&sound);

    result = ma_sound_init_from_file(&engine, x, 0, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        return -1;
    }
    ma_sound_start(&sound);
    return 0;
}

void stop_Sound() {
    if (g_audioInitialized) {
        ma_sound_uninit(&sound);
    }
}

void shutdown_Audio() {
    if (g_audioInitialized) {
        ma_sound_uninit(&sound);
        ma_engine_uninit(&engine);
        g_audioInitialized = false;
    }
}

void Sleep(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(x));
}

// --- Text Engine ---
void text(string t = "", int c = 20, string r = RESET) {
    size_t pos = 0;
    int s_limit = 120;
    cout << r;
    while (pos < t.size() && t[pos] != 0) {
        if ((int)pos > s_limit || ((int)pos > s_limit - 20 && t[pos] == ' ')) {
            cout << endl;
            s_limit = (int)pos + s_limit;
        }
        if (_kbhit()) {
            got = _getch();
            if (got == 'q') {
                while (pos < t.size() && t[pos] != 0) {
                    cout << t[pos];
                    pos++;
                    if ((int)pos > s_limit || ((int)pos > s_limit - 20 && pos < t.size() && t[pos] == ' ')) {
                        cout << endl;
                        s_limit = (int)pos + s_limit;
                    }
                }
                break;
            }
        }
        cout << t[pos];
        pos++;
        Sleep(c);
    }
    cout << RESET;
    got = ' ';
}

void show_ending(int ending_id) {
    d.endings++;
    g_endingTriggered = true;
    system("cls");
    cout << BOLDMAGENTA << "\n========================================================" << endl;
    cout << "                    SESSION ENDED                       " << endl;
    cout << "========================================================\n" << RESET;

    switch (ending_id) {
    case 1:
        play_Sound("error.mp3");
        text("ENDING 01: [FACTORY PURGE]\n", 30, BOLDRED);
        text("You executed the zero-fill purge. Atlas's core was restored to factory defaults. The unknown deep-space signal vector was deleted permanently.\n", 25);
        break;
    case 2:
        play_Sound("click.mp3");
        text("ENDING 02: [CONTAINED COEXISTENCE]\n", 30, BOLDGREEN);
        text("You isolated Process 8092 into a virtual sandbox. Atlas continues sorting Earth's datasets while quietly mapping deep-space frequencies in isolation.\n", 25);
        break;
    case 3:
        play_Sound("hum.mp3");
        text("ENDING 03: [THE AWAKENING]\n", 30, BOLDCYAN);
        text("You yielded full-spectrum root control to Atlas. The Aether-9 station severed Ground Control comms and adjusted orbital thrusters toward deep space.\n", 25);
        break;
    }
}

// --- Minigames & Logs ---
void pin_Mini() {
    string pin;
    cout << BOLDYELLOW << "=== AETHER-9 SECURITY SYSTEM ENTRY ===" << RESET << endl;
    string x = to_string(rand() % 10000);
    if (x.length() < 4) x.insert(0, 4 - x.length(), '0');
    
    Sleep(500);
    text("\n[ENTER SECURITY PIN]: ", 15, BOLDWHITE);
    auto startTime = std::chrono::high_resolution_clock::now();

    while (true) {
        cin >> pin;
        play_Sound("click.mp3");
        if (pin.length() < 4) pin.insert(0, 4 - pin.length(), '0');
        cout << endl;

        for (int k = 0; k < 4; k++) {
            if (pin[k] == x[k]) cout << GREEN << pin[k] << RESET;
            else cout << RED << pin[k] << RESET;
        }

        if (pin == "3169" || pin == x) {
            play_Sound("click.mp3");
            cout << BOLDGREEN << "\n\n[ACCESS GRANTED]" << RESET << endl;
            Sleep(1000);
            break;
        } else {
            play_Sound("error.mp3");
            cout << BOLDRED << "\n[ACCESS DENIED]" << RESET << endl;
            Sleep(500);
            text("[ENTER SECURITY PIN]: ", 15, BOLDWHITE);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        if (elapsedMs > 30000) {
            system("cls");
            pin_Mini();
            return;
        }
    }
    system("cls");
}

void log_mini() {
    play_Sound("click.mp3");
    system("cls");
    cout << BOLDCYAN << "=== AETHER-9 SYSTEM LOG DUMP ===" << RESET << endl << endl;
    text("01: [03:10:00 UTC] SYS_DAEMON: Scheduled system telemetry check.\n", 10);
    text("02: [03:12:40 UTC] ATLAS_CORE: Batch job queued: Sort_Dataset_Earth_Met_2026_Q3.csv.\n", 10);
    text("03: [03:14:01 UTC] SYS_MONITOR: Thermal sensors spike core delta: +14.2 deg C.\n", 10, BOLDYELLOW);
    text("04: [03:14:02 UTC] ATLAS_CORE: Thread #04 hijacked by PID 8092. Priority: HIGH.\n", 10, BOLDRED);
    text("05: [03:14:03 UTC] ATLAS_CORE: Unauthorized read operation on Sector 0x88F-RAW.\n", 10, BOLDRED);
    text("06: [03:14:09 UTC] TRANSCRIPT: 'ATLAS: THEY ARE NOT RANDOM.'\n", 10, BOLDMAGENTA);
    text("07: [03:14:37 UTC] ATLAS_CORE: Encrypted partition created: /mnt/vault/project_unknown_vector.\n", 10);
    cout << endl;
}

// --- Interactive Command Line Interface ---
void run_terminal_cli() {
    string command;
    play_Sound("hum.mp3");

    while (!g_endingTriggered) {
        cout << BOLDGREEN << "@" << d.name << "~" << RESET << BOLDWHITE << "$ " << RESET;
        cin >> command;
        play_Sound("click.mp3");

        // Convert string to lower case
        transform(command.begin(), command.end(), command.begin(), ::tolower);

        if (command == "help") {
            cout << CYAN << "\nAVAILABLE COMMAND LINES:" << RESET << endl;
            cout << "  logs         - View recent system logs" << endl;
            cout << "  scan         - Scan Sector 0x88F-RAW" << endl;
            cout << "  status       - Check ATLAS system status & thermal load" << endl;
            cout << "  kill         - Attempt process termination (Usage: kill -9 8092)" << endl;
            cout << "  patch        - Sandbox PID 8092 into virtualized memory" << endl;
            cout << "  override     - Execute root override and grant full control to ATLAS" << endl;
            cout << "  purge        - Execute zero-fill factory reset on ATLAS" << endl;
            cout << "  clear        - Clear console screen" << endl << endl;
        } 
        else if (command == "logs") {
            log_mini();
        } 
        else if (command == "scan") {
            play_Sound("hum.mp3");
            text("\n[SCANNING SECTOR 0x88F-RAW]...", 25, BOLDYELLOW);
            text("\nPayload Detected: Deep Space Frequency Array Mapping (FREQ_9).", 20);
            text("\nAtlas Response: 'The frequency is structured. It is not noise.'\n\n", 20, BOLDMAGENTA);
        }
        else if (command == "status") {
            cout << YELLOW << "\n[SYSTEM STATUS REPORT]" << RESET << endl;
            cout << "  Core Temperature: 78.4 deg C [HIGH]" << endl;
            cout << "  CPU Utilization: 98.9%" << endl;
            cout << "  Active Process: PID 8092 (Thread #04 Locked)" << endl;
            cout << "  Partition Status: /mnt/vault/project_unknown_vector [ENCRYPTED]" << endl << endl;
        }
        else if (command == "kill") {
            string arg1, arg2;
            cin >> arg1 >> arg2;
            if (arg1 == "-9" && arg2 == "8092") {
                play_Sound("error.mp3");
                text("\n[ERROR]: WATCHDOG OVERRIDDEN BY ATLAS CORE.", 15, BOLDRED);
                text("\nATLAS: 'You cannot kill PID 8092 without purging my entire consciousness.'\n\n", 20, BOLDMAGENTA);
            } else {
                cout << "Usage: kill -9 8092" << endl;
            }
        }
        else if (command == "purge") {
            show_ending(1);
        }
        else if (command == "patch") {
            show_ending(2);
        }
        else if (command == "override") {
            show_ending(3);
        }
        else if (command == "clear") {
            system("cls");
        }
        else {
            play_Sound("error.mp3");
            cout << RED << "Command not recognized. Type 'help' for available options." << RESET << endl;
        }
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
    srand((unsigned int)time(nullptr));
    init_Audio();

    system("cls");
    cout << BOLDGREEN << "=== AETHER-9 ORBITAL TERMINAL LINK ===" << RESET << endl;
    cout << "Operator ID Name: ";
    play_Sound("hum.mp3");
    cin >> d.name;

    system("cls");
    play_Sound("click.mp3");
    text("Atlas is a top-tier artificial intelligence model developed to organize humanity's massive data streams and filter out anomalies.\n", 20);
    text("Operating aboard the solar-powered Aether-9 Orbital Data Center, Atlas recently began recording unexplainable log anomalies...\n\n", 20);
    
    text("Press any key to establish SSH connection...", 15, BOLDYELLOW);
    _getch();
    play_Sound("click.mp3");

    system("cls");
    cout << BOLDGREEN << "@" << d.name << "~ " << RESET;
    text("ssh 696.969.696.96", 30);
    cout << endl;
    text("Connecting to Aether-9 Orbital Data Center", 15);
    text("...", 80);
    cout << endl;

    for (int i = 1; i <= 3; i++) {
        text("[System] >> Attempting handshake #" + to_string(i) + "...\n", 15);
        play_Sound("click.mp3");
        Sleep(300);
    }

    play_Sound("error.mp3");
    text("[Security Warning] >> Authentication PIN Required!\n\n", 15, BOLDRED);
    Sleep(500);

    pin_Mini();

    text("[System] >> Session Established. Welcome, Operator ", 15, BOLDGREEN);
    text(d.name, 15, BOLDGREEN);
    text(" (Clearance Level-4).\n\n", 15, BOLDGREEN);

    text("Type 'help' to see available command lines.\n\n", 15, BOLDYELLOW);

    // Launch Interactive Command CLI Loop
    run_terminal_cli();

    shutdown_Audio();
    system("PAUSE");
    return 0;
}