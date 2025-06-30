#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>

// Forward declarations
class User;
class Interview;
class Scheduler;

// Enum for user roles
enum class UserRole {
    HR_MANAGER,
    INTERVIEWER
};

// Enum for interview status
enum class InterviewStatus {
    SCHEDULED,
    COMPLETED,
    CANCELLED,
    RESCHEDULED
};

// Time slot structure
struct TimeSlot {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    
    TimeSlot(const std::chrono::system_clock::time_point& start, 
             const std::chrono::system_clock::time_point& end)
        : start_time(start), end_time(end) {}
    
    bool overlaps(const TimeSlot& other) const {
        return start_time < other.end_time && end_time > other.start_time;
    }
    
    std::string to_string() const {
        auto start_t = std::chrono::system_clock::to_time_t(start_time);
        auto end_t = std::chrono::system_clock::to_time_t(end_time);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&start_t), "%Y-%m-%d %H:%M") 
           << " - " 
           << std::put_time(std::localtime(&end_t), "%H:%M");
        return ss.str();
    }
};

// User class
class User {
private:
    static int next_id;
    int user_id;
    std::string name;
    std::string email;
    UserRole role;
    std::vector<TimeSlot> availability;
    std::set<int> scheduled_interviews;

public:
    User(const std::string& name, const std::string& email, UserRole role)
        : user_id(next_id++), name(name), email(email), role(role) {}
    
    // Getters
    int get_id() const { return user_id; }
    const std::string& get_name() const { return name; }
    const std::string& get_email() const { return email; }
    UserRole get_role() const { return role; }
    const std::vector<TimeSlot>& get_availability() const { return availability; }
    const std::set<int>& get_scheduled_interviews() const { return scheduled_interviews; }
    
    // Add availability slot
    void add_availability(const TimeSlot& slot) {
        availability.push_back(slot);
    }
    
    // Check if user is available during a time slot
    bool is_available(const TimeSlot& slot) const {
        for (const auto& available_slot : availability) {
            if (slot.start_time >= available_slot.start_time && 
                slot.end_time <= available_slot.end_time) {
                return true;
            }
        }
        return false;
    }
    
    // Add scheduled interview
    void add_scheduled_interview(int interview_id) {
        scheduled_interviews.insert(interview_id);
    }
    
    // Remove scheduled interview
    void remove_scheduled_interview(int interview_id) {
        scheduled_interviews.erase(interview_id);
    }
    
    std::string role_to_string() const {
        return (role == UserRole::HR_MANAGER) ? "HR Manager" : "Interviewer";
    }
};

int User::next_id = 1;

// Interview class
class Interview {
private:
    static int next_id;
    int interview_id;
    std::string candidate_name;
    std::string position;
    int hr_manager_id;
    int interviewer_id;
    TimeSlot time_slot;
    InterviewStatus status;
    std::string notes;

public:
    Interview(const std::string& candidate, const std::string& pos, 
              int hr_id, int int_id, const TimeSlot& slot)
        : interview_id(next_id++), candidate_name(candidate), position(pos),
          hr_manager_id(hr_id), interviewer_id(int_id), time_slot(slot),
          status(InterviewStatus::SCHEDULED) {}
    
    // Getters
    int get_id() const { return interview_id; }
    const std::string& get_candidate_name() const { return candidate_name; }
    const std::string& get_position() const { return position; }
    int get_hr_manager_id() const { return hr_manager_id; }
    int get_interviewer_id() const { return interviewer_id; }
    const TimeSlot& get_time_slot() const { return time_slot; }
    InterviewStatus get_status() const { return status; }
    const std::string& get_notes() const { return notes; }
    
    // Setters
    void set_status(InterviewStatus new_status) { status = new_status; }
    void set_notes(const std::string& new_notes) { notes = new_notes; }
    void set_time_slot(const TimeSlot& new_slot) { time_slot = new_slot; }
    
    std::string status_to_string() const {
        switch (status) {
            case InterviewStatus::SCHEDULED: return "Scheduled";
            case InterviewStatus::COMPLETED: return "Completed";
            case InterviewStatus::CANCELLED: return "Cancelled";
            case InterviewStatus::RESCHEDULED: return "Rescheduled";
            default: return "Unknown";
        }
    }
};

