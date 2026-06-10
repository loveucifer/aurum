#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <string>
#include <map>
#include <functional>
#include <filesystem>
#include <iostream>

// Polling-based file watcher (C++17 std::filesystem).

class FileWatcher {
public:
    using Callback = std::function<void(const std::string& path)>;

private:
    struct WatchedFile {
        std::filesystem::file_time_type lastModified;
        Callback callback;
    };

    std::map<std::string, WatchedFile> watchedFiles;
    float checkInterval      = 1.0f;
    float timeSinceLastCheck = 0.0f;
    bool  enabled            = true;

public:
    // Register a file path to watch. onChanged is called whenever the
    // file's mtime changes. Safe to call multiple times for the same path
    // (replaces the previous callback).
    void Watch(const std::string& filePath, Callback onChanged) {
        WatchedFile wf;
        try {
            wf.lastModified = std::filesystem::last_write_time(filePath);
        } catch (...) {
            std::cerr << "[FileWatcher] Cannot stat: " << filePath << std::endl;
            return;
        }
        wf.callback = std::move(onChanged);
        watchedFiles[filePath] = std::move(wf);
        std::cout << "[FileWatcher] Watching: " << filePath << std::endl;
    }


    void Update(float deltaTime) {
        if (!enabled) return;
        timeSinceLastCheck += deltaTime;
        if (timeSinceLastCheck < checkInterval) return;
        timeSinceLastCheck = 0.0f;

        for (auto& [path, wf] : watchedFiles) {
            try {
                auto current = std::filesystem::last_write_time(path);
                if (current != wf.lastModified) {
                    wf.lastModified = current;
                    std::cout << "[FileWatcher] Changed: " << path << std::endl;
                    wf.callback(path);
                }
            } catch (...) {

            }
        }
    }

    void Unwatch(const std::string& filePath) {
        watchedFiles.erase(filePath);
    }

    // How often to poll (default 1 s).
    void SetCheckInterval(float seconds) { checkInterval = seconds; }

    void SetEnabled(bool e) { enabled = e; }
    bool IsEnabled()  const { return enabled; }

    void ListWatched() const {
        std::cout << "[FileWatcher] Watching " << watchedFiles.size() << " file(s):" << std::endl;
        for (const auto& [path, wf] : watchedFiles) {
            std::cout << "  " << path << std::endl;
        }
    }
};

#endif
