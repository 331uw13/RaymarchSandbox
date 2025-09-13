#ifndef INT_VECTOR_HPP
#define INT_VECTOR_HPP



struct IVector2 {
    int x;
    int y;

    auto operator<=>(IVector2 const &) const = default;
};




#endif
