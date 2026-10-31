#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

extern "C" {
#include "data.h"
#include "line_coding.h"
#include "scrambling.h"
}

#define MAX_BITS 1000

static HWND gInputEdit = NULL;
static HWND gZeroEdit = NULL;
static HWND gMethodCombo = NULL;
static HWND gSamplesEdit = NULL;
static HWND gStepEdit = NULL;
static HWND gBpsEdit = NULL;
static HWND gOutputEdit = NULL;
static HWND gPlotArea = NULL;
static std::vector<int> gPlotValues;
static std::string gPlotBits;
static int gPlotSamplesPerBit = 1;
static WNDPROC gPlotOriginalProc = NULL;

static void AppendText(HWND hwnd, const std::string &text)
{
    int len = GetWindowTextLengthA(hwnd);
    char *buffer = new char[len + text.length() + 1];
    if (!buffer) return;
    GetWindowTextA(hwnd, buffer, len + 1);
    std::string current(buffer);
    delete[] buffer;

    current += text;
    SetWindowTextA(hwnd, current.c_str());
}

static void ClearText(HWND hwnd)
{
    SetWindowTextA(hwnd, "");
}

static std::vector<double> ParseSamples(const std::string &text)
{
    std::vector<double> values;
    std::stringstream ss(text);
    std::string token;
    while (ss >> token) {
        double value = std::stod(token);
        values.push_back(value);
    }
    return values;
}

static std::string SignalToString(const std::vector<int> &data)
{
    std::ostringstream os;
    for (size_t i = 0; i < data.size(); ++i) {
        os << data[i];
        if (i + 1 < data.size()) os << ' ';
    }
    return os.str();
}

static std::vector<int> EncodeBitsByMethod(const std::string &bits, const std::string &method)
{
    std::vector<int> result;
    int n = (int)bits.size();
    if (n <= 0) return result;

    if (method == "NRZ-L") {
        result.reserve(n);
        for (int i = 0; i < n; ++i) result.push_back(bits[i] == '0' ? 1 : -1);
        return result;
    }
    if (method == "NRZ-I") {
        int level = 1;
        result.reserve(n);
        for (int i = 0; i < n; ++i) {
            if (bits[i] == '1') level = -level;
            result.push_back(level);
        }
        return result;
    }
    if (method == "Manchester") {
        result.reserve(n * 2);
        for (int i = 0; i < n; ++i) {
            if (bits[i] == '0') {
                result.push_back(-1);
                result.push_back(1);
            } else {
                result.push_back(1);
                result.push_back(-1);
            }
        }
        return result;
    }
    if (method == "Differential Manchester") {
        int level = 1;
        result.reserve(n * 2);
        for (int i = 0; i < n; ++i) {
            if (bits[i] == '0') level = -level;
            result.push_back(level);
            level = -level;
            result.push_back(level);
        }
        return result;
    }
    if (method == "AMI") {
        int lastPulse = -1;
        result.reserve(n);
        for (int i = 0; i < n; ++i) {
            if (bits[i] == '0') result.push_back(0);
            else {
                result.push_back(-lastPulse);
                lastPulse = result.back();
            }
        }
        return result;
    }
    if (method == "B8ZS") {
        int lastPulse = -1;
        result.reserve(n * 2);
        int i = 0;
        while (i < n) {
            bool hasEightZeros = (i + 7 < n);
            for (int j = 0; hasEightZeros && j < 8; ++j) {
                if (bits[i + j] != '0') hasEightZeros = false;
            }

            if (hasEightZeros) {
                result.push_back(0);
                result.push_back(0);
                result.push_back(0);
                result.push_back(lastPulse);
                result.push_back(-lastPulse);
                result.push_back(0);
                result.push_back(-lastPulse);
                result.push_back(lastPulse);
                lastPulse = result.back();
                i += 8;
            } else {
                if (bits[i] == '0') result.push_back(0);
                else {
                    result.push_back(-lastPulse);
                    lastPulse = result.back();
                }
                i++;
            }
        }
        return result;
    }
    if (method == "HDB3") {
        int lastPulse = -1;
        int pulseCount = 0;
        result.reserve(n * 2);
        int i = 0;
        while (i < n) {
            bool hasFourZeros = (i + 3 < n && bits[i] == '0' && bits[i + 1] == '0' && bits[i + 2] == '0' && bits[i + 3] == '0');
            if (hasFourZeros) {
                if (pulseCount % 2 == 0) {
                    int balancingPulse = -lastPulse;
                    result.push_back(balancingPulse);
                    result.push_back(0);
                    result.push_back(0);
                    result.push_back(balancingPulse);
                    lastPulse = balancingPulse;
                } else {
                    result.push_back(0);
                    result.push_back(0);
                    result.push_back(0);
                    result.push_back(lastPulse);
                }
                pulseCount = 0;
                i += 4;
            } else {
                if (bits[i] == '0') result.push_back(0);
                else {
                    result.push_back(-lastPulse);
                    lastPulse = result.back();
                    pulseCount++;
                }
                i++;
            }
        }
        return result;
    }
    return result;
}

