#pragma once

#include <string>
#include <typeindex>

namespace CarHMI {

class PropertyBase {
public:
    PropertyBase(const std::string& name, std::type_index type)
        : m_name(name), m_type(type) {}
    virtual ~PropertyBase() = default;

    const std::string& GetName() const { return m_name; }
    std::type_index GetType() const { return m_type; }

    virtual std::string ToString() const = 0;
    virtual void FromString(const std::string& str) = 0;

protected:
    std::string m_name;
    std::type_index m_type;
};

} // namespace CarHMI
