//
// Created by rweyra on 5/17/17.
//

#ifndef PATHTRACER_PROGRESS_H
#define PATHTRACER_PROGRESS_H

#include <string>
#include <chrono>
class Progress
{
public:
    Progress(int totalIterations, const std::string& title, const std::string& statusLabel = "");

    ~Progress();

    void update(int numIterations = 1, float value = 0.0f);

    void completed();

protected:
    int m_totalIterations;
    int m_iterationsCompleted;
    int m_printed, m_total;
    char *m_buf, *m_curSpace;
    const std::string m_statusLabel;
    std::chrono::steady_clock::time_point m_start;
};

#endif //PATHTRACER_PROGRESS_H