static std::pair<std::vector<int>, std::string> DeltaEncode(const std::vector<double> &samples, double step)
{
    std::vector<int> bits;
    bits.reserve(samples.size());
    double reconstructed = 0.0;
    std::ostringstream os;
    for (size_t i = 0; i < samples.size(); ++i) {
        if (samples[i] >= reconstructed) {
            bits.push_back(1);
            reconstructed += step;
        } else {
            bits.push_back(0);
            reconstructed -= step;
        }
        os << bits.back();
        if (i + 1 < samples.size()) os << ' ';
    }
    return std::make_pair(bits, os.str());
}

static std::pair<std::vector<int>, std::string> PCMEncode(const std::vector<double> &samples, int bitsPerSample)
{
    std::vector<int> bits;
    std::ostringstream os;
    if (samples.empty()) return std::make_pair(bits, os.str());

    double minValue = samples[0];
    double maxValue = samples[0];
    for (double v : samples) {
        minValue = std::min(minValue, v);
        maxValue = std::max(maxValue, v);
    }

    int levels = 1 << bitsPerSample;
    double step = (maxValue - minValue) / levels;
    for (size_t i = 0; i < samples.size(); ++i) {
        int code = 0;
        if (step == 0.0) {
            code = 0;
        } else {
            code = (int)((samples[i] - minValue) / step);
            if (code >= levels) code = levels - 1;
            if (code < 0) code = 0;
        }
        for (int bit = bitsPerSample - 1; bit >= 0; --bit) {
            int value = (code >> bit) & 1;
            bits.push_back(value);
            os << value;
        }
        os << ' ';
    }
    return std::make_pair(bits, os.str());
}

static void DrawPlot(HDC hdc, const RECT &area, const std::vector<int> &values,
                     const std::string &bits, int samplesPerBit)
{
    HBRUSH bg = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &area, bg);
    DeleteObject(bg);

    int width = area.right - area.left;
    int height = area.bottom - area.top;
    int left = 52, right = width - 18;
    int top = 30, bottom = height - 34;
    if (right <= left || bottom <= top) return;

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(70, 70, 70));
    TextOutA(hdc, 8, 8, "Signal level", 12);
    TextOutA(hdc, left, height - 20, "Time (bit periods)", 18);

    int mid = top + (bottom - top) / 2;
    HPEN axisPen = CreatePen(PS_SOLID, 1, RGB(140, 140, 140));
    HPEN gridPen = CreatePen(PS_DOT, 1, RGB(220, 225, 232));
    HPEN dataPen = CreatePen(PS_SOLID, 2, RGB(0, 110, 220));
    HGDIOBJ oldPen = SelectObject(hdc, gridPen);
    for (int levelY = top; levelY <= bottom; levelY += (bottom - top) / 2) {
        MoveToEx(hdc, left, levelY, NULL);
        LineTo(hdc, right, levelY);
    }

    SelectObject(hdc, axisPen);
    MoveToEx(hdc, left, top, NULL);
    LineTo(hdc, left, bottom);
    MoveToEx(hdc, left, mid, NULL);
    LineTo(hdc, right, mid);
    TextOutA(hdc, 5, mid - 8, "0 V", 3);
    TextOutA(hdc, 25, top - 7, "+1", 2);
    TextOutA(hdc, 25, bottom - 7, "-1", 2);
    TextOutA(hdc, right - 110, mid + 4, "Reference (0 V)", 15);

    if (!values.empty()) {
        int amplitude = 1;
        for (size_t i = 0; i < values.size(); ++i) {
            amplitude = std::max(amplitude, (int)std::abs(values[i]));
        }
        int plotTop = top + 12;
        int plotBottom = bottom - 12;
        int usableHalfHeight = std::min(mid - plotTop, plotBottom - mid);
        double scale = usableHalfHeight / (double)amplitude;
        int plotWidth = right - left;

        SelectObject(hdc, gridPen);
        if (!bits.empty() && samplesPerBit > 0) {
            for (size_t bit = 1; bit < bits.size(); ++bit) {
                size_t sample = bit * (size_t)samplesPerBit;
                if (sample >= values.size()) break;
                int x = left + (int)((sample * (size_t)plotWidth) / values.size());
                MoveToEx(hdc, x, top, NULL);
                LineTo(hdc, x, bottom);
            }
        }

        SelectObject(hdc, dataPen);
        int previousY = mid - (int)(values[0] * scale);
        MoveToEx(hdc, left, previousY, NULL);
        for (size_t i = 0; i < values.size(); ++i) {
            int endX = left + (int)(((i + 1) * (size_t)plotWidth) / values.size());
            LineTo(hdc, endX, previousY);
            if (i + 1 < values.size()) {
                int nextY = mid - (int)(values[i + 1] * scale);
                if (nextY != previousY) {
                    LineTo(hdc, endX, nextY);
                }
                previousY = nextY;
            }
        }

        if (!bits.empty() && bits.size() <= 64 && samplesPerBit > 0) {
            std::string labels = "Bits: " + bits;
            TextOutA(hdc, left + 4, top + 2, labels.c_str(), (int)labels.size());
        }
    }

    SelectObject(hdc, oldPen);
    DeleteObject(axisPen);
    DeleteObject(gridPen);
    DeleteObject(dataPen);
}

