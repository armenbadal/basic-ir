#pragma once

namespace basic {

class Type {
public:
    virtual ~Type() = default;
};

class ScalarType : public Type {
public:
};

class ArrayType : public Type {
public:
};

} // namespace basic
