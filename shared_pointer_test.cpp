#include <iostream>
#include <vector>
#include <memory>

class Session
{
public:
    Session(int id, std::vector<std::shared_ptr<Session>> &sessions)
        : id_(id), sessions_(sessions)
    {
        // Add this session to the sessions vector
        sessions_.push_back(std::shared_ptr<Session>(this));
    }

    void printId() const
    {
        std::cout << "Session ID: " << id_ << std::endl;
    }

private:
    int id_;
    std::vector<std::shared_ptr<Session>> &sessions_;
};

int main()
{
    std::vector<std::shared_ptr<Session>> sessions;

    // Create new Session objects
    auto session1 = std::make_shared<Session>(1, sessions);
    auto session2 = std::make_shared<Session>(2, sessions);
    std::shared_ptr<Session> session3(new Session(3, sessions));

    // Print session IDs
    session1->printId(); // Output: Session ID: 1
    session2->printId(); // Output: Session ID: 2
    session3->printId(); // Output: Session ID: 2

    // Print all sessions' IDs from the sessions vector
    for (const auto &session : sessions)
    {
        session->printId();
    }

    return 0;
}