static void DrawCurrentPlot(HWND hwnd)
{
    if (hwnd != NULL) {
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }
}

static LRESULT CALLBACK PlotWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_ERASEBKGND) return 1;
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT area;
        GetClientRect(hwnd, &area);
        DrawPlot(hdc, area, gPlotValues, gPlotBits, gPlotSamplesPerBit);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return CallWindowProcA(gPlotOriginalProc, hwnd, msg, wParam, lParam);
}

static void GenerateRandomBits()
{
    char buffer[64] = {0};
    GetWindowTextA(gZeroEdit, buffer, sizeof(buffer));
    int length = atoi(buffer);
    if (length <= 0 || length > MAX_BITS) length = 8;
    char data[MAX_BITS + 1];
    srand((unsigned int)GetTickCount());
    generateRandomData(data, length);
    SetWindowTextA(gInputEdit, data);
}

static void GenerateZeroPattern()
{
    char buffer[64] = {0};
    GetWindowTextA(gZeroEdit, buffer, sizeof(buffer));
    int length = atoi(buffer);
    if (length <= 0 || length > MAX_BITS) length = 8;
    int zeroCount = (length >= 8) ? 8 : 4;
    if (zeroCount > length) zeroCount = length;
    char data[MAX_BITS + 1];
    srand((unsigned int)GetTickCount());
    generateDataWithZeros(data, length, zeroCount);
    SetWindowTextA(gInputEdit, data);
}

