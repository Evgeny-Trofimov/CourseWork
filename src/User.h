#ifndef USER_H
#define USER_H

#include <string>

using namespace std;

class User {
private:
    int m_id;
    string m_login;
    string m_role;

public:
    User() : m_id(0) {}
    User(int id, const std::string& login, const std::string& role)
        : m_id(id), m_login(login), m_role(role) {}

    int getId() const { return m_id; }
    string getLogin() const { return m_login; }
    string getRole() const { return m_role; }
    bool isAdmin() const { return m_role == "admin"; }
};

#endif