int Interview::next_id = 1;

// Scheduler class - main application logic
class Scheduler {
private:
    std::map<int, std::unique_ptr<User>> users;
    std::map<int, std::unique_ptr<Interview>> interviews;
    
public:
    // Add user to system
    int add_user(const std::string& name, const std::string& email, UserRole role) {
        auto user = std::make_unique<User>(name, email, role);
        int user_id = user->get_id();
        users[user_id] = std::move(user);
        return user_id;
    }
    
    // Get user by ID
    User* get_user(int user_id) {
        auto it = users.find(user_id);
        return (it != users.end()) ? it->second.get() : nullptr;
    }
    
    // Get all HR managers
    std::vector<User*> get_hr_managers() {
        std::vector<User*> managers;
        for (auto& pair : users) {
            if (pair.second->get_role() == UserRole::HR_MANAGER) {
                managers.push_back(pair.second.get());
            }
        }
        return managers;
    }
    
    // Get all interviewers
    std::vector<User*> get_interviewers() {
        std::vector<User*> interviewers;
        for (auto& pair : users) {
            if (pair.second->get_role() == UserRole::INTERVIEWER) {
                interviewers.push_back(pair.second.get());
            }
        }
        return interviewers;
    }
    
    // Schedule interview
    int schedule_interview(const std::string& candidate_name, 
                          const std::string& position,
                          int hr_manager_id, 
                          int interviewer_id, 
                          const TimeSlot& time_slot) {
        
        User* hr_manager = get_user(hr_manager_id);
        User* interviewer = get_user(interviewer_id);
        
        if (!hr_manager || !interviewer) {
            throw std::runtime_error("Invalid user ID");
        }
        
        if (hr_manager->get_role() != UserRole::HR_MANAGER) {
            throw std::runtime_error("User is not an HR manager");
        }
        
        if (interviewer->get_role() != UserRole::INTERVIEWER) {
            throw std::runtime_error("User is not an interviewer");
        }
        
        // Check availability
        if (!hr_manager->is_available(time_slot)) {
            throw std::runtime_error("HR manager is not available at this time");
        }
        
        if (!interviewer->is_available(time_slot)) {
            throw std::runtime_error("Interviewer is not available at this time");
        }
        
        // Check for conflicts with existing interviews
        if (has_conflict(hr_manager_id, time_slot) || has_conflict(interviewer_id, time_slot)) {
            throw std::runtime_error("Time slot conflicts with existing interview");
        }
        
        // Create interview
        auto interview = std::make_unique<Interview>(candidate_name, position, 
                                                   hr_manager_id, interviewer_id, time_slot);
        int interview_id = interview->get_id();
        
        // Add to users' scheduled interviews
        hr_manager->add_scheduled_interview(interview_id);
        interviewer->add_scheduled_interview(interview_id);
        
        interviews[interview_id] = std::move(interview);
        
        return interview_id;
    }
    
    // Check for scheduling conflicts
    bool has_conflict(int user_id, const TimeSlot& time_slot) {
        User* user = get_user(user_id);
        if (!user) return false;
        
        for (int interview_id : user->get_scheduled_interviews()) {
            auto it = interviews.find(interview_id);
            if (it != interviews.end() && 
                it->second->get_status() == InterviewStatus::SCHEDULED &&
                it->second->get_time_slot().overlaps(time_slot)) {
                return true;
            }
        }
        return false;
    }
    
    // Get interview by ID
    Interview* get_interview(int interview_id) {
        auto it = interviews.find(interview_id);
        return (it != interviews.end()) ? it->second.get() : nullptr;
    }
    
    // Cancel interview
    bool cancel_interview(int interview_id) {
        Interview* interview = get_interview(interview_id);
        if (!interview) return false;
        
        interview->set_status(InterviewStatus::CANCELLED);
        
        // Remove from users' scheduled interviews
        User* hr_manager = get_user(interview->get_hr_manager_id());
        User* interviewer = get_user(interview->get_interviewer_id());
        
        if (hr_manager) hr_manager->remove_scheduled_interview(interview_id);
        if (interviewer) interviewer->remove_scheduled_interview(interview_id);
        
        return true;
    }
    