static void EncodeSelectedMethod()
{
    char input[MAX_BITS + 1];
    GetWindowTextA(gInputEdit, input, sizeof(input));
    std::string bits = input;
    if (bits.empty()) {
        SetWindowTextA(gOutputEdit, "Please enter binary data first.");
        return;
    }

    char methodText[64];
    SendMessageA(gMethodCombo, CB_GETLBTEXT, (WPARAM)SendMessageA(gMethodCombo, CB_GETCURSEL, 0, 0), (LPARAM)methodText);
    std::string method = methodText;

    std::ostringstream out;
    if (method == "Delta Modulation") {
        char sampleText[256];
        GetWindowTextA(gSamplesEdit, sampleText, sizeof(sampleText));
        char stepText[64];
        GetWindowTextA(gStepEdit, stepText, sizeof(stepText));
        std::vector<double> samples = ParseSamples(sampleText);
        double step = std::stod(stepText);
        auto result = DeltaEncode(samples, step);
        out << "Method: Delta Modulation\n";
        out << "Samples: " << sampleText << "\n";
        out << "Step: " << step << "\n";
        out << "Encoded bits: " << result.second << "\n";
        gPlotValues = result.first;
        gPlotBits.clear();
        gPlotSamplesPerBit = 1;
        SetWindowTextA(gOutputEdit, out.str().c_str());
        DrawCurrentPlot(gPlotArea);
        return;
    }

    if (method == "PCM") {
        char sampleText[256];
        GetWindowTextA(gSamplesEdit, sampleText, sizeof(sampleText));
        char bpsText[32];
        GetWindowTextA(gBpsEdit, bpsText, sizeof(bpsText));
        std::vector<double> samples = ParseSamples(sampleText);
        int bitsPer = std::stoi(bpsText);
        auto result = PCMEncode(samples, bitsPer);
        out << "Method: PCM\n";
        out << "Samples: " << sampleText << "\n";
        out << "Bits per sample: " << bitsPer << "\n";
        out << "PCM bit stream: " << result.second << "\n";
        gPlotValues = result.first;
        gPlotBits.clear();
        gPlotSamplesPerBit = 1;
        SetWindowTextA(gOutputEdit, out.str().c_str());
        DrawCurrentPlot(gPlotArea);
        return;
    }

    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] != '0' && bits[i] != '1') {
            SetWindowTextA(gOutputEdit, "Only binary values 0 and 1 are allowed.");
            return;
        }
    }

    gPlotValues = EncodeBitsByMethod(bits, method);
    gPlotBits = bits;
    gPlotSamplesPerBit = (method == "Manchester" || method == "Differential Manchester") ? 2 : 1;
    out << "Method: " << method << "\n";
    out << "Input data: " << bits << "\n";
    out << "Signal levels: " << SignalToString(gPlotValues) << "\n";
    SetWindowTextA(gOutputEdit, out.str().c_str());
    DrawCurrentPlot(gPlotArea);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        RECT rc;
        GetClientRect(hwnd, &rc);

        CreateWindowExA(0, "STATIC", "Binary input:", WS_CHILD | WS_VISIBLE, 20, 20, 90, 22, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        gInputEdit = CreateWindowExA(0, "EDIT", "10110001", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, 120, 18, 260, 24, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        CreateWindowExA(0, "BUTTON", "Random", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 390, 18, 90, 26, hwnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        CreateWindowExA(0, "BUTTON", "Zero-run", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 490, 18, 100, 26, hwnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        CreateWindowExA(0, "STATIC", "Length:", WS_CHILD | WS_VISIBLE, 600, 22, 50, 20, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        gZeroEdit = CreateWindowExA(0, "EDIT", "8", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, 655, 18, 60, 24, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        CreateWindowExA(0, "STATIC", "Method:", WS_CHILD | WS_VISIBLE, 20, 60, 60, 22, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        gMethodCombo = CreateWindowExA(0, "COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 90, 56, 220, 200, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        const char *methods[] = {"NRZ-L", "NRZ-I", "Manchester", "Differential Manchester", "AMI", "B8ZS", "HDB3", "Delta Modulation", "PCM"};
        for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); ++i) {
            SendMessageA(gMethodCombo, CB_ADDSTRING, 0, (LPARAM)methods[i]);
        }
        SendMessageA(gMethodCombo, CB_SETCURSEL, 0, 0);

        CreateWindowExA(0, "STATIC", "Samples:", WS_CHILD | WS_VISIBLE, 330, 60, 60, 20, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        gSamplesEdit = CreateWindowExA(0, "EDIT", "0.2 0.8 1.1 0.7 0.3", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, 400, 56, 220, 24, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        CreateWindowExA(0, "STATIC", "Step:", WS_CHILD | WS_VISIBLE, 630, 60, 36, 20, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        gStepEdit = CreateWindowExA(0, "EDIT", "0.35", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, 675, 56, 60, 24, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        CreateWindowExA(0, "STATIC", "Bits/S:", WS_CHILD | WS_VISIBLE, 745, 60, 50, 20, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        gBpsEdit = CreateWindowExA(0, "EDIT", "4", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, 805, 56, 48, 24, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        CreateWindowExA(0, "BUTTON", "Encode", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 865, 18, 100, 30, hwnd, (HMENU)3, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        gPlotArea = CreateWindowExA(0, "STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_NOTIFY, 20, 110, 930, 300, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        gPlotOriginalProc = (WNDPROC)SetWindowLongPtrA(gPlotArea, GWLP_WNDPROC, (LONG_PTR)PlotWndProc);
        CreateWindowExA(0, "STATIC", "Output:", WS_CHILD | WS_VISIBLE, 20, 420, 80, 22, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        gOutputEdit = CreateWindowExA(0, "EDIT", "Ready. Select a method and press Encode.\r\n", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 20, 445, 930, 190, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        gPlotBits = "10110001";
        gPlotValues = EncodeBitsByMethod(gPlotBits, "NRZ-L");
        DrawCurrentPlot(gPlotArea);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) 
            GenerateRandomBits();
        else if (LOWORD(wParam) == 2) 
            GenerateZeroPattern();
        else if (LOWORD(wParam) == 3) 
            EncodeSelectedMethod();
        return 0;

    case WM_SIZE: {
        if (gPlotArea != NULL) {
            DrawCurrentPlot(gPlotArea);
        }
        return 0;
    }

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "DigitalSignalGuiClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Could not register window class.", "Error", MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExA(0, "DigitalSignalGuiClass", "Digital Signal Simulator", WS_OVERLAPPEDWINDOW,
        100, 100, 980, 710, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create window.", "Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}
