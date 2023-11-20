
# Personal To-Do List Manager Programming Exercise

Create a simple To-Do List Manager where users can add tasks, mark them as completed, and delete them. The user should also have the option to view all tasks or filter them based on their completion status.

### Functional Requirements

    1. Enable users to add new tasks with a description and due date.
    2. Allow tasks to be marked as 'completed'.
    3. Provide an option to delete tasks.
    4. Implement the ability to view tasks, either all at once or filtered by 'completed' or 'pending'.

### Implementation and Explaination

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
    using namespace std;

This section includes various C++ standard library headers needed for different functionalities, such as input/output, data structures (vector), memory management (memory), date and time operations (ctime), algorithmic operations (algorithm), formatted output (iomanip), string stream processing (sstream), exception handling (stdexcept), file input/output (fstream), and other standard utilities.

    
    class Logger {
    public:
        static void log(const string& message) {
            ofstream logFile("app_log.txt", ios_base::app);
            if (logFile.is_open()) {
                logFile << getCurrentTime() << " " << message << endl;
                logFile.close();
            }
        }

Defines a Logger class responsible for logging messages. The log function logs a message to a file named "app_log.txt" along with a timestamp.

    private:
        static string getCurrentTime() {
            time_t now = time(nullptr);
            tm timeinfo = *localtime(&now);
            char buffer[80];
            strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S]", &timeinfo);
            return buffer;
        }
    };

Contains a private static method getCurrentTime that returns the current time as a formatted string, used for timestamping log messages.

    class TaskManagerException : public runtime_error {
    public:
        TaskManagerException(const string& message) : runtime_error(message) {}
    };

Defines a custom exception class TaskManagerException that inherits from runtime_error. It's used to handle exceptions specific to the TaskManager.

    class TaskMemento {
    public:
        string description;
        bool completed;
        tm due_date;

        TaskMemento(const string& desc, bool comp, const tm& date)
            : description(desc), completed(comp), due_date(date) {}
    };

Defines a TaskMemento class representing a snapshot of a Task object's state. It includes the task's description, completion status, and due date.

    class Task {
    public:
        string description;
        bool completed;
        tm due_date;
        Task(const string& desc) : description(desc), completed(false) {}
        void mark_completed() {
            completed = true;
        }

Defines a Task class representing a task with a description, completion status, and due date. It includes a constructor to initialize the task with a description and a method to mark the task as completed.

    void undo(const TaskMemento& memento) {
        description = memento.description;
        completed = memento.completed;
        due_date = memento.due_date;
    }

Provides an undo method to revert the task to a previous state using a TaskMemento object.

    TaskMemento create_memento() const {
        return TaskMemento(description, completed, due_date);
    }

Creates and returns a TaskMemento object representing the current state of the task.

        void print() const {
            string status = completed ? "Completed" : "Pending";
            cout << description << " - " << status << ", Due: " << due_date.tm_year + 1900
                << "-" << due_date.tm_mon + 1 << "-" << due_date.tm_mday << endl;
        }
    };

Implements a print method to display the details of a task, including its description, completion status, and due date.

    class TaskBuilder {
    private:
        shared_ptr<Task> task;
    public:
        TaskBuilder(const string& desc) : task(make_shared<Task>(desc)) {}

Defines a TaskBuilder class responsible for building Task objects. It includes a constructor that initializes the task builder with a description.

    TaskBuilder& set_due_date(const tm& date) {
        task->due_date = date;
        return *this;
    }

Provides a set_due_date method to set the due date for the task being built.

        shared_ptr<Task> build() const {
            return task;
        }
    };

Implements a build method to create and return the Task object built by the TaskBuilder.

    class TodoListManager {
    private:
        vector<shared_ptr<Task>> tasks;
        vector<TaskMemento> undo_stack;
        vector<TaskMemento> redo_stack;

    public:
        void add_task(const shared_ptr<Task>& task) {
            tasks.push_back(task);
            undo_stack.push_back(task->create_memento());
            Logger::log("Task added: " + task->description);
        }

Defines a TodoListManager class responsible for managing a list of tasks. It includes a method add_task to add a task to the task list.

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

Provides a mark_completed method to mark a task as completed, updating the task list and logging the action.

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

Implements a delete_task method to remove a task from the task list, updating the undo stack and logging the action.

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

Provides an undo method to undo the last operation, restoring the previous state of the task list.

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

Implements a redo method to redo the last undone operation, restoring the state that was previously undone.

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

Implements a view_tasks method to display the tasks based on a filter option (all, completed, pending).

## Output

    C:\Users\abhij\Downloads>g++ to_do_list.cpp

    C:\Users\abhij\Downloads>to_do_list

    Options:
    1. Add Task
    2. Mark Task as Completed
    3. Delete Task
    4. View Tasks
    5. Undo
    6. Redo
    7. Exit
    Enter your choice: 1
    Enter task description: Task1
    Enter due date (YYYY MM DD): 2024 03 01
    Task added successfully!

    Options:
    1. Add Task
    2. Mark Task as Completed
    3. Delete Task
    4. View Tasks
    5. Undo
    6. Redo
    7. Exit
    Enter your choice: 4
    Filter options: all, completed, pending
    Enter filter option: 1
    Task1 - Pending, Due: 2024-1-0

    Options:
    1. Add Task
    2. Mark Task as Completed
    3. Delete Task
    4. View Tasks
    5. Undo
    6. Redo
    7. Exit
    Enter your choice: 2
    Enter task description to mark as completed: Task1
    Task marked as completed!

    Options:
    1. Add Task
    2. Mark Task as Completed
    3. Delete Task
    4. View Tasks
    5. Undo
    6. Redo
    7. Exit
    Enter your choice: 3
    Enter task description to delete: Task2
    Task not found!

    Options:
    1. Add Task
    2. Mark Task as Completed
    3. Delete Task
    4. View Tasks
    5. Undo
    6. Redo
    7. Exit
    Enter your choice: 4
    Filter options: all, completed, pending
    Enter filter option: 1
    Task1 - Completed, Due: 2024-1-0

    Options:
    1. Add Task
    2. Mark Task as Completed
    3. Delete Task
    4. View Tasks
    5. Undo
    6. Redo
    7. Exit
    Enter your choice: 3
    Enter task description to delete: Task1
    Task deleted!