    // Get all interviews for a user
    std::vector<Interview*> get_user_interviews(int user_id) {
        std::vector<Interview*> user_interviews;
        User* user = get_user(user_id);
        if (!user) return user_interviews;
        
        for (int interview_id : user->get_scheduled_interviews()) {
            auto it = interviews.find(interview_id);
            if (it != interviews.end()) {
                user_interviews.push_back(it->second.get());
            }
        }
        return user_interviews;
    }
    
    // Get all interviews
    std::vector<Interview*> get_all_interviews() {
        std::vector<Interview*> all_interviews;
        for (auto& pair : interviews) {
            all_interviews.push_back(pair.second.get());
        }
        return all_interviews;
    }
    
    // Display system statistics
    void display_statistics() {
        std::cout << "\n=== CLOUDFIT SCHEDULING STATISTICS ===\n";
        std::cout << "Total Users: " << users.size() << std::endl;
        std::cout << "HR Managers: " << get_hr_managers().size() << std::endl;
        std::cout << "Interviewers: " << get_interviewers().size() << std::endl;
        std::cout << "Total Interviews: " << interviews.size() << std::endl;
        
        int scheduled = 0, completed = 0, cancelled = 0;
        for (auto& pair : interviews) {
            switch (pair.second->get_status()) {
                case InterviewStatus::SCHEDULED: scheduled++; break;
                case InterviewStatus::COMPLETED: completed++; break;
                case InterviewStatus::CANCELLED: cancelled++; break;
                case InterviewStatus::RESCHEDULED: break;
            }
        }
        
        std::cout << "Scheduled: " << scheduled << std::endl;
        std::cout << "Completed: " << completed << std::endl;
        std::cout << "Cancelled: " << cancelled << std::endl;
        std::cout << "=======================================\n";
    }
};

