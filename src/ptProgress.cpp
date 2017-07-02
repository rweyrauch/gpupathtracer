//
// Created by rweyra on 5/17/17.
//

#include <cstring>
#include <cmath>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ptProgress.h"

int GetTerminalWidth()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

Progress::Progress(int totalIterations, const std::string &title, const std::string& statusLabel) :
    m_totalIterations(totalIterations),
    m_iterationsCompleted(0),
    m_printed(0),
    m_total(0),
    m_buf(nullptr),
    m_curSpace(nullptr),
    m_statusLabel(statusLabel)
{
    int length = GetTerminalWidth() - 42;
    m_total = std::max(2, length - static_cast<int>(title.size()) - static_cast<int>(m_statusLabel.size()));

    auto bufLen = title.size() + m_statusLabel.size() + m_total + 64;
    m_buf = new char[bufLen];
    snprintf(m_buf, bufLen, "\r%s: [", title.c_str());
    m_curSpace = m_buf + strlen(m_buf);
    char *s = m_curSpace;
    for (int i = 0; i < m_total; ++i)
        *s++ = ' ';
    *s++ = ']';
    *s++ = ' ';
    *s++ = '\0';
    fputs(m_buf, stdout);
    fflush(stdout);
    system("tput civis");
    m_start = std::chrono::steady_clock::now();
}

Progress::~Progress()
{
    delete[] m_buf;
}

void Progress::update(int numIterations, float value)
{
    m_iterationsCompleted += numIterations;
    float percentDone = static_cast<float>(m_iterationsCompleted) / static_cast<float>(m_totalIterations);
    int needed = static_cast<int>(roundf(m_total * percentDone));
    if (needed > m_total) needed = m_total;
    while (m_printed < needed)
    {
        *m_curSpace++ = '*';
        ++m_printed;
    }
    fputs(m_buf, stdout);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - m_start;
    float totalSeconds = static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
    float estRemaining = totalSeconds / percentDone - totalSeconds;
    int hours = static_cast<int>(floorf(totalSeconds/3600.0f));
    totalSeconds -= hours * 3600;
    int minutes = static_cast<int>(floorf(totalSeconds/60.0f));
    int seconds = static_cast<int>(totalSeconds) % 60;
    int estHours = static_cast<int>(floorf(estRemaining/3600.0f));
    estRemaining -= estHours * 3600;
    int estMinutes = static_cast<int>(floorf(estRemaining/60.0f));
    int estSeconds = static_cast<int>(estRemaining) % 60;

    if (percentDone >= 1.0f)
    {
        fprintf(stdout, " (%3d:%02d:%02ds)           ", hours, minutes, seconds);
    }
    else
    {
        if (m_statusLabel.empty())
            fprintf(stdout, " (%3d:%02d:%02ds|%3d:%02d:%02ds)  ", hours, minutes, seconds, estHours, estMinutes, std::max(0, estSeconds));
        else
            fprintf(stdout, " (%3d:%02d:%02ds|%3d:%02d:%02ds) %s=%6.2f", hours, minutes, seconds, estHours, estMinutes, std::max(0, estSeconds), m_statusLabel.c_str(), value);
    }
    fflush(stdout);
}

void Progress::completed()
{
    while (m_printed++ < m_total)
    {
        *m_curSpace++ = '+';
    }
    fputs(m_buf, stdout);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - m_start;
    float totalSeconds = static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
    int hours = static_cast<int>(floorf(totalSeconds/3600.0f));
    totalSeconds -= hours * 3600;
    int minutes = static_cast<int>(floorf(totalSeconds/60.0f));
    int seconds = static_cast<int>(totalSeconds) % 60;
    fprintf(stdout, " (%3d:%02d:%02ds)           \n", hours, minutes, seconds);
    fflush(stdout);
    system("tput cnorm");
}
