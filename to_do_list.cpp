#include <iostream>
#include <vector>
#include <memory>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <cstdlib>
#include <cassert>

using namespace std; // Using the entire std namespace for simplicity

// Logging Utility
class Logger {
public:
    // Log message to a file with a timestamp
    static void log(const string& message) {
        ofstream logFile("app_log.txt", ios_base::app);
        if (logFile.is_open()) {
            logFile << getCurrentTime() << " " << message << endl;
            logFile.close();
        }
    }

private:
    // Get the current time as a formatted string
    static string getCurrentTime() {
        time_t now = time(nullptr);
        tm timeinfo = *localtime(&now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S]", &timeinfo);
        return buffer;
    }
};

// Custom Exception for TaskManager
class TaskManagerException : public runtime_error {
public:
    TaskManagerException(const string& message) : runtime_error(message) {}
};

class TaskMemento {
public:
    string description;
    bool completed;
    tm due_date;

    TaskMemento(const string& desc, bool comp, const tm& date)
        : description(desc), completed(comp), due_date(date) {}
};

class Task {
public:
    string description;
    bool completed;
    tm due_date;

    Task(const string& desc) : description(desc), completed(false) {}

    // Mark the task as completed
    void mark_completed() {
        completed = true;
    }

    // Undo the task to a previous state using a memento
    void undo(const TaskMemento& memento) {
        description = memento.description;
        completed = memento.completed;
        due_date = memento.due_date;
    }

    // Create a memento representing the current state of the task
    TaskMemento create_memento() const {
        return TaskMemento(description, completed, due_date);
    }

    // Print the task details
    void print() const {
        string status = completed ? "Completed" : "Pending";
        cout << description << " - " << status << ", Due: " << due_date.tm_year + 1900
            << "-" << due_date.tm_mon + 1 << "-" << due_date.tm_mday << endl;
    }
};

class TaskBuilder {
private:
    shared_ptr<Task> task;

public:
    // Initialize the task builder with a description
    TaskBuilder(const string& desc) : task(make_shared<Task>(desc)) {}

    // Set the due date for the task
    TaskBuilder& set_due_date(const tm& date) {
        task->due_date = date;
        return *this;
    }

    // Build and return the task
    shared_ptr<Task> build() const {
        return task;
    }
};

class TodoListManager {
private:
    vector<shared_ptr<Task>> tasks;
    vector<TaskMemento> undo_stack;
    vector<TaskMemento> redo_stack;

public:
    // Add a task to the task list
    void add_task(const shared_ptr<Task>& task) {
        tasks.push_back(task);
        undo_stack.push_back(task->create_memento());
        Logger::log("Task added: " + task->description);
    }

    // Mark a task as completed
    bool mark_completed(const string& description) {
        for (auto& task : tasks) {
            if (task->description == description && !task->completed) {
                task->mark_completed();
                undo_stack.push_back(task->create_memento());
                Logger::log("Task marked as completed: " + task->description);
                return true;
            }
        }
        Logger::log("Task not found or already completed: " + description);
        return false;
    }

    // Delete a task from the task list
    bool delete_task(const string& description) {
        for (auto& task : tasks) {
            if (task->description == description) {
                tasks.erase(remove(tasks.begin(), tasks.end(), task), tasks.end());
                undo_stack.push_back(TaskMemento("", false, tm{}));
                Logger::log("Task deleted: " + description);
                return true;
            }
        }
        Logger::log("Task not found: " + description);
        return false;
    }

    // Undo the last operation
    void undo() {
        if (undo_stack.size() > 1) {
            auto task_memento = undo_stack.back();
            undo_stack.pop_back();
            redo_stack.push_back(task_memento);
            tasks.back()->undo(undo_stack.back());
            Logger::log("Undo completed");
        } else {
            Logger::log("Undo not possible");
        }
    }

    // Redo the last undone operation
    void redo() {
        if (!redo_stack.empty()) {
            auto task_memento = redo_stack.back();
            redo_stack.pop_back();
            undo_stack.push_back(task_memento);
            if (task_memento.description.empty()) {
                tasks.pop_back();
                Logger::log("Redo completed (Task deleted)");
            } else {
                tasks.back()->undo(undo_stack.back());
                Logger::log("Redo completed");
            }
        } else {
            Logger::log("Redo not possible");
        }
    }

    // View tasks based on a filter option (all, completed, pending)
    void view_tasks(const string& filter_option = "all") const {
        auto filtered_tasks = tasks;
        if (filter_option == "completed") {
            filtered_tasks.erase(
                remove_if(filtered_tasks.begin(), filtered_tasks.end(),
                    [](const shared_ptr<Task>& task) { return !task->completed; }),
                filtered_tasks.end()
            );
        } else if (filter_option == "pending") {
            filtered_tasks.erase(
                remove_if(filtered_tasks.begin(), filtered_tasks.end(),
                    [](const shared_ptr<Task>& task) { return task->completed; }),
                filtered_tasks.end()
            );
        }

        for (const auto& task : filtered_tasks) {
            task->print();
        }
    }
};

int main() {
    TodoListManager todo_manager;

    try {
        Logger::log("To-Do List Manager");

        while (true) {
            cout << "\nOptions:\n";
            cout << "1. Add Task\n";
            cout << "2. Mark Task as Completed\n";
            cout << "3. Delete Task\n";
            cout << "4. View Tasks\n";
            cout << "5. Undo\n";
            cout << "6. Redo\n";
            cout << "7. Exit\n";
            cout << "Enter your choice: ";

            int choice;
            cin >> choice;

            switch (choice) {
                case 1: {
                    string description;
                    cout << "Enter task description: ";
                    cin.ignore();
                    getline(cin, description);

                    tm due_date = {};
                    string dateStr;
                    cout << "Enter due date (YYYY MM DD): ";
                    cin >> dateStr;

                    istringstream dateStream(dateStr);
                    dateStream >> get_time(&due_date, "%Y %m %d");

                    if (dateStream.fail()) {
                        throw TaskManagerException("Invalid date format");
                    }

                    auto task = TaskBuilder(description).set_due_date(due_date).build();
                    todo_manager.add_task(task);

                    cout << "Task added successfully!\n";
                    break;
                }
                case 2: {
                    string description;
                    cout << "Enter task description to mark as completed: ";
                    cin.ignore();
                    getline(cin, description);

                    if (todo_manager.mark_completed(description)) {
                        cout << "Task marked as completed!\n";
                    } else {
                        cout << "Task not found or already completed!\n";
                    }
                    break;
                }
                case 3: {
                    string description;
                    cout << "Enter task description to delete: ";
                    cin.ignore();
                    getline(cin, description);

                    if (todo_manager.delete_task(description)) {
                        cout << "Task deleted!\n";
                    } else {
                        cout << "Task not found!\n";
                    }
                    break;
                }
                case 4: {
                    string filter_option;
                    cout << "Filter options: all, completed, pending\n";
                    cout << "Enter filter option: ";
                    cin >> filter_option;

                    todo_manager.view_tasks(filter_option);
                    break;
                }
                case 5:
                    todo_manager.undo();
                    break;
                case 6:
                    todo_manager.redo();
                    break;
                case 7:
                    cout << "Exiting...\n";
                    return 0;
                default:
                    cout << "Invalid choice! Please enter a valid option.\n";
                    break;
            }
        }
    } catch (const exception& ex) {
        cerr << "An exception occurred: " << ex.what() << endl;
        Logger::log("An exception occurred: " + string(ex.what()));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