// Utility functions
std::chrono::system_clock::time_point parse_datetime(const std::string& datetime_str) {
    // Simple datetime parsing for demo (YYYY-MM-DD HH:MM format)
    std::tm tm = {};
    std::istringstream ss(datetime_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

void print_user_details(const User* user) {
    std::cout << "ID: " << user->get_id() 
              << ", Name: " << user->get_name()
              << ", Email: " << user->get_email()
              << ", Role: " << user->role_to_string() << std::endl;
}

void print_interview_details(const Interview* interview, const Scheduler& scheduler) {
    User* hr = scheduler.get_user(interview->get_hr_manager_id());
    User* interviewer = scheduler.get_user(interview->get_interviewer_id());
    
    std::cout << "Interview ID: " << interview->get_id() << std::endl;
    std::cout << "Candidate: " << interview->get_candidate_name() << std::endl;
    std::cout << "Position: " << interview->get_position() << std::endl;
    std::cout << "HR Manager: " << (hr ? hr->get_name() : "Unknown") << std::endl;
    std::cout << "Interviewer: " << (interviewer ? interviewer->get_name() : "Unknown") << std::endl;
    std::cout << "Time: " << interview->get_time_slot().to_string() << std::endl;
    std::cout << "Status: " << interview->status_to_string() << std::endl;
    if (!interview->get_notes().empty()) {
        std::cout << "Notes: " << interview->get_notes() << std::endl;
    }
    std::cout << "---" << std::endl;
}

// Main application
int main() {
    Scheduler scheduler;
    
    std::cout << "=== CLOUDFIT INTERVIEW SCHEDULING SYSTEM ===\n\n";
    
    // Add some sample users
    int hr1 = scheduler.add_user("Alice Johnson", "alice@cloudfit.com", UserRole::HR_MANAGER);
    int hr2 = scheduler.add_user("Bob Smith", "bob@cloudfit.com", UserRole::HR_MANAGER);
    int int1 = scheduler.add_user("Carol Davis", "carol@cloudfit.com", UserRole::INTERVIEWER);
    int int2 = scheduler.add_user("David Wilson", "david@cloudfit.com", UserRole::INTERVIEWER);
    int int3 = scheduler.add_user("Eve Brown", "eve@cloudfit.com", UserRole::INTERVIEWER);
    
    // Add availability for users (using current time + offsets for demo)
    auto now = std::chrono::system_clock::now();
    auto tomorrow = now + std::chrono::hours(24);
    auto day_after = now + std::chrono::hours(48);
    
    // Add availability slots
    User* alice = scheduler.get_user(hr1);
    alice->add_availability(TimeSlot(tomorrow, tomorrow + std::chrono::hours(8)));
    alice->add_availability(TimeSlot(day_after, day_after + std::chrono::hours(6)));
    
    User* carol = scheduler.get_user(int1);
    carol->add_availability(TimeSlot(tomorrow, tomorrow + std::chrono::hours(4)));
    carol->add_availability(TimeSlot(day_after, day_after + std::chrono::hours(8)));
    
    User* david = scheduler.get_user(int2);
    david->add_availability(TimeSlot(tomorrow + std::chrono::hours(2), 
                                   tomorrow + std::chrono::hours(6)));
    
    // Schedule some sample interviews
    try {
        TimeSlot slot1(tomorrow + std::chrono::hours(1), tomorrow + std::chrono::hours(2));
        int interview1 = scheduler.schedule_interview("John Doe", "Software Engineer", 
                                                     hr1, int1, slot1);
        
        TimeSlot slot2(day_after + std::chrono::hours(2), day_after + std::chrono::hours(3));
        int interview2 = scheduler.schedule_interview("Jane Smith", "Product Manager", 
                                                     hr1, int1, slot2);
        
        std::cout << "Sample interviews scheduled successfully!\n\n";
        
    } catch (const std::exception& e) {
        std::cout << "Error scheduling interview: " << e.what() << std::endl;
    }
    
    // Interactive menu
    int choice;
    do {
        std::cout << "\n=== MAIN MENU ===\n";
        std::cout << "1. View all users\n";
        std::cout << "2. View all interviews\n";
        std::cout << "3. Schedule new interview\n";
        std::cout << "4. Cancel interview\n";
        std::cout << "5. View user's interviews\n";
        std::cout << "6. Display statistics\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";
        
        std::cin >> choice;
        
        switch (choice) {
            case 1: {
                std::cout << "\n=== ALL USERS ===\n";
                auto hr_managers = scheduler.get_hr_managers();
                auto interviewers = scheduler.get_interviewers();
                
                std::cout << "HR Managers:\n";
                for (const auto& user : hr_managers) {
                    print_user_details(user);
                }
                
                std::cout << "\nInterviewers:\n";
                for (const auto& user : interviewers) {
                    print_user_details(user);
                }
                break;
            }
            
            case 2: {
                std::cout << "\n=== ALL INTERVIEWS ===\n";
                auto interviews = scheduler.get_all_interviews();
                for (const auto& interview : interviews) {
                    print_interview_details(interview, scheduler);
                }
                break;
            }
            
            case 3: {
                std::cout << "\n=== SCHEDULE NEW INTERVIEW ===\n";
                std::cout << "Feature not fully implemented in console demo.\n";
                std::cout << "This would include forms for candidate details, time selection, etc.\n";
                break;
            }
            
            case 4: {
                std::cout << "\n=== CANCEL INTERVIEW ===\n";
                std::cout << "Enter interview ID to cancel: ";
                int id;
                std::cin >> id;
                
                if (scheduler.cancel_interview(id)) {
                    std::cout << "Interview cancelled successfully.\n";
                } else {
                    std::cout << "Interview not found.\n";
                }
                break;
            }
            
            case 5: {
                std::cout << "\n=== USER'S INTERVIEWS ===\n";
                std::cout << "Enter user ID: ";
                int user_id;
                std::cin >> user_id;
                
                auto interviews = scheduler.get_user_interviews(user_id);
                if (interviews.empty()) {
                    std::cout << "No interviews found for this user.\n";
                } else {
                    for (const auto& interview : interviews) {
                        print_interview_details(interview, scheduler);
                    }
                }
                break;
            }
            
            case 6: {
                scheduler.display_statistics();
                break;
            }
            
            case 0:
                std::cout << "Goodbye!\n";
                break;
                
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
        
    } while (choice != 0);
    
    return 0;
}
