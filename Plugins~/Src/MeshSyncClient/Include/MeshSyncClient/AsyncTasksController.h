#pragma once

#include <vector>
#include <future>

namespace MeshSyncClient {

class AsyncTasksController {
public:
    template<typename T>
    void AddTask(const std::launch launchMode, T task) {
        m_tasks.push_back(std::async(launchMode, task));
    }

    void Wait();

private:

    std::vector<std::future<void>> m_tasks;

};


} // namespace MeshSyncClient
